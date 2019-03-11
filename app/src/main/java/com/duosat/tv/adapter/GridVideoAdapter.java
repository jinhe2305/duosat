package com.duosat.tv.adapter;

import android.content.Context;
import android.content.Intent;
import android.provider.ContactsContract;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.TextView;

import com.duosat.tv.R;
import com.duosat.tv.main.VodActivity;
import com.duosat.tv.model.VodVideoItem;
import com.duosat.tv.utils.Utils;

import java.util.ArrayList;

public class GridVideoAdapter extends BaseAdapter {
    private final static String VIDEO = "video";

    private VodActivity activity;
    private ArrayList<VodVideoItem> videoInfoList;
    String tag;
    int     m_oldFocusIndex;
    public GridVideoAdapter(VodActivity activity, ArrayList<VodVideoItem> videoInfoList, String tag) {
        this.activity = activity;
        this.videoInfoList = videoInfoList;
        this.tag = tag;
        this.m_oldFocusIndex = -1;
    }

    public View getView(final int position, View convertView, final ViewGroup parent) {
        if (convertView == null) {
            LayoutInflater inflater = (LayoutInflater) activity
                    .getSystemService(Context.LAYOUT_INFLATER_SERVICE);

            convertView = inflater.inflate(R.layout.item_video, null);

            final ViewHolder viewHolder = new ViewHolder();

            viewHolder.ivVideo = (ImageView) convertView.findViewById(R.id.ivVideo);
            viewHolder.ivDeu = (ImageView) convertView.findViewById(R.id.ivDeu);
            viewHolder.ivNew = (ImageView) convertView.findViewById(R.id.ivNew);
            viewHolder.ivLike = (ImageView) convertView.findViewById(R.id.ivLike);
            viewHolder.tvVideoName = (TextView) convertView.findViewById(R.id.tvVideoName);

            viewHolder.ivVideo.setFocusable(false);
            viewHolder.ivDeu.setFocusable(false);
            viewHolder.ivNew.setFocusable(false);
            viewHolder.ivLike.setFocusable(false);
            viewHolder.tvVideoName.setFocusable(false);

            convertView.setTag(viewHolder);
            convertView.setFocusable(false);
        }

        final ViewHolder holder = (ViewHolder)convertView.getTag();

        final VodVideoItem videoInfo = videoInfoList.get(position);

        if (videoInfo.thumbnail != null && !videoInfo.thumbnail.isEmpty()) {
            Utils.setNetworkImage(holder.ivVideo, videoInfo.thumbnail, R.drawable.item_video, Utils.IMG_TRANSFORM_FILL, tag);
        } else {
            holder.ivVideo.setImageResource(R.drawable.logo_middle);
        }

        if (videoInfo.videoisDeu) {
            holder.ivDeu.setImageResource(R.drawable.dub_btn);
        } else {
            holder.ivDeu.setImageResource(R.drawable.cat_btn);
        }

        if (videoInfo.videoIsNew) {
            holder.ivNew.setImageResource(R.drawable.new_btn);
        } else {
            holder.ivNew.setImageResource(R.drawable.cat_btn);
        }

        if (videoInfo.videoIsLike) {
            holder.ivLike.setImageResource(R.drawable.videoteca);
        } else {
            holder.ivLike.setImageResource(R.drawable.videoteca);
        }

        if (videoInfo.videoName != null && !videoInfo.videoName.isEmpty())
            holder.tvVideoName.setText(videoInfo.videoName);

        convertView.setOnLongClickListener(new View.OnLongClickListener() {
            @Override
            public boolean onLongClick(View v) {
                m_oldFocusIndex = position;
                return true;
            }
        });

        convertView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

            }
        });

        convertView.setOnKeyListener(new View.OnKeyListener() {
            @Override
            public boolean onKey(View v, int keyCode, KeyEvent event) {
            int colNumber = ((GridView) parent).getNumColumns();
            if (event.getAction() == KeyEvent.ACTION_DOWN && keyCode == KeyEvent.KEYCODE_DPAD_DOWN) {
                if (position + colNumber < videoInfoList.size()) {
                    ((GridView) parent).setSelection(position + colNumber);
                    ((GridView) parent).smoothScrollToPosition(position + colNumber);
                } else if (position % colNumber > (videoInfoList.size() - 1) % colNumber) {
                    ((GridView) parent).setSelection(videoInfoList.size() - 1);
                    ((GridView) parent).smoothScrollToPosition(videoInfoList.size() - 1);
                }
                return true;
            }
            else if (event.getAction() == KeyEvent.ACTION_DOWN && keyCode == KeyEvent.KEYCODE_DPAD_UP) {
                if (position - colNumber > -1) {
                    ((GridView) parent).setSelection(position - colNumber);
                    ((GridView) parent).smoothScrollToPosition(position - colNumber);
                    return true;
                }
            }
            return false;
            }
        });

        if(m_oldFocusIndex == position) {
            convertView.requestFocus();
            m_oldFocusIndex = -1;
        }

        return convertView;
    }

    private static class ViewHolder {
        public ImageView ivVideo;
        public ImageView ivDeu;
        public ImageView ivNew;
        public ImageView ivLike;
        public TextView  tvVideoName;
    }

    @Override
    public int getCount() {
        return videoInfoList.size();
    }

    @Override
    public Object getItem(int position) {
        return null;
    }

    @Override
    public long getItemId(int position) {
        return 0;
    }

}

