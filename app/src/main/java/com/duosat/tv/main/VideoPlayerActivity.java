package com.duosat.tv.main;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.media.MediaCodec;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.support.v4.app.ActivityCompat;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.ImageButton;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.duosat.tv.R;
import com.duosat.tv.model.VodVideoItem;
import com.duosat.tv.utils.AppConstants;
import com.duosat.tv.utils.DataResult;
import com.duosat.tv.utils.Utils;
import com.github.se_bastiaan.torrentstream.StreamStatus;
import com.github.se_bastiaan.torrentstream.Torrent;
import com.github.se_bastiaan.torrentstream.TorrentOptions;
import com.github.se_bastiaan.torrentstream.TorrentStream;
import com.github.se_bastiaan.torrentstream.listeners.TorrentListener;
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
import com.google.android.exoplayer2.audio.MediaCodecAudioRenderer;
import com.google.android.exoplayer2.mediacodec.MediaCodecRenderer;
import com.google.android.exoplayer2.mediacodec.MediaCodecSelector;
import com.google.android.exoplayer2.source.BehindLiveWindowException;
import com.google.android.exoplayer2.source.ExtractorMediaSource;
import com.google.android.exoplayer2.source.MediaSource;
import com.google.android.exoplayer2.source.MergingMediaSource;
import com.google.android.exoplayer2.source.SingleSampleMediaSource;
import com.google.android.exoplayer2.source.TrackGroup;
import com.google.android.exoplayer2.source.TrackGroupArray;
import com.google.android.exoplayer2.source.hls.HlsMediaSource;
import com.google.android.exoplayer2.trackselection.AdaptiveTrackSelection;
import com.google.android.exoplayer2.trackselection.DefaultTrackSelector;
import com.google.android.exoplayer2.trackselection.TrackSelection;
import com.google.android.exoplayer2.trackselection.TrackSelector;
import com.google.android.exoplayer2.ui.DefaultTimeBar;
import com.google.android.exoplayer2.ui.PlayerView;
import com.google.android.exoplayer2.ui.SimpleExoPlayerView;
import com.google.android.exoplayer2.upstream.BandwidthMeter;
import com.google.android.exoplayer2.upstream.DataSource;
import com.google.android.exoplayer2.upstream.DefaultAllocator;
import com.google.android.exoplayer2.upstream.DefaultBandwidthMeter;
import com.google.android.exoplayer2.upstream.DefaultDataSourceFactory;
import com.google.android.exoplayer2.upstream.TransferListener;
import com.google.android.exoplayer2.util.MimeTypes;
import com.google.android.exoplayer2.util.Util;

import org.w3c.dom.Text;

import java.util.ArrayList;

import static com.google.android.exoplayer2.util.MimeTypes.*;

@SuppressLint("SetTextI18n")
public class VideoPlayerActivity extends Activity implements TorrentListener, View.OnClickListener {
    private static final String KEY_PLAY_WHEN_READY = "play_when_ready";
    private static final String KEY_WINDOW = "window";
    private static final String KEY_POSITION = "position";
    private static final String TORRENT = "Torrent";

    private ProgressBar     progressBar;
    private TorrentStream   torrentStream;

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

    private boolean     isDirectURL;

    private String streamUrl;

    private DefaultTimeBar exo_progress;
    private ImageButton btnPlay;
    private ImageButton btnReplay;
    private ImageButton btnForward;
    private ImageButton btnPrev;
    private ImageButton btnNext;
    private TextView btnSwitchingLang;
    private ImageButton btnClosedCaption;
    private TextView tvTitle;

    ArrayList<VodVideoItem> playList;

    int playPosition = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video_player);

        initControl();
        setEventListener();

        playList = DataResult.getInstance().getMediaData();
        playPosition = getIntent().getIntExtra(AppConstants.CURRENT_POSITION, 0);

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

        streamUrl = getIntent().getStringExtra(AppConstants.TORRENT_URL);

        isDirectURL = false;

        String directUrl = getIntent().getStringExtra(AppConstants.DIRECT_URL);

        if(directUrl != null && !directUrl.isEmpty()) {
            streamUrl = directUrl;
            if(!streamUrl.isEmpty()) {
                isDirectURL = true;
//                initializePlayer();
            }
        }
        else if(!streamUrl.isEmpty()) {
            TorrentOptions torrentOptions = new TorrentOptions.Builder()
                    .saveLocation(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS))
                    .removeFilesAfterStop(true)
                    .autoDownload(true)
                    .build();

            torrentStream = TorrentStream.init(torrentOptions);
            torrentStream.addListener(this);

            progressBar.setProgress(0);
            progressBar.setMax(100);
            torrentStream.startStream(streamUrl);
        }
    }

    @Override
    public void onStart() {
        super.onStart();
        if (Util.SDK_INT > 23 && isDirectURL) {
            initializePlayer();
        }
    }

    @Override
    public void onResume() {
        super.onResume();

//        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
//            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, 0);
//        }

        if ((Util.SDK_INT <= 23 || player == null) && isDirectURL) {
            initializePlayer();
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        if (Util.SDK_INT <= 23 && isDirectURL) {
            releasePlayer();
        }
    }

    @Override
    public void onStop() {
        super.onStop();

        if (Util.SDK_INT > 23 && isDirectURL) {
            releasePlayer();
        }
    }

    @Override
    public void finish() {
        if(torrentStream != null && torrentStream.isStreaming()) {
            torrentStream.stopStream();
        }
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

        streamUrl = playList.get(playPosition).directURL;

        tvTitle.setText(playList.get(playPosition).videoName);

        MediaSource mediaSource;
        if(streamUrl.endsWith(".m3u8"))
            mediaSource = new HlsMediaSource.Factory(mediaDataSourceFactory).createMediaSource(Uri.parse(Utils.makeAvaiableUrl(streamUrl)));
        else
            mediaSource = new ExtractorMediaSource.Factory(mediaDataSourceFactory).createMediaSource(Uri.parse(Utils.makeAvaiableUrl(streamUrl)));

        boolean haveStartPosition = currentWindow != C.INDEX_UNSET;
        if (haveStartPosition) {
            player.seekTo(currentWindow, playbackPosition);
        }

        if(!playList.get(playPosition).srtUrl.isEmpty()) {
            Format textFormat = Format.createTextSampleFormat(null, MimeTypes.APPLICATION_SUBRIP,
                    null, Format.NO_VALUE, Format.NO_VALUE, "pt_BR", null, Format.OFFSET_SAMPLE_RELATIVE);
            MediaSource textMediaSource = new SingleSampleMediaSource(Uri.parse(playList.get(playPosition).srtUrl), mediaDataSourceFactory, textFormat, C.TIME_UNSET);
            MediaSource mediaSourceWithText = new MergingMediaSource(mediaSource, textMediaSource);

            player.prepare(mediaSourceWithText, !haveStartPosition, false);
        }
        else
            player.prepare(mediaSource, !haveStartPosition, false);

        trackSelector.setParameters(new DefaultTrackSelector.ParametersBuilder().setPreferredAudioLanguage("por"));

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
        if (playPosition > 0) {
            playPosition--;
            reloadVideo();
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
        if (playPosition < playList.size() - 1) {
            playPosition++;
            reloadVideo();
        }
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
        if (playList.size() > 0) {
            btnPrev.setEnabled(playPosition != 0);
            btnNext.setEnabled(playPosition != (playList.size() - 1));
        }

        VodVideoItem vodVideoItem = playList.get(playPosition);

        if (!vodVideoItem.videoName.isEmpty())
            tvTitle.setText(vodVideoItem.videoName);
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

    /************ Torrent Listener **********/

    @Override
    public void onStreamPrepared(Torrent torrent) {
        Log.d(TORRENT, "OnStreamPrepared");
//         If you set TorrentOptions#autoDownload(false) then this is probably the place to call
//         torrent.startDownload();
    }

    @Override
    public void onStreamStarted(Torrent torrent) {
        Log.d(TORRENT, "onStreamStarted");
    }

    @Override
    public void onStreamError(Torrent torrent, Exception e) {
        Log.e(TORRENT, "onStreamError", e);
    }

    @Override
    public void onStreamReady(Torrent torrent) {
        progressBar.setProgress(100);
        streamUrl = torrent.getVideoFile().toString();
        isDirectURL = true;

        Log.d(TORRENT, "onStreamReady: " + torrent.getVideoFile());

//        Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(torrent.getVideoFile().toString()));
//        intent.setDataAndType(Uri.parse(torrent.getVideoFile().toString()), "video/mp4");
//        startActivity(intent);
        initializePlayer();
    }

    @Override
    public void onStreamProgress(Torrent torrent, StreamStatus status) {
        if(status.bufferProgress <= 100 && progressBar.getProgress() < 100 && progressBar.getProgress() != status.bufferProgress) {
            Log.d(TORRENT, "Progress: " + status.bufferProgress);
            progressBar.setProgress(status.bufferProgress);
        }
    }

    @Override
    public void onStreamStopped() {
        Log.d(TORRENT, "onStreamStopped");
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