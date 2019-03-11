package com.duosat.tv.adapter;

import android.app.Activity;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import com.duosat.tv.R;
import com.duosat.tv.model.VodVideoItem;
import com.duosat.tv.utils.Utils;


public class RecyclerScreenShotAdapter extends RecyclerView.Adapter<RecyclerScreenShotAdapter.ViewHolder> {
    VodVideoItem vodVideoItem;
    Activity activity;
    String tag;

    public RecyclerScreenShotAdapter(Activity activity, VodVideoItem vodVideoItem, String tag) {
        super();
        this.activity = activity;
        this.vodVideoItem = vodVideoItem;
        this.tag = tag;
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup viewGroup, int i) {
        View v = LayoutInflater.from(viewGroup.getContext())
                .inflate(R.layout.item_screenshot, viewGroup, false);

        ViewHolder viewHolder = new ViewHolder(v);
        ViewGroup.LayoutParams layoutParams = v.getLayoutParams();
        v.setLayoutParams(layoutParams);
        return viewHolder;
    }

    @Override
    public void onBindViewHolder(final ViewHolder viewHolder, final int i) {
        viewHolder.setIsRecyclable(false);
        final String screenShot = vodVideoItem.arrayScreenShot.get(i);

        if (screenShot != null && !screenShot.isEmpty()) {
            Utils.setNetworkImage(viewHolder.ivVideo, screenShot, R.drawable.item_video, Utils.IMG_TRANSFORM_FILL, tag);
        } else {
            viewHolder.ivVideo.setImageResource(R.drawable.item_video);
        }

        viewHolder.itemView.setFocusable(true);
    }

    @Override
    public int getItemCount() {
        return vodVideoItem.arrayScreenShot.size();
    }

    public static class ViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener, View.OnLongClickListener {

        public ImageView ivVideo;

        public ViewHolder(View itemView) {
            super(itemView);
            ivVideo = (ImageView) itemView.findViewById(R.id.ivVideo);
        }

        @Override
        public void onClick(View v) {

        }

        @Override
        public boolean onLongClick(View v) {
            return false;
        }
    }

}
