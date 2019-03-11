package com.duosat.tv.main;

import android.app.Activity;
import android.app.Dialog;
import android.content.Intent;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.text.format.DateFormat;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.duosat.tv.R;
import com.duosat.tv.http.AppController;
import com.duosat.tv.http.DuosatAPI;
import com.duosat.tv.http.DuosatAPIConstant;
import com.duosat.tv.http.VolleyCallback;
import com.duosat.tv.model.ChannelMenuItem;
import com.duosat.tv.model.EpgMenuItem;
import com.duosat.tv.utils.AppConstants;
import com.duosat.tv.utils.Utils;
import com.goalbit.android.sdk.Content;
import com.goalbit.android.sdk.ContentLoader;
import com.goalbit.android.sdk.GoalBitPlus;
import com.goalbit.android.sdk.Log;
import com.google.android.exoplayer2.C;
import com.google.android.exoplayer2.DefaultLoadControl;
import com.google.android.exoplayer2.DefaultRenderersFactory;
import com.google.android.exoplayer2.ExoPlaybackException;
import com.google.android.exoplayer2.ExoPlayer;
import com.google.android.exoplayer2.ExoPlayerFactory;
import com.google.android.exoplayer2.PlaybackParameters;
import com.google.android.exoplayer2.Player;
import com.google.android.exoplayer2.SimpleExoPlayer;
import com.google.android.exoplayer2.Timeline;
import com.google.android.exoplayer2.source.BehindLiveWindowException;
import com.google.android.exoplayer2.source.ExtractorMediaSource;
import com.google.android.exoplayer2.source.MediaSource;
import com.google.android.exoplayer2.source.TrackGroupArray;
import com.google.android.exoplayer2.source.hls.HlsMediaSource;
import com.google.android.exoplayer2.trackselection.AdaptiveTrackSelection;
import com.google.android.exoplayer2.trackselection.DefaultTrackSelector;
import com.google.android.exoplayer2.trackselection.TrackSelection;
import com.google.android.exoplayer2.ui.PlayerView;
import com.google.android.exoplayer2.upstream.BandwidthMeter;
import com.google.android.exoplayer2.upstream.DataSource;
import com.google.android.exoplayer2.upstream.DefaultAllocator;
import com.google.android.exoplayer2.upstream.DefaultBandwidthMeter;
import com.google.android.exoplayer2.upstream.DefaultDataSourceFactory;
import com.google.android.exoplayer2.upstream.TransferListener;
import com.google.android.exoplayer2.util.Util;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.xmlpull.v1.XmlPullParserException;

import java.io.File;
import java.io.IOException;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.TimeZone;
import java.util.Timer;
import java.util.TimerTask;

import javax.xml.namespace.NamespaceContext;

public class MainActivity extends Activity implements View.OnClickListener{

    private static final String KEY_PLAY_WHEN_READY = "play_when_ready";
    private static final String KEY_WINDOW = "window";
    private static final String KEY_POSITION = "position";

    private final static int            UPDATE_TIME_DELAY = 1 * 1000;

    RelativeLayout  m_rlDuosatLive, m_rlVod, m_rlCatchUpTV, m_rlSettings, m_rlAccountSetting;
    ImageView       m_ivDuosatLive, m_ivVod, m_ivCatchUpTV, m_ivSettings, m_ivAccountSetting, m_ivLogout;

    TextView m_tvTime, m_tvDate, m_tvLocal;

    Handler handler = new Handler();
    private PlayerView playerView;
    private SimpleExoPlayer player;

    private Timeline.Window         window;
    private DataSource.Factory      mediaDataSourceFactory;
    private DefaultTrackSelector    trackSelector;
    private TrackGroupArray         lastSeenTrackGroupArray;
    private boolean                 shouldAutoPlay;
    private BandwidthMeter          bandwidthMeter;

    private boolean     playWhenReady;
    private int         currentWindow;
    private long        playbackPosition;

    private boolean     goalbitChannel = false;
    private int         sessionID;

    private ProgressBar     progressBar;
    private TextView        tvMaintainence;

    private Handler reconnectHandler = new Handler();
    private Runnable reconnectRunnable = new Runnable() {
        @Override
        public void run() {
            currentWindow = C.INDEX_UNSET;
            playbackPosition = C.INDEX_UNSET;
            initializePlayer();
        }
    };

    private String      m_path;

    View                m_oldFocusView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main_layout);

        if (savedInstanceState == null) {
            playWhenReady = true;
            currentWindow = 0;
            playbackPosition = 0;
        } else {
            playWhenReady = savedInstanceState.getBoolean(KEY_PLAY_WHEN_READY);
            currentWindow = savedInstanceState.getInt(KEY_WINDOW);
            playbackPosition = savedInstanceState.getLong(KEY_POSITION);
        }

        shouldAutoPlay = true;
        bandwidthMeter = new DefaultBandwidthMeter();
        mediaDataSourceFactory = new DefaultDataSourceFactory(this, Util.getUserAgent(this, "mediaPlayerSample"), (TransferListener<? super DataSource>) bandwidthMeter);
        window = new Timeline.Window();

        Utils.disableSSLCertificateChecking();

        initControl();
        setEventListener();
        loadChannelData();

        handler.post(runnableUpdateTime);
        GoalBitPlus sdk = GoalBitPlus.getInstance();
        if ( !sdk.initialize( getApplicationContext(), 2 ) ) {
            AppConstants.isP2P = false;
        }
    }

    @Override
    public void onStart() {
        super.onStart();
        if (Util.SDK_INT > 23 && findViewById(R.id.rlmainPage).getVisibility() == View.VISIBLE) {
            initializePlayer();
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        if ((Util.SDK_INT <= 23 || player == null) && findViewById(R.id.rlmainPage).getVisibility() == View.VISIBLE) {
            initializePlayer();
        }

        if(m_oldFocusView != null)
            m_oldFocusView.requestFocus();
    }

    @Override
    public void onPause() {
        super.onPause();
        stopGPASession();
        if (Util.SDK_INT <= 23) {
            releasePlayer();
        }
    }

    @Override
    public void onStop() {
        super.onStop();

        if (Util.SDK_INT > 23) {
            releasePlayer();
        }
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        updateStartPosition();

        outState.putBoolean(KEY_PLAY_WHEN_READY, playWhenReady);
        outState.putInt(KEY_WINDOW, currentWindow);
        outState.putLong(KEY_POSITION, playbackPosition);
        super.onSaveInstanceState(outState);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        handler.removeCallbacks(runnableUpdateTime);
    }

    @Override
    public void onBackPressed() {
        AppController.getInstance().cancelPendingRequests(AppConstants.LIVE_TAG);
        super.onBackPressed();
        finish();
        if (this.goalbitChannel) {
            stopGPASession();
        }
    }

    private void stopGPASession() {
        goalbitChannel =false;
        GoalBitPlus sdk = GoalBitPlus.getInstance();
        sdk.DeleteSession(sessionID);
        Log.d("GoalBitPlusSDK.P2PPlayerActivity", "GPA session stopped");
    }

    private void updateStartPosition() {
        if (player == null) return;

        playbackPosition = player.getCurrentPosition();
        currentWindow = player.getCurrentWindowIndex();
        playWhenReady = player.getPlayWhenReady();
    }

    private void loadChannelData() {
        AppConstants.CHANNEL_LIST.clear();
        String userId = Utils.getSharePreferenceValue(this, AppConstants.LOGIN_USER, "");
        String password = Utils.getSharePreferenceValue(this, AppConstants.LOGIN_PASSWORD, "");

        DuosatAPI.getLiveChannels(userId, password, new VolleyCallback() {
            @Override
            public void onSuccess(String result) {
                final String resultString = result;
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        try {
                            final Date currentDate = Utils.CurrentTime();
                            JSONObject jsonObj = new JSONObject(resultString);
                            JSONArray liveArray = jsonObj.getJSONArray(DuosatAPIConstant.ITEM_NOEDS);
                            for(int i = 0; i < liveArray.length(); i++){
                                JSONObject jsonObject = liveArray.getJSONObject(i).getJSONObject(DuosatAPIConstant.ITEM_NOED);
                                ChannelMenuItem channelMenuItem = new ChannelMenuItem();
                                channelMenuItem.channelName = jsonObject.has(DuosatAPIConstant.ITEM_NAME) ? jsonObject.getString(DuosatAPIConstant.ITEM_NAME) : "";
                                channelMenuItem.channelGenre = jsonObject.has(DuosatAPIConstant.ITEM_GENRE) ? jsonObject.getString(DuosatAPIConstant.ITEM_GENRE) : "";
                                channelMenuItem.channelURL = jsonObject.has(DuosatAPIConstant.ITEM_URL) ? jsonObject.getString(DuosatAPIConstant.ITEM_URL) : "";
                                channelMenuItem.channelDescription = jsonObject.has(DuosatAPIConstant.ITEM_DESCRIPTION) ? jsonObject.getString(DuosatAPIConstant.ITEM_DESCRIPTION) : "";
                                channelMenuItem.channelNumber = jsonObject.has(DuosatAPIConstant.ITEM_CHANNEL_NUMBER) ? jsonObject.getString(DuosatAPIConstant.ITEM_CHANNEL_NUMBER) : "";
                                channelMenuItem.channelP2PUrl = jsonObject.has(DuosatAPIConstant.ITEM_P2P_URL) ? jsonObject.getString(DuosatAPIConstant.ITEM_P2P_URL) : "";
                                channelMenuItem.channelEPGData = jsonObject.has(DuosatAPIConstant.ITEM_EPGURL)? jsonObject.getString(DuosatAPIConstant.ITEM_EPGURL) : "";
                                channelMenuItem.channelFeed = jsonObject.has(DuosatAPIConstant.ITEM_FEED)? jsonObject.getString(DuosatAPIConstant.ITEM_FEED) : "";
                                channelMenuItem.channelPackage = jsonObject.has(DuosatAPIConstant.ITEM_PACKAGE)? jsonObject.getString(DuosatAPIConstant.ITEM_PACKAGE) : "4";
                                if(channelMenuItem.channelPackage.isEmpty())
                                    channelMenuItem.channelPackage = "4";

                                jsonObject = jsonObject.getJSONObject(DuosatAPIConstant.ITEM_THUMBNAIL_URL);
                                if(jsonObject != null)
                                    channelMenuItem.channelSrc = jsonObject.has(DuosatAPIConstant.ITEM_SRC) ? jsonObject.getString(DuosatAPIConstant.ITEM_SRC) : "";

                                AppConstants.CHANNEL_LIST.add(channelMenuItem);

                                if(!channelMenuItem.channelEPGData.isEmpty()) {
                                    JSONArray epgArray = null;
                                    try {
                                        epgArray = new JSONArray(channelMenuItem.channelEPGData);
                                    } catch (JSONException e) {
                                        e.printStackTrace();
                                        continue;
                                    }
                                    for (int j = 0; j < epgArray.length(); j++) {
                                        try {
                                            JSONObject epgObj = epgArray.getJSONObject(j);
                                            EpgMenuItem epgItem = new EpgMenuItem();
                                            epgItem.strName = epgObj.getString("title");
                                            epgItem.strKind = epgObj.getString("category");
                                            JSONObject epgProgramme = epgObj.getJSONObject("programme");
                                            String startTime = Utils.UTCStringToLocalString(epgProgramme.getString("start")), endTime = Utils.UTCStringToLocalString(epgProgramme.getString("stop"));
                                            SimpleDateFormat format = new SimpleDateFormat("yyyyMMddHHmmss Z");
                                            Date startDate = format.parse(startTime);
                                            Date endDate = format.parse(endTime);
                                            epgItem.m_dateTopicStart = startDate;

                                            if (startDate.before(currentDate) && currentDate.getTime() - startDate.getTime() > 6 * 24 * 3600 * 1000)
                                                continue;
                                            else if (startDate.after(currentDate) && startDate.getTime() - currentDate.getTime() > 7 * 24 * 3600 * 1000)
                                                continue;

                                            epgItem.m_dateTopicEnd = endDate;
                                            String dayOfTheWeek = (String) DateFormat.format("EEEE", startDate); // Thursday
                                            String day = (String) DateFormat.format("d", startDate); // 20
                                            String monthNumber = (String) DateFormat.format("MM", startDate); // 06

                                            epgItem.strTime = dayOfTheWeek + " " + day + "/" + monthNumber + " ";
                                            epgItem.strTime += startTime.substring(8, 10) + ":" + startTime.substring(10, 12) + "-" + endTime.substring(8, 10) + ":" + endTime.substring(10, 12);

                                            int startTimeSec = Integer.parseInt(startTime.substring(8, 10)) * 3600 +
                                                    Integer.parseInt(startTime.substring(10, 12)) * 60 + Integer.parseInt(startTime.substring(12, 14));
                                            int endTimeSec = Integer.parseInt(endTime.substring(8, 10)) * 3600 +
                                                    Integer.parseInt(endTime.substring(10, 12)) * 60 + Integer.parseInt(endTime.substring(12, 14));

                                            int duration = endTimeSec - startTimeSec;

                                            epgItem.strVideoLength = duration / 3600 > 0 ? String.valueOf(duration / 3600) + " hr " : " ";
                                            duration = duration % 3600;
                                            epgItem.strVideoLength += String.valueOf(duration / 60) + " min ";

                                            epgItem.strMark = "+16";

                                            channelMenuItem.m_arrItemTopic.add(epgItem);
                                        }
                                        catch (JSONException e) {
                                            continue;
                                        }
                                        catch (ParseException e) {
                                            continue;
                                        }
                                    }
                                }

                                if(channelMenuItem.m_arrItemTopic.size() > 0)
                                    AppConstants.CATCHUP_TV_CHANNEL_LIST.add(channelMenuItem);
                            }

                            Collections.sort(AppConstants.CHANNEL_LIST, new ChannelComparator());
                            Collections.sort(AppConstants.CATCHUP_TV_CHANNEL_LIST, new ChannelComparator());

                            if(AppConstants.CHANNEL_LIST.size() > 0) {
                                if(AppConstants.CHANNEL_LIST.get(0).channelFeed.equals("p2p") && !AppConstants.CHANNEL_LIST.get(0).channelP2PUrl.isEmpty())
                                    m_path = AppConstants.CHANNEL_LIST.get(0).channelP2PUrl;
                                else {
                                    m_path = AppConstants.CHANNEL_LIST.get(0).channelURL;
                                }
                            }

                            if(m_path.isEmpty() && !AppConstants.CHANNEL_LIST.get(0).channelFeed.equals("p2p")) {
                                Utils.showToast(MainActivity.this, R.string.backup_url_error);
                                m_path = AppConstants.CHANNEL_LIST.get(0).channelP2PUrl;
                            }

                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    findViewById(R.id.rllaunchPage).setVisibility(View.GONE);
                                    findViewById(R.id.ivbackground).setBackgroundResource(R.drawable.bg_main);
                                    findViewById(R.id.rlmainPage).setVisibility(View.VISIBLE);

                                    if(AppConstants.CHANNEL_LIST.size() > 0 && AppConstants.ROLES_LIST.contains(AppConstants.CHANNEL_LIST.get(0).channelPackage))
                                        initializePlayer();
                                    else {
                                        Utils.showToast(MainActivity.this, R.string.package_error);
                                    }
                                }
                            });
                        }catch (JSONException e) {
                            e.printStackTrace();
                        }
                    }
                }).start();
            }

            @Override
            public void onError(Object error) {

            }
        }, AppConstants.LIVE_TAG);
    }

    private void initControl() {
        m_tvTime = (TextView)findViewById(R.id.tvTime);
        m_tvDate = (TextView)findViewById(R.id.tvDate);
        m_tvLocal = (TextView)findViewById(R.id.tvLocal);

        m_ivDuosatLive = (ImageView)findViewById(R.id.ivduosattv);
        m_ivVod = (ImageView)findViewById(R.id.ivvod);
        m_ivCatchUpTV = (ImageView)findViewById(R.id.ivcatchuptv);
        m_ivSettings = (ImageView)findViewById(R.id.ivlargesettings);
        m_ivAccountSetting = (ImageView)findViewById(R.id.ivaccountsetting);
        m_ivLogout = (ImageView)findViewById(R.id.ivlogin);

        m_rlDuosatLive = (RelativeLayout)findViewById(R.id.rlduosattv);
        m_rlVod = (RelativeLayout)findViewById(R.id.rlvodtv);
        m_rlCatchUpTV = (RelativeLayout)findViewById(R.id.rlcatchuptv);
        m_rlSettings = (RelativeLayout)findViewById(R.id.rlsetting);
        m_rlAccountSetting = (RelativeLayout)findViewById(R.id.rlaccountSetting);

        progressBar = (ProgressBar)findViewById(R.id.pbProgress);
        tvMaintainence = (TextView)findViewById(R.id.tvMaintainence);

        m_rlDuosatLive.requestFocus();
    }

    private void setEventListener() {
        m_rlDuosatLive.setOnClickListener(this);
        m_rlDuosatLive.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View v, boolean hasFocus) {
                if(hasFocus) {
                    m_ivDuosatLive.setImageResource(R.drawable.duotv_button_focus);
                }
                else {
                    m_ivDuosatLive.setImageResource(R.drawable.duotv_button);
                }
            }
        });
        m_rlVod.setOnClickListener(this);
        m_rlVod.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View v, boolean hasFocus) {
                if(hasFocus) {
                    m_ivVod.setImageResource(R.drawable.vod_button_focus);
                }
                else {
                    m_ivVod.setImageResource(R.drawable.vod_button);
                }
            }
        });
        m_rlCatchUpTV.setOnClickListener(this);
        m_rlCatchUpTV.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View v, boolean hasFocus) {
                if(hasFocus) {
                    m_ivCatchUpTV.setImageResource(R.drawable.catchup_tv_focus);
                }
                else {
                    m_ivCatchUpTV.setImageResource(R.drawable.catchup_tv);
                }
            }
        });
        m_rlSettings.setOnClickListener(this);
        m_rlSettings.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View v, boolean hasFocus) {
                if(hasFocus) {
                    m_ivSettings.setImageResource(R.drawable.settings_focus);
                }
                else {
                    m_ivSettings.setImageResource(R.drawable.settings);
                }
            }
        });

        m_rlAccountSetting.setOnClickListener(this);
        m_rlAccountSetting.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View v, boolean hasFocus) {
                if(hasFocus) {
                    m_ivAccountSetting.setImageResource(R.drawable.accountsettings_focus);
                }
                else {
                    m_ivAccountSetting.setImageResource(R.drawable.accountsettings);
                }
            }
        });

        m_ivLogout.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
//                logout();
            }
        });
    }

    private void logout() {
//        String username = Utils.getSharePreferenceValue(MainActivity.this, AppConstants.LOGIN_USER, "");
//        String pass = Utils.getSharePreferenceValue(MainActivity.this, AppConstants.LOGIN_PASSWORD, "");

//        DuosatAPI.logOutUser(username, pass, new VolleyCallback() {
//                    @Override
//                    public void onSuccess(String result) {
//                        finish();
//                        Utils.setSharePreferenceValue(MainActivity.this, AppConstants.AUTO_LOGIN, String.valueOf(false));
//                        Intent intent = new Intent(getApplicationContext(), LoginActivity.class);
//                        getApplicationContext().startActivity(intent);
//                    }
//
//                    @Override
//                    public void onError(Object error) {
//
//                    }
//                }, AppConstants.LOGIN_TAG);

        finish();
        Utils.setSharePreferenceValue(MainActivity.this, AppConstants.AUTO_LOGIN, String.valueOf(false));
        Intent intent = new Intent(getApplicationContext(), LoginActivity.class);
        getApplicationContext().startActivity(intent);
    }

    private void initializePlayer() {
        if(playerView == null) {
            playerView = findViewById(R.id.video_view);
        }
        TrackSelection.Factory videoTrackSelectionFactory =
                new AdaptiveTrackSelection.Factory(bandwidthMeter);

        trackSelector = new DefaultTrackSelector(videoTrackSelectionFactory);
        lastSeenTrackGroupArray = null;

        if(m_path.substring(0, 10).equals("goalbit://")) {
            DefaultAllocator allocator = new DefaultAllocator(true, C.DEFAULT_BUFFER_SEGMENT_SIZE);
            DefaultLoadControl loadControl = new DefaultLoadControl(allocator, 360000, 600000, 15000, 20000, -1, true);
            //***

            player = ExoPlayerFactory.newSimpleInstance(new DefaultRenderersFactory(this), trackSelector, loadControl);
        }
        else {
            // Here
            DefaultAllocator allocator = new DefaultAllocator(true, C.DEFAULT_BUFFER_SEGMENT_SIZE);
            DefaultLoadControl loadControl = new DefaultLoadControl(allocator, 360000, 600000, 2500, 5000, -1, true);
            //***

            player = ExoPlayerFactory.newSimpleInstance(new DefaultRenderersFactory(this), trackSelector, loadControl);
        }

        player.addListener(new PlayerEventListener());
        playerView.setPlayer(player);
        playerView.setControllerShowTimeoutMs(1);

        player.setPlayWhenReady(shouldAutoPlay);

        if(m_path == null || m_path.isEmpty())
            return;

        playerView.setVisibility(View.VISIBLE);

        if (m_path.substring(0, 10).equals("goalbit://")) {
            findViewById(R.id.pbProgress).setVisibility(View.VISIBLE);
            new MainActivity.LoadContentXmlTask().execute(m_path);
            return;
        }

        stopGPASession();

        MediaSource mediaSource;
        if(m_path.endsWith(".m3u8"))
            mediaSource = new HlsMediaSource.Factory(mediaDataSourceFactory).createMediaSource(Uri.parse(Utils.makeAvaiableUrl(m_path)));
        else
            mediaSource = new ExtractorMediaSource.Factory(mediaDataSourceFactory).createMediaSource(Uri.parse(Utils.makeAvaiableUrl(m_path)));

        boolean haveStartPosition = currentWindow != C.INDEX_UNSET;
        if (haveStartPosition) {
            player.seekTo(currentWindow, playbackPosition);
        }

        player.prepare(mediaSource, !haveStartPosition, false);
    }

    private void releasePlayer() {
        if (player != null) {
            updateStartPosition();
            shouldAutoPlay = player.getPlayWhenReady();
            player.release();
            player = null;
            trackSelector = null;
        }

        if(reconnectHandler != null)
            reconnectHandler.removeCallbacks(reconnectRunnable);
    }

    private class LoadContentXmlTask extends AsyncTask<String, Void, Object> {
        protected Object doInBackground(String... urls) {
            try {
                ContentLoader e = new ContentLoader();
                Content content = e.getContent(urls[0]);
                return content;
            } catch (IOException var4) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        progressBar.setVisibility(View.INVISIBLE);
                    }
                });
                return getResources().getString(R.string.connection_error);
            } catch (XmlPullParserException var5) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        progressBar.setVisibility(View.INVISIBLE);
                    }
                });
                return getResources().getString(R.string.xml_parsing_error);
            }
        }

        protected void onPostExecute(Object result) {
            if (result instanceof String) {
            } else if (result instanceof Content) {
                startGPASession((Content) result);
            }
        }
    }

    private void startGPASession(Content content) {
        this.goalbitChannel = true;
        com.goalbit.android.sdk.Log.i("GoalBitPlusSDK.P2PPlayerActivity", "Creating a new GPA session");
        GoalBitPlus sdk = GoalBitPlus.getInstance();
        sessionID = sdk.createSession(content.getContentID(), content.getP2pTrackerURL(), content.getHlsServerURL(), content.getP2pManifestURL());
        if(sessionID == 0) {
            findViewById(R.id.pbProgress).setVisibility(View.INVISIBLE);
            Toast.makeText(this, "Could not create P2P session", Toast.LENGTH_LONG).show();
            releasePlayer();
        } else {
            try {
                String e = sdk.getContentPath(this) + File.separator + sessionID;
                File sessionFolder = new File(e);
                if(!sessionFolder.exists() && !sessionFolder.mkdir()) {
                    findViewById(R.id.pbProgress).setVisibility(View.INVISIBLE);
                    com.goalbit.android.sdk.Log.e("GoalBitPlusSDK.P2PPlayerActivity", "ERROR: Could not create folder: " + e);
                    Toast.makeText(this, "Error while creating folder for P2P content!", Toast.LENGTH_LONG).show();
                    releasePlayer();
                    return;
                }
            } catch (Exception var4) {
                findViewById(R.id.pbProgress).setVisibility(View.INVISIBLE);
                var4.printStackTrace();
                Toast.makeText(this, "Error while creating folder for P2P content!", Toast.LENGTH_LONG).show();
                releasePlayer();
                return;
            }

            updateGPAStatus(sessionID);
            Log.d("GoalBitPlusSDK.P2PPlayerActivity", "The GPA session " + sessionID + " was created");
        }
    }

    private void updateGPAStatus(final int sessionID) {
        GoalBitPlus sdk = GoalBitPlus.getInstance();
        String sessionStreamingURL = sdk.getPlayerURL(sessionID);
        Log.d("MAXI", sessionStreamingURL);
        if(sessionStreamingURL != null && sessionStreamingURL.length() > 0) {
            try {
                findViewById(R.id.pbProgress).setVisibility(View.INVISIBLE);
                Log.i("GoalBitPlusSDK.P2PPlayerActivity", "Configuring the MediaPlayer");
                MediaSource mediaSource;
                mediaSource = new HlsMediaSource(Uri.parse(Utils.makeAvaiableUrl(sessionStreamingURL)), mediaDataSourceFactory, 1, null, null);

                player.prepare(mediaSource, !false, false);
            } catch (Exception var7) {
                var7.printStackTrace();
                Toast.makeText(this, "Error while starting the MediaPlayer", Toast.LENGTH_LONG).show();
                releasePlayer();
                return;
            }

            Log.e("GoalBitPlusSDK.P2PPlayerActivity", "Update GPA status: buffering completed");
        } else {
            final Handler qualities = new Handler();
            Timer e = new Timer();
            e.schedule(new TimerTask() {
                public void run() {
                    qualities.post(new Runnable() {
                        public void run() {
                            updateGPAStatus(sessionID);
                        }
                    });
                }
            }, 200L);
        }
    }

    private void showAccountSettingDialog() {
        final Dialog accountDlg = new Dialog(this, R.style.Theme_CustomDialog);
        LayoutInflater inflater = ((LayoutInflater) getSystemService(LAYOUT_INFLATER_SERVICE));
        View accountView = inflater.inflate(R.layout.account_setting_dialog, null, false);
        String username = Utils.getSharePreferenceValue(MainActivity.this, AppConstants.LOGIN_USER, "");
        String pass = Utils.getSharePreferenceValue(MainActivity.this, AppConstants.ACCOUNT_PASSWORD, "9999");
        String isSubscribed = Utils.getSharePreferenceValue(MainActivity.this, AppConstants.ACCOUNT_SUBSCRIBED, "false");
        String passAdult = Utils.getSharePreferenceValue(MainActivity.this, AppConstants.ADULT_PASSWORD, "1234");

        EditText edtAccountID = (EditText)accountView.findViewById(R.id.edtAccountID);
        edtAccountID.setText(username);

        final EditText edtAccoundPwd = (EditText)accountView.findViewById(R.id.edtAccountPassword);
        edtAccoundPwd.setText(pass);

        CheckBox chbSubscribed = (CheckBox)accountView.findViewById(R.id.chboxSubcribed);
        chbSubscribed.setChecked(Boolean.parseBoolean(isSubscribed));

        final EditText edtAdultPwd = (EditText)accountView.findViewById(R.id.edtAdultPassword);
        edtAdultPwd.setText(passAdult);

        accountDlg.getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);

        accountView.findViewById(R.id.tvOkBtn).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Utils.setSharePreferenceValue(MainActivity.this, AppConstants.ACCOUNT_PASSWORD, edtAccoundPwd.getText().toString());
                Utils.setSharePreferenceValue(MainActivity.this, AppConstants.ADULT_PASSWORD, edtAdultPwd.getText().toString());

                Toast.makeText(MainActivity.this, "Updated Successfully!", Toast.LENGTH_SHORT).show();

                accountDlg.dismiss();
            }
        });
        accountDlg.setContentView(accountView);
        accountDlg.show();
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.rlduosattv:
                m_oldFocusView = m_rlDuosatLive;
                Intent liveIntent = new Intent(this, LiveActivity.class);
                liveIntent.putExtra(AppConstants.LIVE_LAAST_CHANNEL_URI, m_path);
                startActivityForResult(liveIntent, AppConstants.LIVE_REQUEST_CODE);
                break;
            case R.id.rlvodtv:
                m_oldFocusView = m_rlVod;
                Intent intent = new Intent(this, VodActivity.class);
                startActivity(intent);
                break;
            case R.id.rlcatchuptv:
                m_oldFocusView = m_rlCatchUpTV;
                Intent catchUpTVIntent = new Intent(this, CatchUpTVActivity.class);
                startActivity(catchUpTVIntent);
                break;
            case R.id.rlsetting:
                m_oldFocusView = m_rlSettings;
                startActivityForResult(new Intent(android.provider.Settings.ACTION_SETTINGS), 0);
                break;
            case R.id.rlaccountSetting:
                m_oldFocusView = m_rlAccountSetting;
                final Dialog protectionDlg = new Dialog(this, R.style.Theme_CustomDialog);
                LayoutInflater inflater = ((LayoutInflater) getSystemService(LAYOUT_INFLATER_SERVICE));
                View protectionView = inflater.inflate(R.layout.protection_dialog, null, false);

                EditText editText = protectionView.findViewById(R.id.edtPassword);
                editText.setOnKeyListener(new View.OnKeyListener() {
                    @Override
                    public boolean onKey(View v, int keyCode, KeyEvent event) {
                        if((keyCode == KeyEvent.KEYCODE_DPAD_CENTER || keyCode == KeyEvent.KEYCODE_ENTER) && event.getAction() == KeyEvent.ACTION_UP) {
                            String strSavedPassword = Utils.getSharePreferenceValue(MainActivity.this, AppConstants.ACCOUNT_PASSWORD, "9999");
                            if(strSavedPassword.equals(((EditText)v).getText().toString())) {
                                showAccountSettingDialog();
                            }
                            else {
                                Toast.makeText(MainActivity.this, "Invalid Password!", Toast.LENGTH_SHORT).show();
                            }
                            protectionDlg.dismiss();
                            return true;
                        }

                        return false;
                    }
                });
                protectionDlg.setContentView(protectionView);
                protectionDlg.getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);
                protectionDlg.show();
                break;
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if(resultCode == RESULT_OK && requestCode == AppConstants.LIVE_REQUEST_CODE) {
            m_path = data.getStringExtra(AppConstants.CURRENT_LIVE_CHANNEL);
        }
    }

    private void setTimeInfo() {
        SimpleDateFormat dateFormat;
        final Calendar calendar = Calendar.getInstance();

        dateFormat = new SimpleDateFormat("HH:mm");
        String mStrTime = dateFormat.format(calendar.getTime());

        dateFormat = new SimpleDateFormat("dd/MM/yy");
        String mStrDate = dateFormat.format(calendar.getTime()) + " ";

        int nWeek = calendar.get(Calendar.DAY_OF_WEEK);
        if(nWeek == 1) {
            mStrDate += "Sun.";
        } else if(nWeek == 2) {
            mStrDate += "Mon.";
        } else if(nWeek == 3) {
            mStrDate += "Tue.";
        } else if(nWeek == 4) {
            mStrDate += "Wed.";
        } else if(nWeek == 5) {
            mStrDate += "Thu.";
        } else if(nWeek == 6) {
            mStrDate += "Fri.";
        } else if(nWeek == 7) {
            mStrDate += "Sat.";
        }

        m_tvTime.setText(mStrTime);
        m_tvDate.setText(mStrDate);

        TimeZone tz = TimeZone.getDefault();
        String strLocal = tz.getID();
        m_tvLocal.setText(strLocal.substring(strLocal.indexOf('/') + 1));
    }

    private Runnable runnableUpdateTime = new Runnable() {
        @Override
        public void run() {
            setTimeInfo();
            handler.postDelayed(runnableUpdateTime, UPDATE_TIME_DELAY);
        }
    };

    public class ChannelComparator implements Comparator<ChannelMenuItem>
    {
        public int compare(ChannelMenuItem left, ChannelMenuItem right) {
            if(left.channelNumber.isEmpty())
                return -1;
            else if(right.channelNumber.isEmpty())
                return 1;
            else if(Integer.parseInt(left.channelNumber) < Integer.parseInt(right.channelNumber))
                return -1;
            else
                return 1;
        }
    }

    private static boolean isBehindLiveWindow(ExoPlaybackException e) {
        if (e.type != ExoPlaybackException.TYPE_SOURCE) {
            return false;
        }
        Throwable cause = e.getSourceException();
        while (cause != null) {
            if (cause instanceof BehindLiveWindowException) {
                return true;
            }
            cause = cause.getCause();
        }
        return false;
    }

    private class PlayerEventListener extends ExoPlayer.DefaultEventListener {
        @Override
        public void onPlayerStateChanged(boolean playWhenReady, int playbackState) {
            if (playbackState == Player.STATE_BUFFERING) {
                progressBar.setVisibility(View.VISIBLE);
                hideMaintainence();
            }
            if (playbackState == Player.STATE_READY) {
                progressBar.setVisibility(View.GONE);
                hideMaintainence();
            }
            if (playbackState == Player.STATE_ENDED) {

            }
        }

        @Override
        public void onPlayerError(ExoPlaybackException e) {
            if (goalbitChannel) {
                player.stop();
                updateGPAStatus(sessionID);
                return;
            }

            new Thread(new Runnable() {@Override
            public void run() {
                if (Utils.getConnectivityStatus(MainActivity.this) != Utils.TYPE_NOT_CONNECTED) {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            progressBar.setVisibility(View.GONE);
//                            showMaintainence();
                        }
                    });
                }
                else {
                    player.stop();
                    playerView.setVisibility(View.INVISIBLE);
                    runOnUiThread(new Runnable() {@Override
                    public void run() {
                        progressBar.setVisibility(View.GONE);
                        showNetworkError();
                    }
                    });
                }
            }
            }).start();

            reconnectHandler.postDelayed(reconnectRunnable, AppConstants.RECONNECT_NEXT_TIME_OUT);
        }
    }

    private void showMaintainence() {
        tvMaintainence.setText("UNDER MAINTAINENCE");
        tvMaintainence.setVisibility(View.VISIBLE);
    }

    private void showNetworkError() {
        tvMaintainence.setText("NETWORK ERROR");
        tvMaintainence.setVisibility(View.VISIBLE);
    }

    private void hideMaintainence() {
        tvMaintainence.setVisibility(View.GONE);
    }
}
