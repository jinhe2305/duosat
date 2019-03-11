package com.duosat.tv.main;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.text.format.DateFormat;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.duosat.tv.R;
import com.duosat.tv.adapter.LiveMenuListAdapter;
import com.duosat.tv.http.AppController;
import com.duosat.tv.model.ChannelMenuItem;
import com.duosat.tv.model.EpgMenuItem;
import com.duosat.tv.model.LiveMenuActionItem;
import com.duosat.tv.utils.AppConstants;
import com.duosat.tv.utils.Utils;
import com.duosat.tv.view.EPGView;
import com.google.android.exoplayer2.C;
import com.google.android.exoplayer2.DefaultLoadControl;
import com.google.android.exoplayer2.DefaultRenderersFactory;
import com.google.android.exoplayer2.ExoPlaybackException;
import com.google.android.exoplayer2.ExoPlayer;
import com.google.android.exoplayer2.ExoPlayerFactory;
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
import com.squareup.picasso.Picasso;
import com.squareup.picasso.Target;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

public class CatchUpTVActivity extends Activity{

    private static final String KEY_PLAY_WHEN_READY = "play_when_ready";
    private static final String KEY_WINDOW = "window";
    private static final String KEY_POSITION = "position";

    private final static int    UPDATE_TIME_DELAY = 1 * 1000;

    private ImageView   ivCurrentChannelLogo;
    private TextView    tvCurrentChannelName, tvCurrentChannelGenre;
    private TextView    tvCurrentEPGDuration, tvCurrentEPGMark, tvCurrentEPGDescription;

    private ListView                lvGenreList;
    private LiveMenuListAdapter     liveMenuAdapter;
    private TextView                tvTodayView;
    private LinearLayout            llSelectDay;
    private TextView                tvCurrentDateTime;

    private EPGView     epgView;

    Handler handler = new Handler();
    private PlayerView playerView;
    private SimpleExoPlayer player;

    private ProgressBar     progressBar;
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

    private String      m_path;

    View                m_oldFocusView;

    private Handler reconnectHandler = new Handler();
    private Runnable reconnectRunnable = new Runnable() {
        @Override
        public void run() {
            currentWindow = C.INDEX_UNSET;
            playbackPosition = C.INDEX_UNSET;
            initializePlayer();
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.catchup_tv_layout);

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

        for (ChannelMenuItem channelItem : AppConstants.CATCHUP_TV_CHANNEL_LIST) {
            AppConstants.CATCHUP_TV_FILTERED_CAHNNEL_LIST.add(channelItem);
        }

        epgView.setEPGData(AppConstants.CATCHUP_TV_FILTERED_CAHNNEL_LIST);
        epgView.recalculateAndRedraw(false);

        if(AppConstants.CATCHUP_TV_FILTERED_CAHNNEL_LIST.size() > 0)
            m_path = AppConstants.CATCHUP_TV_FILTERED_CAHNNEL_LIST.get(0).channelURL;

        updateSelectedTopicInfo();
    }

    @Override
    public void onStart() {
        super.onStart();
        if (Util.SDK_INT > 23) {
            initializePlayer();
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        if ((Util.SDK_INT <= 23 || player == null)) {
            initializePlayer();
        }

        if(m_oldFocusView != null)
            m_oldFocusView.requestFocus();
    }

    @Override
    public void onPause() {
        super.onPause();
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
    }

    private void updateStartPosition() {
        if (player == null) return;

        playbackPosition = player.getCurrentPosition();
        currentWindow = player.getCurrentWindowIndex();
        playWhenReady = player.getPlayWhenReady();
    }

    private void initControl() {
        ivCurrentChannelLogo = (ImageView) findViewById(R.id.ID_CURRENT_CHANNEL_LOGO);
        tvCurrentChannelName = (TextView)findViewById(R.id.ID_TEXT_CHANNEL_NAME);
        tvCurrentChannelGenre = (TextView)findViewById(R.id.ID_TEXT_CHANNEL_GENRE);

        tvCurrentEPGDuration = (TextView)findViewById(R.id.ID_TEXT_EPG_DURATION);
        tvCurrentEPGMark = (TextView)findViewById(R.id.ID_TEXT_EPG_MARK);
        tvCurrentEPGDescription = (TextView)findViewById(R.id.ID_TEXT_EPG_DESCRIPTION);

        lvGenreList = (ListView)findViewById(R.id.ID_GENRE_LIST);
        lvGenreList.setChoiceMode(ListView.CHOICE_MODE_SINGLE);
        liveMenuAdapter = new LiveMenuListAdapter(R.layout.live_main_menu_item, this, LiveMenuActionItem.values());
        lvGenreList.setAdapter(liveMenuAdapter);
        epgView = (EPGView)findViewById(R.id.ID_CATUP_TV_VIEW);
        tvTodayView = (TextView)findViewById(R.id.ID_TEXT_TODAY);
        llSelectDay = (LinearLayout)findViewById(R.id.ID_CATUP_TV_DATE_BAR);
        tvCurrentDateTime = (TextView)findViewById(R.id.ID_CURRENT_DATE_TIME);

        progressBar = (ProgressBar)findViewById(R.id.pbProgress);
        tvMaintainence = (TextView)findViewById(R.id.tvMaintainence);

        buildCatchUpTVDateBar();
    }

    private void buildCatchUpTVDateBar() {
        llSelectDay.removeAllViews();
        long startTime = Utils.CurrentTime().getTime() - 6 * 24 * 3600 * 1000;
        Date date = new Date(startTime);

        for(int i = 0; i < 14; i++)
        {
            String dayOfTheWeek = (String) DateFormat.format("EE", date); // Thursday
            String day          = (String) DateFormat.format("d",   date); // 20
            LinearLayout columnView = (LinearLayout) getLayoutInflater().inflate(R.layout.item_catchup_date, null);
            TextView dayView = (TextView)columnView.findViewById(R.id.ID_CATUP_TV_DAY);
            dayView.setText(dayOfTheWeek);
            TextView dateView = (TextView)columnView.findViewById(R.id.ID_CATUP_TV_DATE);
            dateView.setText(day);
            llSelectDay.addView(columnView);

            startTime += 24 * 3600 * 1000;
            date.setTime(startTime);
        }
    }

    //------------------------------------------------------------------------------
    public void updateSelectedTopicInfo() {
        if (epgView == null || AppConstants.CATCHUP_TV_FILTERED_CAHNNEL_LIST.size() == 0)
            return;
        final ChannelMenuItem selectedChannel = AppConstants.CATCHUP_TV_FILTERED_CAHNNEL_LIST.get(epgView.getSelectedChannelIndex());

        if (epgView.getSelectedTopicIndex() < 0)
            return;
        EpgMenuItem selectedTopic = selectedChannel.m_arrItemTopic.get(epgView.getSelectedTopicIndex());

        Date epgStartDate = selectedTopic.m_dateTopicStart;
        String day = (String) DateFormat.format("d",   epgStartDate);

        for(int i = 0; i < 14; i++)
        {
            View columnView = llSelectDay.getChildAt(i);
            TextView dateView = (TextView)columnView.findViewById(R.id.ID_CATUP_TV_DATE);
            if(dateView.getText().equals(day)) {
                columnView.setBackgroundResource(R.color.listview_back_color);
            }
            else
                columnView.setBackgroundColor(Color.TRANSPARENT);
        }

        tvCurrentChannelName.setText(selectedTopic.strName);
        tvCurrentChannelGenre.setText(selectedTopic.strKind);
        tvCurrentEPGDuration.setText(selectedTopic.strTime);
        tvCurrentEPGDescription.setText(selectedTopic.strDescription);

        if (AppConstants.CHANNEL_IMAGE_CACHE.containsKey(selectedChannel.channelSrc)) {
            Bitmap image = AppConstants.CHANNEL_IMAGE_CACHE.get(selectedChannel.channelSrc);
            Drawable drawable = new BitmapDrawable(getResources(), image);
            ivCurrentChannelLogo.setBackground(drawable);
        } else {
            if (!AppConstants.CHANNEL_IMAGE_CACHE.containsKey(selectedChannel.channelSrc)) {
                AppConstants.CHANNEL_IMAGE_TARGET_CACHE.put(selectedChannel.channelSrc, new Target() {
                    @Override
                    public void onBitmapLoaded(Bitmap bitmap, Picasso.LoadedFrom from) {
                        AppConstants.CHANNEL_IMAGE_CACHE.put(selectedChannel.channelSrc, bitmap);
                        Drawable drawable = new BitmapDrawable(getResources(), bitmap);
                        ivCurrentChannelLogo.setBackground(drawable);
                        AppConstants.CHANNEL_IMAGE_TARGET_CACHE.remove(selectedChannel.channelSrc);
                    }

                    @Override
                    public void onBitmapFailed(Exception e, Drawable errorDrawable) {

                    }

                    @Override
                    public void onPrepareLoad(Drawable placeHolderDrawable) {

                    }
                });

                if(selectedChannel.channelSrc != null &&!selectedChannel.channelSrc.isEmpty())
                    Utils.loadImageInto(CatchUpTVActivity.this, selectedChannel.channelSrc, getResources().getDimensionPixelOffset(R.dimen.img_size_44dp),
                            getResources().getDimensionPixelSize(R.dimen.img_size_33dp), AppConstants.CHANNEL_IMAGE_TARGET_CACHE.get(selectedChannel.channelSrc));
            }

        }
    }

    public void setFocusOnMenu() {
        lvGenreList.requestFocus();
    }

    private void setEventListener() {
        lvGenreList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                liveMenuAdapter.setActiveMenuIndex(position);
                liveMenuAdapter.notifyDataSetChanged();
                filterGenre(position);
            }
        });
    }

    private void filterGenre(int position) {
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

        AppConstants.CATCHUP_TV_FILTERED_CAHNNEL_LIST.clear();

        for (ChannelMenuItem item: AppConstants.CATCHUP_TV_CHANNEL_LIST) {
            if(position == 0) {
                if(AppConstants.FAVORITE_CHANNELS.contains(item.channelName))
                    AppConstants.CATCHUP_TV_FILTERED_CAHNNEL_LIST.add(item);
            }
            else if(item.channelGenre.contains(filterString) || filterString.isEmpty())
                AppConstants.CATCHUP_TV_FILTERED_CAHNNEL_LIST.add(item);
        }

        epgView.recalculateAndRedraw(false);
    }

    private void initializePlayer() {
        if(player == null) {
            playerView = findViewById(R.id.ID_HEAD_VIDEO_VIEW);

            TrackSelection.Factory videoTrackSelectionFactory =
                    new AdaptiveTrackSelection.Factory(bandwidthMeter);

            trackSelector = new DefaultTrackSelector(videoTrackSelectionFactory);
            lastSeenTrackGroupArray = null;

            // Here
            DefaultAllocator allocator = new DefaultAllocator(true, C.DEFAULT_BUFFER_SEGMENT_SIZE);
            DefaultLoadControl loadControl = new DefaultLoadControl(allocator, 360000, 600000, 2500, 5000, -1, true);
            //***

            player = ExoPlayerFactory.newSimpleInstance(new DefaultRenderersFactory(this), trackSelector, loadControl);
            player.addListener(new PlayerEventListener());

            playerView.setPlayer(player);

            playerView.setControllerShowTimeoutMs(1000);

            player.setPlayWhenReady(shouldAutoPlay);
        }

        if(m_path == null || m_path.isEmpty())
            return;

        playerView.setVisibility(View.VISIBLE);
        playerView.setUseController(true);

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

        tvCurrentDateTime.setText(mStrTime);
    }

    private Runnable runnableUpdateTime = new Runnable() {
        @Override
        public void run() {
            setTimeInfo();
            handler.postDelayed(runnableUpdateTime, UPDATE_TIME_DELAY);
        }
    };

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
            player.stop();
            playerView.setVisibility(View.INVISIBLE);
            playerView.setUseController(false);
            new Thread(new Runnable() {@Override
            public void run() {
                if (Utils.getConnectivityStatus(CatchUpTVActivity.this) != Utils.TYPE_NOT_CONNECTED)
                    runOnUiThread(new Runnable() {@Override
                    public void run() {
                        progressBar.setVisibility(View.GONE);
                        showMaintainence();
                    }
                    });
                else
                    runOnUiThread(new Runnable() {@Override
                    public void run() {
                        progressBar.setVisibility(View.GONE);
                        showNetworkError();
                    }
                    });
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
