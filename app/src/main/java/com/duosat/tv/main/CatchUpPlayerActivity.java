package com.duosat.tv.main;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.ImageButton;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.duosat.tv.R;
import com.duosat.tv.model.VodVideoItem;
import com.duosat.tv.utils.AppConstants;
import com.duosat.tv.utils.Utils;
import com.github.se_bastiaan.torrentstream.StreamStatus;
import com.github.se_bastiaan.torrentstream.Torrent;
import com.google.android.exoplayer2.C;
import com.google.android.exoplayer2.DefaultLoadControl;
import com.google.android.exoplayer2.DefaultRenderersFactory;
import com.google.android.exoplayer2.ExoPlaybackException;
import com.google.android.exoplayer2.ExoPlayer;
import com.google.android.exoplayer2.ExoPlayerFactory;
import com.google.android.exoplayer2.Format;
import com.google.android.exoplayer2.Player;
import com.google.android.exoplayer2.SimpleExoPlayer;
import com.google.android.exoplayer2.Timeline;
import com.google.android.exoplayer2.source.BehindLiveWindowException;
import com.google.android.exoplayer2.source.ExtractorMediaSource;
import com.google.android.exoplayer2.source.MediaSource;
import com.google.android.exoplayer2.source.MergingMediaSource;
import com.google.android.exoplayer2.source.SingleSampleMediaSource;
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
import com.google.android.exoplayer2.util.MimeTypes;
import com.google.android.exoplayer2.util.Util;

@SuppressLint("SetTextI18n")
public class CatchUpPlayerActivity extends Activity implements View.OnClickListener {
    private static final String KEY_PLAY_WHEN_READY = "play_when_ready";
    private static final String KEY_WINDOW = "window";
    private static final String KEY_POSITION = "position";
    private static final String TORRENT = "Torrent";

    private ProgressBar     progressBar;

    private PlayerView playerView;
    private SimpleExoPlayer player;

    private Timeline.Window         window;
    private DataSource.Factory      mediaDataSourceFactory;
    private DefaultTrackSelector    trackSelector;
    private boolean                 shouldAutoPlay;
    private BandwidthMeter          bandwidthMeter;

    private boolean     playWhenReady;
    private int         currentWindow;
    private long        playbackPosition;

    private String      streamUrl;
    private String      programmeName;

    private DefaultTimeBar exo_progress;
    private ImageButton btnPlay;
    private ImageButton btnReplay;
    private ImageButton btnForward;
    private ImageButton btnPrev;
    private ImageButton btnNext;
    private TextView btnSwitchingLang;
    private ImageButton btnClosedCaption;
    private TextView tvTitle;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video_player);

        initControl();
        setEventListener();


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
        mediaDataSourceFactory = new DefaultDataSourceFactory(this, Util.getUserAgent(this, getString(R.string.app_name)), (TransferListener<? super DataSource>) bandwidthMeter);
        window = new Timeline.Window();

        streamUrl = getIntent().getStringExtra(AppConstants.DIRECT_URL);
        programmeName = getIntent().getStringExtra(AppConstants.PROGRAMME_NAME);
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
    public void finish() {
        super.finish();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        updateStartPosition();

        outState.putBoolean(KEY_PLAY_WHEN_READY, playWhenReady);
        outState.putInt(KEY_WINDOW, currentWindow);
        outState.putLong(KEY_POSITION, playbackPosition);
        super.onSaveInstanceState(outState);
    }

    private void updateStartPosition() {
        if (player == null) return;

        playbackPosition = player.getCurrentPosition();
        currentWindow = player.getCurrentWindowIndex();
        playWhenReady = player.getPlayWhenReady();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event){
        if(event.getAction() != KeyEvent.ACTION_DOWN)
            return true;
        switch (keyCode) {
            case KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE:
            {
                onPlay();
                return true;
            }
            case KeyEvent.KEYCODE_DPAD_CENTER:
            case KeyEvent.KEYCODE_ENTER:
                playerView.showController();
                return true;
            case KeyEvent.KEYCODE_MEDIA_STEP_FORWARD:
            case KeyEvent.KEYCODE_MEDIA_SKIP_FORWARD:
                long curPos = player.getCurrentPosition();
                long duration = player.getDuration();
                long seekPos = (curPos + AppConstants.SEEK_OFFSET > duration) ? duration : curPos + AppConstants.SEEK_OFFSET;
                player.seekTo(seekPos);
                return true;
            case KeyEvent.KEYCODE_MEDIA_STEP_BACKWARD:
            case KeyEvent.KEYCODE_MEDIA_SKIP_BACKWARD:
                curPos = player.getCurrentPosition();
                seekPos = (curPos - AppConstants.SEEK_OFFSET < 0) ? 0 : curPos - AppConstants.SEEK_OFFSET;
                player.seekTo(seekPos);
                return true;
            default:
                return super.onKeyDown(keyCode, event);
        }
    }

    public void initControl() {
        progressBar = findViewById(R.id.progress);

        exo_progress = findViewById(R.id.exo_progress);
        btnPlay = (ImageButton) findViewById(R.id.btnPlay);
        btnReplay = (ImageButton) findViewById(R.id.btnReplay);
        btnForward = (ImageButton) findViewById(R.id.btnForward);
        btnPrev = (ImageButton) findViewById(R.id.btnPrev);
        btnNext = (ImageButton) findViewById(R.id.btnNext);
        btnSwitchingLang = (TextView) findViewById(R.id.btnSwitchingLang);
        btnClosedCaption = (ImageButton) findViewById(R.id.btnClosedCaption);
        tvTitle = (TextView) findViewById(R.id.tvTitle);

        btnPrev.setVisibility(View.GONE);
        btnNext.setVisibility(View.GONE);
        btnSwitchingLang.setVisibility(View.GONE);
        btnClosedCaption.setVisibility(View.GONE);
    }

    private void setEventListener() {
        btnPlay.setOnClickListener(this);
        btnForward.setOnClickListener(this);
        btnReplay.setOnClickListener(this);
        btnPrev.setOnClickListener(this);
        btnNext.setOnClickListener(this);
        btnSwitchingLang.setOnClickListener(this);
        btnClosedCaption.setOnClickListener(this);
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

    private void initializePlayer() {
        if(player == null) {
            progressBar.setVisibility(View.GONE);

            playerView = findViewById(R.id.player_view);

            TrackSelection.Factory videoTrackSelectionFactory =
                    new AdaptiveTrackSelection.Factory(bandwidthMeter);

            trackSelector = new DefaultTrackSelector(videoTrackSelectionFactory);

            // Here
            DefaultAllocator allocator = new DefaultAllocator(true, C.DEFAULT_BUFFER_SEGMENT_SIZE);
            DefaultLoadControl loadControl = new DefaultLoadControl(allocator, 360000, 600000, 2500, 5000, -1, true);
            //***

            player = ExoPlayerFactory.newSimpleInstance(new DefaultRenderersFactory(this), trackSelector, loadControl);

            playerView.setPlayer(player);

            playerView.setControllerShowTimeoutMs(1000);

            player.setPlayWhenReady(shouldAutoPlay);
            player.addListener(new PlayerEventListener());
        }

        tvTitle.setText(programmeName);

        MediaSource mediaSource;
        if(streamUrl.endsWith(".m3u8"))
            mediaSource = new HlsMediaSource.Factory(mediaDataSourceFactory).createMediaSource(Uri.parse(Utils.makeAvaiableUrl(streamUrl)));
        else
            mediaSource = new ExtractorMediaSource.Factory(mediaDataSourceFactory).createMediaSource(Uri.parse(Utils.makeAvaiableUrl(streamUrl)));

        boolean haveStartPosition = currentWindow != C.INDEX_UNSET;
        if (haveStartPosition) {
            player.seekTo(currentWindow, playbackPosition);
        }

        player.prepare(mediaSource, !haveStartPosition, false);

        playerView.getSubtitleView().setVisibility(View.VISIBLE);
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

    }

    private void reloadVideo() {
        releasePlayer();

        playWhenReady = true;
        currentWindow = 0;
        playbackPosition = 0;

        shouldAutoPlay = true;
        bandwidthMeter = new DefaultBandwidthMeter();
        mediaDataSourceFactory = new DefaultDataSourceFactory(this, Util.getUserAgent(this, "mediaPlayerSample"), (TransferListener<? super DataSource>) bandwidthMeter);
        window = new Timeline.Window();

        initControlState();

        initializePlayer();
    }

    private void initControlState() {
        btnPrev.setEnabled(false);
        btnNext.setEnabled(false);
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
        if(playerView.getSubtitleView().getVisibility() == View.VISIBLE)
            playerView.getSubtitleView().setVisibility(View.GONE);
        else
            playerView.getSubtitleView().setVisibility(View.VISIBLE);
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
                findViewById(R.id.pbProgress).setVisibility(View.VISIBLE);
            }
            if (playbackState == Player.STATE_READY) {
                findViewById(R.id.pbProgress).setVisibility(View.GONE);
            }
        }

        @Override
        public void onPlayerError(ExoPlaybackException e) {
            if (isBehindLiveWindow(e)) {
                currentWindow = C.INDEX_UNSET;
                playbackPosition = C.INDEX_UNSET;
                initializePlayer();
            }
            else {
                findViewById(R.id.pbProgress).setVisibility(View.GONE);
            }
        }
    }
}