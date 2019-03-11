package com.duosat.tv.main;


import android.app.Activity;
import android.app.Dialog;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.view.Window;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.duosat.tv.R;
import com.duosat.tv.adapter.RecyclerScreenShotAdapter;
import com.duosat.tv.model.VodVideoItem;
import com.duosat.tv.utils.AppConstants;
import com.duosat.tv.utils.DataResult;
import com.duosat.tv.utils.Utils;

public class VideoDetailsDialog extends Dialog {
    private Activity m_activity;
    private VodVideoItem m_vodVideoItem;

    private TextView m_tvName;
    private TextView m_tvYear;
    private TextView m_tvNumber;
    private TextView m_tvGenre;
    private TextView m_tvMark;
    private TextView m_tvDescription;
    private TextView m_tvDirector;
    private TextView m_tvStars;
    private TextView m_tvIdioms;

    private RelativeLayout m_rl;
    private ImageView m_ivVideoImage;

    private RecyclerView m_rvScreenShot;
    private RecyclerView.Adapter m_rvAdapter;

    private int position = 0;

    public VideoDetailsDialog(Activity context, int position) {
        super(context);

        m_activity = context;
        this.position = position;
        m_vodVideoItem = DataResult.getInstance().getMediaData().get(position);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.movie_detail_dialog);

        initControl();
    }

    private void initControl() {
        m_tvName = (TextView) findViewById(R.id.ID_TEXT_NAME);
        m_tvYear = (TextView) findViewById(R.id.ID_TEXT_YEAR);
        m_tvNumber = (TextView) findViewById(R.id.ID_TEXT_NUMBER);
        m_tvGenre = (TextView) findViewById(R.id.ID_TEXT_GENRE);
        m_tvMark = (TextView) findViewById(R.id.ID_TEXT_MARK);
        m_tvDescription = (TextView) findViewById(R.id.ID_TEXT_DESCRIPTION);
        m_tvDirector = (TextView) findViewById(R.id.ID_TEXT_DIRECTOR);
        m_tvStars = (TextView) findViewById(R.id.ID_TEXT_STARS);
        m_tvIdioms = (TextView) findViewById(R.id.ID_TEXT_IDIOMS);
        m_rl = (RelativeLayout) findViewById(R.id.ID_RL);
        m_rl.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View v, boolean hasFocus) {
                if(hasFocus)
                    v.setBackgroundColor(Color.WHITE);
                else
                    v.setBackgroundColor(Color.TRANSPARENT);
            }
        });
        m_rl.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(m_activity, VideoPlayerActivity.class);
                if(!m_vodVideoItem.videoURL.isEmpty())
                    intent.putExtra(AppConstants.TORRENT_URL, m_vodVideoItem.videoURL);
                if(!m_vodVideoItem.directURL.isEmpty())
                    intent.putExtra(AppConstants.DIRECT_URL, m_vodVideoItem.directURL);

                intent.putExtra(AppConstants.CURRENT_POSITION, position);
                m_activity.startActivity(intent);
                dismiss();
            }
        });

        m_ivVideoImage = (ImageView) findViewById(R.id.ID_VIDEO_IMAGE);

        m_tvName.setText(m_vodVideoItem.videoName);
        m_tvDescription.setText(m_vodVideoItem.videoLongDes);
        m_tvGenre.setText(m_vodVideoItem.category);

        if (m_vodVideoItem.thumbnail != null && !m_vodVideoItem.thumbnail.isEmpty()) {
            Utils.setNetworkImage(m_ivVideoImage, m_vodVideoItem.thumbnail, R.drawable.item_video, Utils.IMG_TRANSFORM_FILL, "VideoThumbnail");
        } else {
            m_ivVideoImage.setImageResource(R.drawable.logo_middle);
        }

        m_rvScreenShot = (RecyclerView) findViewById(R.id.ID_SCREEN_SHOTS);

        // The number of Columns
        RecyclerView.LayoutManager layoutManager = new LinearLayoutManager(m_activity, LinearLayoutManager.HORIZONTAL, false);
        m_rvScreenShot.setLayoutManager(layoutManager);

        m_rvAdapter = new RecyclerScreenShotAdapter(m_activity, m_vodVideoItem, "Screenshot");
        m_rvScreenShot.setAdapter(m_rvAdapter);

        m_rl.requestFocus();
    }
}
