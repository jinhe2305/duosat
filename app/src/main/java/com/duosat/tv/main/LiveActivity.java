package com.duosat.tv.main;

import android.app.Activity;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.text.format.DateFormat;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.duosat.tv.R;
import com.duosat.tv.adapter.ChannelMenuListAdapter;
import com.duosat.tv.adapter.EpgListAdapter;
import com.duosat.tv.adapter.LiveMenuListAdapter;
import com.duosat.tv.model.ArrayChannelItem;
import com.duosat.tv.model.ArrayItemTopic;
import com.duosat.tv.model.ChannelMenuItem;
import com.duosat.tv.model.EpgMenuItem;
import com.duosat.tv.model.LiveMenuActionItem;
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
import com.google.android.exoplayer2.Player;
import com.google.android.exoplayer2.SimpleExoPlayer;
import com.google.android.exoplayer2.Timeline;
import com.google.android.exoplayer2.source.ExtractorMediaSource;
import com.google.android.exoplayer2.source.MediaSource;
import com.google.android.exoplayer2.source.TrackGroupArray;
import com.google.android.exoplayer2.source.hls.HlsMediaSource;
import com.google.android.exoplayer2.trackselection.AdaptiveTrackSelection;
import com.google.android.exoplayer2.trackselection.DefaultTrackSelector;
import com.google.android.exoplayer2.trackselection.TrackSelection;
import com.google.android.exoplayer2.ui.DefaultTimeBar;
import com.google.android.exoplayer2.ui.PlayerView;
import com.google.android.exoplayer2.upstream.BandwidthMeter;
import com.google.android.exoplayer2.upstream.DataSource;
import com.google.android.exoplayer2.upstream.DefaultAllocator;
import com.google.android.exoplayer2.upstream.DefaultBandwidthMeter;
import com.google.android.exoplayer2.upstream.DefaultDataSourceFactory;
import com.google.android.exoplayer2.upstream.TransferListener;
import com.google.android.exoplayer2.util.Util;

import org.xmlpull.v1.XmlPullParserException;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

import static android.view.SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS;

public class LiveActivity extends Activity implements View.OnClickListener, MediaPlayer.OnErrorListener, MediaPlayer.OnBufferingUpdateListener, MediaPlayer.OnPreparedListener, MediaPlayer.OnInfoListener, SurfaceHolder.Callback {

    private static final String KEY_PLAY_WHEN_READY = "play_when_ready";
    private static final String KEY_WINDOW = "window";
    private static final String KEY_POSITION = "position";

    ListView        m_lvMainList, m_lvChannelList, m_lvEpgList;
    EditText        m_edtSearch;
    LinearLayout    m_llMainMenu;
    TextView        m_tvChannelNumber;
    TextView        m_tvChannelName;

    int             m_selectedMenuIndex;
    int             m_selectedChannelIndex;

    Handler         handler = new Handler();
    String          m_channelNumber = "";

    LiveMenuListAdapter     m_liveMenuAdapter;
    ChannelMenuListAdapter  m_channelAdapter;
    EpgListAdapter          m_epgAdapter;
    private PlayerView      playerView;
    private SimpleExoPlayer player;
    private TextView        tvMaintainence;

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

    private Handler qualities = new Handler();
    private Runnable qualitiesRunnable = new Runnable() {
        public void run() {
            updateGPAStatus(sessionID);
        }
    };

    private Content content = null;
    private MediaPlayer mMediaPlayer = null;
    private SurfaceView mSurfaceView = null;
    private SurfaceHolder mSurfaceHolder = null;

    private boolean playerReleased = false;
    private ProgressBar progressBar = null;
    private int state;

    private static final int IDLE = 0;
    private static final int BUFFERING = 1;
    private static final int PLAYING = 2;

    private String      m_path;
    private boolean     bIsSubscribedPackage;

    private DefaultTimeBar exo_progress;
    private ImageButton btnPlay;
    private ImageButton btnReplay;
    private ImageButton btnForward;
    private ImageButton btnPrev;
    private ImageButton btnNext;
    private TextView btnSwitchingLang;
    private ImageButton btnClosedCaption;
    private TextView tvTitle;

    ArrayChannelItem      m_channelList;
    ArrayChannelItem      m_filteredChannelList = new ArrayChannelItem();

    ArrayItemTopic m_epgList;
    private int sessionID;

    private Handler reconnectHandler = new Handler();
    private Runnable reconnectRunnable = new Runnable() {
        @Override
        public void run() {
            if(goalbitChannel) {
                updateGPAStatus(sessionID);
            }
            else {
                currentWindow = C.INDEX_UNSET;
                playbackPosition = C.INDEX_UNSET;
                initializePlayer();
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.live_layout);

        if (savedInstanceState == null) {
            playWhenReady = true;
            currentWindow = C.INDEX_UNSET;
            playbackPosition = C.INDEX_UNSET;
        } else {
            playWhenReady = savedInstanceState.getBoolean(KEY_PLAY_WHEN_READY);
            currentWindow = savedInstanceState.getInt(KEY_WINDOW);
            playbackPosition = savedInstanceState.getLong(KEY_POSITION);
        }

        shouldAutoPlay = true;
        bandwidthMeter = new DefaultBandwidthMeter();
        mediaDataSourceFactory = new DefaultDataSourceFactory(this, Util.getUserAgent(this, "mediaPlayerSample"), (TransferListener) bandwidthMeter);
        window = new Timeline.Window();

        initControl();
        setEventListener();
    }

    @Override
    public void onStart() {
        super.onStart();
        if (Util.SDK_INT > 23 && bIsSubscribedPackage) {
            initializePlayer();
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        if ((Util.SDK_INT <= 23 || player == null) && bIsSubscribedPackage) {
            initializePlayer();
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        stopGPASession();
        String valueStr = "";
        for (String channelName: AppConstants.FAVORITE_CHANNELS) {
            valueStr += channelName + ",";
        }
        Utils.setSharePreferenceValue(this, AppConstants.FAVORITE_ARRAY, valueStr);
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
    public void onBackPressed() {
        if(m_lvEpgList.getVisibility() == View.VISIBLE) {
            m_lvEpgList.setVisibility(View.GONE);
            return;
        }
        else if(m_lvChannelList.getVisibility() == View.VISIBLE) {
            m_lvChannelList.setVisibility(View.GONE);
            m_lvMainList.requestFocus();
            return;
        }
        else if(m_llMainMenu.getVisibility() == View.VISIBLE) {
            m_llMainMenu.setVisibility(View.GONE);
            return;
        }

        finish();
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
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if(keyCode == KeyEvent.KEYCODE_MENU && event.getAction() == KeyEvent.ACTION_DOWN) {
            if(m_llMainMenu.getVisibility() != View.VISIBLE) {
                m_llMainMenu.setVisibility(View.VISIBLE);
                m_lvMainList.setSelection(m_selectedMenuIndex);
                m_lvChannelList.setVisibility(View.VISIBLE);
                m_lvChannelList.setSelection(m_selectedChannelIndex);
                m_lvChannelList.requestFocus();
            }
            else {
                m_lvEpgList.setVisibility(View.GONE);
                m_lvChannelList.setVisibility(View.GONE);
                m_llMainMenu.setVisibility(View.GONE);
            }
            return true;
        }
        else if((keyCode == KeyEvent.KEYCODE_DPAD_CENTER || keyCode == KeyEvent.KEYCODE_ENTER) && event.getAction() == KeyEvent.ACTION_DOWN && m_llMainMenu.getVisibility() != View.VISIBLE) {
            if(findViewById(R.id.llSearchLayout).hasFocus()) {
                m_edtSearch.requestFocus();
            }
            else {
                m_llMainMenu.setVisibility(View.VISIBLE);
                m_lvMainList.setSelection(m_selectedMenuIndex);
                m_lvChannelList.setVisibility(View.VISIBLE);
                m_lvChannelList.setSelection(m_selectedChannelIndex);
                m_lvChannelList.requestFocus();
            }
            return true;
        }
        else if(keyCode >= KeyEvent.KEYCODE_0 && keyCode <= KeyEvent.KEYCODE_9 && event.getAction() == KeyEvent.ACTION_DOWN) {
            handler.removeCallbacks(runnable);
            m_channelNumber += String.valueOf(keyCode - KeyEvent.KEYCODE_0);
            m_tvChannelNumber.setText(m_channelNumber);
            m_tvChannelName.setText("");
            for (ChannelMenuItem item : m_channelList) {
                if(item.channelNumber.equals(m_channelNumber)) {
                    m_tvChannelName.setText(item.channelName);
                }
            }
            handler.postDelayed(runnable, 1000);
            return true;
        }
        else if(keyCode == KeyEvent.KEYCODE_DPAD_DOWN && event.getAction() == KeyEvent.ACTION_DOWN && m_llMainMenu.getVisibility() != View.VISIBLE) {
            if(m_selectedChannelIndex < m_filteredChannelList.size() - 1) {
                handler.removeCallbacks(runnable);
                m_selectedChannelIndex++;
                ChannelMenuItem item = m_filteredChannelList.get(m_selectedChannelIndex);
                m_channelNumber = item.channelNumber;
                m_tvChannelNumber.setText(item.channelNumber);
                m_tvChannelName.setText(item.channelName);

                handler.postDelayed(runnable, 1000);
                return true;
            }
        }
        else if(keyCode == KeyEvent.KEYCODE_DPAD_UP && event.getAction() == KeyEvent.ACTION_DOWN  && m_llMainMenu.getVisibility() != View.VISIBLE) {
            if(m_selectedChannelIndex < m_filteredChannelList.size() && m_selectedChannelIndex > 0) {
                handler.removeCallbacks(runnable);
                m_selectedChannelIndex--;
                ChannelMenuItem item = m_filteredChannelList.get(m_selectedChannelIndex);
                m_channelNumber = item.channelNumber;
                m_tvChannelNumber.setText(item.channelNumber);
                m_tvChannelName.setText(item.channelName);

                handler.postDelayed(runnable, 1000);
                return true;
            }
        }
        else if((keyCode == KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE) && event.getAction() == KeyEvent.ACTION_DOWN)
        {
            onPlay();
            return true;
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public void finish() {
        Intent intent = new Intent();
        if (m_path != null && !m_path.isEmpty()) {
            intent.putExtra(AppConstants.CURRENT_LIVE_CHANNEL, m_path);
        }
        setResult(RESULT_OK, intent);
        if (this.goalbitChannel) {
            stopGPASession();
        }
        super.finish();
    }

    private void stopGPASession() {
        Log.d("GoalBitPlusSDK.P2PPlayerActivity", "Stopping GPA session");

        if(mMediaPlayer != null) {
            if(state == PLAYING) {
                mMediaPlayer.stop();
            }

            mMediaPlayer.release();
            mMediaPlayer = null;
        }

        goalbitChannel =false;
        GoalBitPlus sdk = GoalBitPlus.getInstance();
        sdk.DeleteSession(sessionID);
        setState(IDLE);
        Log.d("GoalBitPlusSDK.P2PPlayerActivity", "GPA session stopped");
    }

    private void updateStartPosition() {
        if (player == null) return;

        playbackPosition = player.getCurrentPosition();
        currentWindow = player.getCurrentWindowIndex();
        playWhenReady = player.getPlayWhenReady();
    }

    private void initControl() {
        m_llMainMenu = (LinearLayout)findViewById(R.id.llMainMenu);
        m_tvChannelNumber = (TextView)findViewById(R.id.tvSelChannelNumber);
        m_tvChannelName = (TextView)findViewById(R.id.tvSelChannelName);
        m_edtSearch = (EditText)findViewById(R.id.edtSearch);
        m_lvMainList = findViewById(R.id.lvliveMainMenu);
        m_lvMainList.setChoiceMode(ListView.CHOICE_MODE_SINGLE);
        m_liveMenuAdapter = new LiveMenuListAdapter(R.layout.live_main_menu_item, this, LiveMenuActionItem.values());
        m_lvMainList.setAdapter(m_liveMenuAdapter);

        m_lvChannelList = findViewById(R.id.lvliveChannelMenu);
        m_lvChannelList.setChoiceMode(ListView.CHOICE_MODE_SINGLE);
        m_lvChannelList.requestFocus();

        progressBar = findViewById(R.id.progressBar);
        tvMaintainence = (TextView)findViewById(R.id.tvMaintainence);

        exo_progress = findViewById(R.id.exo_progress);
        btnPlay = (ImageButton) findViewById(R.id.btnPlay);
        btnReplay = (ImageButton) findViewById(R.id.btnReplay);
        btnForward = (ImageButton) findViewById(R.id.btnForward);
        btnPrev = (ImageButton) findViewById(R.id.btnPrev);
        btnNext = (ImageButton) findViewById(R.id.btnNext);
        btnSwitchingLang = (TextView) findViewById(R.id.btnSwitchingLang);
        btnClosedCaption = (ImageButton) findViewById(R.id.btnClosedCaption);
        tvTitle = (TextView) findViewById(R.id.tvTitle);

        exo_progress.setEnabled(false);
        btnPlay.setEnabled(false);
        btnReplay.setEnabled(false);
        btnForward.setEnabled(false);
        btnPrev.setEnabled(false);
        btnNext.setEnabled(false);
        btnSwitchingLang.setEnabled(false);

        m_path = getIntent().getStringExtra(AppConstants.LIVE_LAAST_CHANNEL_URI);
        m_channelList = AppConstants.CHANNEL_LIST;
        m_selectedChannelIndex = 0;
        int index = 0;
        m_filteredChannelList.clear();
        for (ChannelMenuItem item: m_channelList) {
            if(m_path.equals(item.channelURL)) {
                m_selectedChannelIndex = index;
                bIsSubscribedPackage = AppConstants.ROLES_LIST.contains(item.channelPackage);
                tvTitle.setText(item.channelName);
            }
            m_filteredChannelList.add(item);
            index++;
        }

        updateCurrentEpg();
        m_selectedMenuIndex = 1;

        m_channelAdapter = new ChannelMenuListAdapter(this, m_filteredChannelList, AppConstants.LIVE_CHANNEL_ADAPTER);
        m_channelAdapter.setActiveChannel(m_selectedChannelIndex);
        m_lvChannelList.setAdapter(m_channelAdapter);

        m_lvEpgList = findViewById(R.id.lvliveEpgMenu);

        m_epgList = new ArrayItemTopic();
        m_epgAdapter = new EpgListAdapter(this, m_epgList, AppConstants.EPG_LIST);

        m_lvEpgList.setAdapter(m_epgAdapter);

        m_lvMainList.setSelection(m_selectedMenuIndex);
        m_lvChannelList.setSelection(m_selectedChannelIndex);
        m_lvChannelList.requestFocus();
    }

    private void updateCurrentEpg() {
        Date currentDate = Utils.CurrentTime();
        for(int i = 0; i< m_filteredChannelList.size(); i ++) {
            ChannelMenuItem channelMenuItem = m_filteredChannelList.get(i);
            if(channelMenuItem.m_arrItemTopic.size() > 0) {
                for (EpgMenuItem topic : channelMenuItem.m_arrItemTopic) {
                    if(topic.m_dateTopicStart.after(currentDate) && currentDate.before(topic.m_dateTopicEnd)) {
                        String dayOfTheWeek = (String) DateFormat.format("EEEE", topic.m_dateTopicStart); // Thursday
                        String day          = (String) DateFormat.format("d",   topic.m_dateTopicStart); // 20
                        String monthNumber  = (String) DateFormat.format("MM",   topic.m_dateTopicStart); // 06

                        channelMenuItem.strTime = dayOfTheWeek + " " + day + "/" + monthNumber + " ";
                        channelMenuItem.strTitle = topic.strName;
                        channelMenuItem.strSubTitle = topic.strKind;
                        channelMenuItem.strTime = topic.strTime;
                        break;
                    }
                }
            }
        }
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btnPlay:
                onPlay();
                break;
            case R.id.btnForward:
                onForward();
                break;
            case R.id.btnReplay:
                onReplay();
                break;
            case R.id.btnNext:
                onNext();
                break;
            case R.id.btnPrev:
                onPrev();
                break;
            case R.id.btnSwitchingLang:
                onSwitchLang();
                break;
            case R.id.btnClosedCaption:
                onCC();
                break;
        }
    }

    private void setEventListener() {
        btnPlay.setOnClickListener(this);
        btnForward.setOnClickListener(this);
        btnReplay.setOnClickListener(this);
        btnPrev.setOnClickListener(this);
        btnNext.setOnClickListener(this);
        btnSwitchingLang.setOnClickListener(this);
        btnClosedCaption.setOnClickListener(this);
        m_edtSearch.setOnKeyListener(new View.OnKeyListener() {
            @Override
            public boolean onKey(View v, int keyCode, KeyEvent event) {
                if (keyCode == KeyEvent.KEYCODE_SEARCH || keyCode == KeyEvent.KEYCODE_ENTER) {
                    searchChannels();
                }

                return false;
            }
        });

        m_lvMainList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                m_selectedMenuIndex = position;
                m_liveMenuAdapter.setActiveMenuIndex(m_selectedMenuIndex);
                m_liveMenuAdapter.notifyDataSetChanged();
                filterGenre(position);
                m_lvChannelList.setVisibility(View.VISIBLE);
            }
        });

        m_lvChannelList.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                m_epgList.clear();
                m_epgAdapter.notifyDataSetChanged();
                Date currentDate = Utils.CurrentTime();
                for (EpgMenuItem topic: m_filteredChannelList.get(position).m_arrItemTopic) {
                    if(topic.m_dateTopicStart.after(currentDate) || topic.m_dateTopicEnd.after(currentDate))
                        m_epgList.add(topic);
                }
                m_epgAdapter.notifyDataSetChanged();
                m_lvEpgList.setVisibility(View.VISIBLE);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        m_lvChannelList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if(m_path.equals(m_filteredChannelList.get(position).channelURL))
                    return;

                handler.removeCallbacks(runnable);
                m_selectedChannelIndex = position;
                ChannelMenuItem item = m_filteredChannelList.get(m_selectedChannelIndex);
                m_channelNumber = item.channelNumber;
                m_tvChannelNumber.setText(item.channelNumber);
                m_tvChannelName.setText(item.channelName);

                m_llMainMenu.setVisibility(View.GONE);
                m_lvChannelList.setVisibility(View.GONE);
                m_lvEpgList.setVisibility(View.GONE);

                handler.postDelayed(runnable, 1000);
                return ;
            }
        });

        m_lvChannelList.setOnItemLongClickListener(new AdapterView.OnItemLongClickListener() {
            @Override
            public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
                if(AppConstants.FAVORITE_CHANNELS.contains(m_filteredChannelList.get(position).channelName))
                {
                    AppConstants.FAVORITE_CHANNELS.remove(m_filteredChannelList.get(position).channelName);
                    Toast.makeText(LiveActivity.this, "Removed to favorite channels", Toast.LENGTH_LONG);
                }
                else {
                    AppConstants.FAVORITE_CHANNELS.add(m_filteredChannelList.get(position).channelName);
                    Toast.makeText(LiveActivity.this, "Added to favorite channels", Toast.LENGTH_LONG);
                }

                m_channelAdapter.notifyDataSetChanged();
                return true;
            }
        });
    }

    private void searchChannels() {
        String filterString = m_edtSearch.getText().toString();
        m_lvEpgList.setVisibility(View.GONE);

        m_filteredChannelList.clear();
        m_channelAdapter.notifyDataSetChanged();
        for (ChannelMenuItem item: m_channelList) {
            if(item.channelGenre.contains(filterString))
                m_filteredChannelList.add(item);
        }
        m_channelAdapter.notifyDataSetChanged();
    }

    private void filterGenre(int position) {
        m_lvEpgList.setVisibility(View.GONE);

        String filterString = "";
        switch (position) {
            case 0:
                filterString = ""; //favorites
                break;
            case 1:
                filterString = ""; //todos
                break;
            case 2:
                filterString = getString(R.string.live_menu_noticias);
                break;
            case 3:
                filterString = getString(R.string.live_menu_esports);
                break;
            case 4:
                filterString = getString(R.string.live_menu_filmes);
                break;
            case 5:
                filterString = getString(R.string.live_menu_aberto);
                break;
            case 6:
                filterString = getString(R.string.live_menu_infantil);
                break;
            case 7:
                filterString = getString(R.string.live_menu_variedades);
                break;
            case 8:
                filterString = getString(R.string.live_menu_documentarios);
                break;
            case 9:
                filterString = getString(R.string.live_menu_adulto);
                break;
            case 10:
                filterString = getString(R.string.live_menu_religiosos);
                break;
            case 11:
                filterString = getString(R.string.live_menu_4k);
                break;
        }

        ChannelMenuItem oldItem = null;
        m_filteredChannelList.clear();
        m_channelAdapter.notifyDataSetChanged();

        for (ChannelMenuItem item: m_channelList) {
            if(position == 0) {
                if(AppConstants.FAVORITE_CHANNELS.contains(item.channelName))
                    m_filteredChannelList.add(item);
            }
            else if(item.channelGenre.contains(filterString) || filterString.isEmpty())
                m_filteredChannelList.add(item);

            if(item.channelURL.equals(m_path))
                oldItem = item;
        }

        if(oldItem != null) {
            m_selectedChannelIndex = m_filteredChannelList.indexOf(oldItem);
        }
        else
            m_selectedChannelIndex = -1;
        m_lvChannelList.setSelection(m_selectedChannelIndex);
        m_channelAdapter.setActiveChannel(m_selectedChannelIndex);
        updateCurrentEpg();
        m_channelAdapter.notifyDataSetChanged();
    }

    private void initializePlayer() {
        if(playerView == null) {
            playerView = findViewById(R.id.player_view);
        }

        if(mSurfaceView == null && m_path.substring(0, 10).equals("goalbit://")) {
            mSurfaceView = findViewById(R.id.video_surface);
            mSurfaceHolder = mSurfaceView.getHolder();
            if(Build.VERSION.SDK_INT <= 10) {
                mSurfaceHolder.setType(SURFACE_TYPE_PUSH_BUFFERS);
            }
            mSurfaceHolder.addCallback(this);
        }

        if(m_path.substring(0, 10).equals("goalbit://")) {
            configurePlayer();

            playerView.setVisibility(View.GONE);
            mSurfaceView.setVisibility(View.VISIBLE);
        }
        else {
            TrackSelection.Factory videoTrackSelectionFactory =
                    new AdaptiveTrackSelection.Factory(bandwidthMeter);

            trackSelector = new DefaultTrackSelector(videoTrackSelectionFactory);
            lastSeenTrackGroupArray = null;

            if(player == null) {
                // Here
                DefaultAllocator allocator = new DefaultAllocator(true, C.DEFAULT_BUFFER_SEGMENT_SIZE);
                DefaultLoadControl loadControl = new DefaultLoadControl(allocator, 360000, 600000, 2500, 5000, -1, true);
                //***

                player = ExoPlayerFactory.newSimpleInstance(new DefaultRenderersFactory(this), trackSelector, loadControl);
                if (mSurfaceView != null)
                    mSurfaceView.setVisibility(View.GONE);

                player.addListener(new PlayerEventListener());
                playerView.setPlayer(player);
                playerView.setControllerShowTimeoutMs(1000);

                player.setPlayWhenReady(shouldAutoPlay);
            }

            if(m_path == null || m_path.isEmpty())
                return;

            playerView.setVisibility(View.VISIBLE);
        }

        if(goalbitChannel)
            stopGPASession();

        if (m_path.substring(0, 10).equals("goalbit://")) {
            new LoadContentXmlTask().execute(m_path);
            return;
        }

        MediaSource mediaSource;
        if(m_path.endsWith(".m3u8"))
            mediaSource = new HlsMediaSource.Factory(mediaDataSourceFactory).createMediaSource(Uri.parse(Utils.makeAvaiableUrl(m_path)));
        else
            mediaSource = new ExtractorMediaSource.Factory(mediaDataSourceFactory).createMediaSource(Uri.parse(Utils.makeAvaiableUrl(m_path)));

        boolean haveStartPosition = currentWindow != C.INDEX_UNSET;
        if (haveStartPosition) {
            player.seekTo(currentWindow, playbackPosition);
        }

        player.prepare(mediaSource, false, false);
    }

    private void setVideoSize() {
        if(!playerReleased && mMediaPlayer != null) {
            int videoWidth = mMediaPlayer.getVideoWidth();
            int videoHeight = mMediaPlayer.getVideoHeight();
            float videoProportion = (float)videoWidth / (float)videoHeight;
            int screenWidth = getWindowManager().getDefaultDisplay().getWidth();
            int screenHeight = getWindowManager().getDefaultDisplay().getHeight();
            float screenProportion = (float)screenWidth / (float)screenHeight;
            ViewGroup.LayoutParams lp = mSurfaceView.getLayoutParams();
            if(videoProportion > screenProportion) {
                lp.width = screenWidth;
                lp.height = (int)((float)screenWidth / videoProportion);
            } else {
                lp.width = (int)(videoProportion * (float)screenHeight);
                lp.height = screenHeight;
            }

            mSurfaceView.setLayoutParams(lp);
        }
    }

    private void configurePlayer() {
        playerReleased = false;
        setState(IDLE);
        setupProgressDialog("Initializing streaming, please wait...");
        Log.d("GoalBitPlusSDK.P2PPlayerActivity", "Getting content information");
    }

    private void setupProgressDialog(String message) {
        if(!playerReleased || !goalbitChannel) {
//            progressDialog = ProgressDialog.show(this, "", message, true, false);
//            progressDialog.setOnKeyListener(new DialogInterface.OnKeyListener() {
//                public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
//                    if(keyCode == KeyEvent.KEYCODE_BACK) {
//                        releasePlayer();
//                        finish();
//                    }
//                    return false;
//                }
//            });
            progressBar.setVisibility(View.VISIBLE);
        }
    }

    @Override
    public void onBufferingUpdate(MediaPlayer mp, int percent) {

    }

    @Override
    public boolean onError(MediaPlayer mp, int what, int extra) {
        if(!playerReleased) {
            new Thread(new Runnable() {@Override
            public void run() {
                Log.e("onError", "Passed");
                if (Utils.getConnectivityStatus(LiveActivity.this) != Utils.TYPE_NOT_CONNECTED) {
//                    runOnUiThread(new Runnable() {@Override
//                    public void run() {
//                        progressBar.setVisibility(View.GONE);
//                        showMaintainence();
//                    }
//                    });
                }
                else {
                    mMediaPlayer.stop();
                    mMediaPlayer.release();
                    mMediaPlayer = null;
                    setState(IDLE);
                    goalbitChannel = false;
                    runOnUiThread(new Runnable() {
                        @Override
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
        return true;
    }

    @Override
    public boolean onInfo(MediaPlayer mp, int what, int extra) {
        return false;
    }

    @Override
    public void onPrepared(MediaPlayer mp) {
        Log.i("onPrepared", "passed" );
        if(!playerReleased) {
            Log.d("GoalBitPlusSDK.P2PPlayerActivity", "The MediaPlayer is prepared: " + mMediaPlayer.getVideoWidth() + "x" + mMediaPlayer.getVideoHeight());
            setState(PLAYING);
            setVideoSize();
            progressBar.setVisibility(View.GONE);

            mMediaPlayer.start();
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        if(!playerReleased) {
            if(content != null) {
                runOnUiThread(new Runnable() {
                    public void run() {
                        startGPASession(content);
                    }
                });
            }
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    private void setState(int inState) {
        switch(inState) {
            case IDLE:
                Log.d("GoalBitPlusSDK.P2PPlayerActivity", "GPA new state: IDLE");
                state = inState;
                sessionID = 0;
                break;
            case BUFFERING:
                Log.d("GoalBitPlusSDK.P2PPlayerActivity", "GPA new state: BUFFERING");
                state = inState;
                break;
            case PLAYING:
                Log.d("GoalBitPlusSDK.P2PPlayerActivity", "GPA new state: PLAYING");
                state = inState;
        }

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
                        progressBar.setVisibility(View.GONE);
                    }
                });
                return getResources().getString(R.string.connection_error);
            } catch (XmlPullParserException var5) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        progressBar.setVisibility(View.GONE);
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
        Log.e("MAXI", sessionStreamingURL);
        if(sessionStreamingURL != null && sessionStreamingURL.length() > 0) {
            try {
                progressBar.setVisibility(View.GONE);
                Log.e("GoalBitPlusSDK.P2PPlayerActivity", "Configuring the MediaPlayer");

//                MediaSource mediaSource;
//                mediaSource = new HlsMediaSource(Uri.parse(Utils.makeAvaiableUrl(sessionStreamingURL)), mediaDataSourceFactory, 5, null, null);
//
//                player.prepare(mediaSource, false, false);
                MediaPlayer.OnVideoSizeChangedListener var10 = new MediaPlayer.OnVideoSizeChangedListener() {
                    public void onVideoSizeChanged(MediaPlayer arg0, int arg1, int arg2) {
                        setVideoSize();
                    }
                };

                if(mMediaPlayer == null) {

                    mMediaPlayer = new MediaPlayer();
                    mMediaPlayer.setOnVideoSizeChangedListener(var10);
                    mMediaPlayer.setAudioStreamType(3);
                    mMediaPlayer.setOnBufferingUpdateListener(this);
                    mMediaPlayer.setOnErrorListener(this);
                    mMediaPlayer.setOnPreparedListener(this);
                    mMediaPlayer.setDisplay(mSurfaceHolder);
                    mMediaPlayer.setScreenOnWhilePlaying(true);
                    try {
                        mMediaPlayer.setDataSource(getApplicationContext(), Uri.parse(sessionStreamingURL));
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    mMediaPlayer.setOnInfoListener(this);
                }
                mMediaPlayer.prepareAsync();
            } catch (Exception var7) {
                var7.printStackTrace();
                Toast.makeText(this, "Error while starting the MediaPlayer", Toast.LENGTH_LONG).show();
                releasePlayer();
                return;
            }

            Log.e("GoalBitPlusSDK.P2PPlayerActivity", "Update GPA status: buffering completed");
        } else {

            //Log.d("GoalBitPlusSDK.P2PPlayerActivity", "Update GPA status: buffering " + sdk.getBufferStatus(sessionID) + " %");
            progressBar.setVisibility(View.VISIBLE);
            qualities.postDelayed(qualitiesRunnable, 5000);
        }
    }

    private  void onPlay() {
        if (player.getPlayWhenReady()) {
            onPauseVideo();
            if(btnPlay.getVisibility() == View.VISIBLE) {
                btnPlay.requestFocus();
            }
        } else {
            onPlayVideo();
        }
    }

    private  void onForward() {
        long curPos = player.getCurrentPosition();
        long duration = player.getDuration();
        long seekPos = (curPos + AppConstants.SEEK_OFFSET > duration) ? duration : curPos + AppConstants.SEEK_OFFSET;
        player.seekTo(seekPos);
    }

    private  void onReplay() {
        long curPos = player.getCurrentPosition();
        long seekPos = (curPos - AppConstants.SEEK_OFFSET < 0) ? 0 : curPos - AppConstants.SEEK_OFFSET;
        player.seekTo(seekPos);
    }

    private  void onPrev() {
        if(m_selectedChannelIndex < m_filteredChannelList.size() && m_selectedChannelIndex > 0) {
            handler.removeCallbacks(runnable);
            m_selectedChannelIndex--;
            ChannelMenuItem item = m_filteredChannelList.get(m_selectedChannelIndex);
            m_channelNumber = item.channelNumber;
            m_tvChannelNumber.setText(item.channelNumber);
            m_tvChannelName.setText(item.channelName);

            handler.postDelayed(runnable, 1000);
        }
    }

    private void onPlayVideo() {
        if (player != null)
            player.setPlayWhenReady(true);
        btnPlay.setImageResource(R.drawable.player_pause);
    }

    private void onPauseVideo() {
        if (player != null)
            player.setPlayWhenReady(false);
        btnPlay.setImageResource(R.drawable.player_play);
    }

    private  void onNext() {
        if(m_selectedChannelIndex < m_filteredChannelList.size() - 1) {
            handler.removeCallbacks(runnable);
            m_selectedChannelIndex++;
            ChannelMenuItem item = m_filteredChannelList.get(m_selectedChannelIndex);
            m_channelNumber = item.channelNumber;
            m_tvChannelNumber.setText(item.channelNumber);
            m_tvChannelName.setText(item.channelName);

            handler.postDelayed(runnable, 1000);
        }
    }

    private void onSwitchLang() {
        if(trackSelector.getParameters().preferredAudioLanguage.equals("eng")) {
            trackSelector.setParameters(new DefaultTrackSelector.ParametersBuilder().setPreferredAudioLanguage("por"));
            btnSwitchingLang.setText("po");
        }
        else {
            trackSelector.setParameters(new DefaultTrackSelector.ParametersBuilder().setPreferredAudioLanguage("eng"));
            btnSwitchingLang.setText("en");
        }
    }

    private void onCC() {
        if(playerView.getSubtitleView().getVisibility() == View.VISIBLE) {
            playerView.getSubtitleView().setVisibility(View.GONE);
        }
        else {

            playerView.getSubtitleView().setVisibility(View.VISIBLE);
        }
    }

    private void releasePlayer() {
        if (player != null) {
            updateStartPosition();
            shouldAutoPlay = player.getPlayWhenReady();
            player.release();
            player = null;
            trackSelector = null;
        }

        if(reconnectHandler != null) {
            reconnectHandler.removeCallbacks(reconnectRunnable);
        }

        if(qualities != null) {
            qualities.removeCallbacks(qualitiesRunnable);
        }

        if(handler != null) {
            handler.removeCallbacks(runnable);
        }

        if(goalbitChannel) {
            Log.d("GoalBitPlusSDK.P2PPlayerActivity", "Releasing Player");
            stopGPASession();

            if(progressBar != null) {
                progressBar.setVisibility(View.GONE);
            }

            playerReleased = true;
            Log.d("GoalBitPlusSDK.P2PPlayerActivity", "The Player was released");
        }
    }

    private void checkPlayAdult(final ChannelMenuItem item) {
        final Dialog protectionDlg = new Dialog(this, R.style.Theme_CustomDialog);
        LayoutInflater inflater = ((LayoutInflater) getSystemService(LAYOUT_INFLATER_SERVICE));
        View protectionView = inflater.inflate(R.layout.protection_dialog, null, false);
        TextView  headView = (TextView) protectionView.findViewById(R.id.tvDescription);
        headView.setText("Please Confirm Password");
        ImageView userImage = (ImageView) protectionView.findViewById(R.id.ivUserType);
        userImage.setImageResource(R.drawable.user);
        protectionView.findViewById(R.id.edtPassword).setOnKeyListener(new View.OnKeyListener() {
            @Override
            public boolean onKey(View v, int keyCode, KeyEvent event) {
                if((keyCode == KeyEvent.KEYCODE_DPAD_CENTER || keyCode == KeyEvent.KEYCODE_ENTER) && event.getAction() == KeyEvent.ACTION_UP) {
                    String strSavedPassword = Utils.getSharePreferenceValue(LiveActivity.this, AppConstants.ADULT_PASSWORD, "1234");
                    if(strSavedPassword.equals(((EditText)v).getText().toString())) {
                        m_path = item.channelURL;
                        tvTitle.setText(item.channelName);
                        releasePlayer();

                        playWhenReady = true;
                        currentWindow = C.INDEX_UNSET;
                        playbackPosition = C.INDEX_UNSET;

                        shouldAutoPlay = true;
                        bandwidthMeter = new DefaultBandwidthMeter();
                        mediaDataSourceFactory = new DefaultDataSourceFactory(getApplicationContext(), Util.getUserAgent(getApplicationContext(), "mediaPlayerSample"), (TransferListener) bandwidthMeter);
                        window = new Timeline.Window();

                        initializePlayer();

                        int index = 0;
                        m_selectedChannelIndex = -1;
                        for (ChannelMenuItem item : m_filteredChannelList) {
                            if(item.channelNumber.equals(m_channelNumber) && item.channelName.equals(m_tvChannelName.getText().toString())) {
                                m_selectedChannelIndex = index;
                                break;
                            }
                            index++;
                        }

                        m_lvChannelList.setSelection(m_selectedChannelIndex);
                        m_channelAdapter.setActiveChannel(m_selectedChannelIndex);
                        m_channelAdapter.notifyDataSetChanged();
                    }
                    else {
                        Toast.makeText(LiveActivity.this, "Invalid Password!", Toast.LENGTH_SHORT).show();
                    }
                    protectionDlg.dismiss();
                    return true;
                }

                return false;
            }
        });
        protectionDlg.setOnDismissListener(new DialogInterface.OnDismissListener() {
            @Override
            public void onDismiss(DialogInterface dialog) {
                m_channelNumber = "";
                m_tvChannelNumber.setText(m_channelNumber);
                m_tvChannelName.setText("");
            }
        });
        protectionDlg.setContentView(protectionView);
        protectionDlg.getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);
        protectionDlg.show();
    }

    private Runnable runnable = new Runnable() {
        @Override
        public void run() {
            int index = 0;

            for (ChannelMenuItem item : m_channelList) {
                if(item.channelNumber.equals(m_channelNumber) && item.channelName.equals(m_tvChannelName.getText().toString())) {
                    if(!m_path.equals(item.channelURL) && !m_path.equals(item.channelP2PUrl)) {
                        if(AppConstants.ROLES_LIST.contains(item.channelPackage))
                            bIsSubscribedPackage = true;
                        else {
                            bIsSubscribedPackage = false;
                            break;
                        }
                        if(item.channelGenre.equals(getString(R.string.live_menu_adulto))) {
                            checkPlayAdult(item);
                            return;
                        }
                        m_path = item.channelURL;
                        tvTitle.setText(item.channelName);

						if (AppConstants.isP2P && item.channelFeed.equals("p2p") && !item.channelP2PUrl.isEmpty()) {
                            m_path = item.channelP2PUrl;
                        }

                        if(m_path.isEmpty() && !item.channelFeed.equals("p2p")) {
                            Utils.showToast(LiveActivity.this, R.string.backup_url_error);
                            m_path = item.channelP2PUrl;
                        }

                        releasePlayer();

                        playWhenReady = true;
                        currentWindow = C.INDEX_UNSET;
                        playbackPosition = C.INDEX_UNSET;

                        shouldAutoPlay = true;
                        bandwidthMeter = new DefaultBandwidthMeter();
                        mediaDataSourceFactory = new DefaultDataSourceFactory(getApplicationContext(), Util.getUserAgent(getApplicationContext(), "mediaPlayerSample"), (TransferListener) bandwidthMeter);
                        window = new Timeline.Window();

                        initializePlayer();
                    }
                    break;
                }
                index++;
            }

            if(bIsSubscribedPackage) {
                index = 0;
                m_selectedChannelIndex = -1;
                for (ChannelMenuItem item : m_filteredChannelList) {
                    if (item.channelNumber.equals(m_channelNumber) && item.channelName.equals(m_tvChannelName.getText().toString())) {
                        m_selectedChannelIndex = index;
                        break;
                    }
                    index++;
                }

                m_lvChannelList.setSelection(m_selectedChannelIndex);
                m_channelAdapter.setActiveChannel(m_selectedChannelIndex);
                m_channelAdapter.notifyDataSetChanged();
            }
            else {
                Utils.showToast(LiveActivity.this, R.string.package_error);
            }

            m_channelNumber = "";
            m_tvChannelNumber.setText(m_channelNumber);
            m_tvChannelName.setText("");
            bIsSubscribedPackage = true;
        }
    };

    private class PlayerEventListener extends ExoPlayer.DefaultEventListener {
        @Override
        public void onPlayerStateChanged(boolean playWhenReady, int playbackState) {
            if (playbackState == Player.STATE_BUFFERING) {
                setupProgressDialog("Loading... Please wait");
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
                    if (Utils.getConnectivityStatus(LiveActivity.this) != Utils.TYPE_NOT_CONNECTED) {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                progressBar.setVisibility(View.GONE);
//                                showMaintainence();
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
