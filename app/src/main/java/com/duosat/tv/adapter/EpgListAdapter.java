package com.duosat.tv.adapter;

import android.app.Activity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.duosat.tv.R;
import com.duosat.tv.model.ArrayItemTopic;
import com.duosat.tv.model.ChannelMenuItem;
import com.duosat.tv.model.EpgMenuItem;
import com.duosat.tv.utils.Utils;

import java.util.ArrayList;

public class EpgListAdapter extends BaseAdapter {

    Activity activity;
    private ArrayList<EpgMenuItem> epgInfoList;

    String  tag;
    int     m_oldFocusIndex;

    public EpgListAdapter(Activity activity, ArrayItemTopic channelInfoList, String tag) {
        this.activity = activity;
        this.epgInfoList = channelInfoList;
        this.tag = tag;
        this.m_oldFocusIndex = -1;
    }

    @Override
    public int getCount() {
        return epgInfoList.size();
    }

    @Override
    public Object getItem(int position) {
        return null;
    }

    @Override
    public long getItemId(int position) {
        return 0;
    }

    public View getView (int position, View convertView, ViewGroup parent) {
        View rowView = convertView;

        if(rowView == null) {
            rowView = activity.getLayoutInflater().inflate(R.layout.live_channel_epg_item, null);

            MenuItemViewHolder viewHolder = new MenuItemViewHolder();

            viewHolder.tvNameView = (TextView) rowView.findViewById(R.id.tvName);
            viewHolder.tvTimeView = (TextView)rowView.findViewById(R.id.tvTime);
            viewHolder.tvKindView = (TextView) rowView.findViewById(R.id.tvKind);
            viewHolder.tvDescriptionView = (TextView) rowView.findViewById(R.id.tvDescription);

            rowView.setTag(viewHolder);
        }

//        if (position == activity.getCurIndex()) {
//            rowView.setBackgroundColor(activity.getResources().getColor(R.color.theme_color));
//        } else {
//            rowView.setBackgroundResource(R.drawable.menu_item);
//        }

        final MenuItemViewHolder holder = (MenuItemViewHolder)rowView.getTag();

        final EpgMenuItem epgInfo = epgInfoList.get(position);

//        holder.ivVideo.setScaleType(ImageView.ScaleType.CENTER_INSIDE);
        if (epgInfo.strName != null && !epgInfo.strName.isEmpty()) {
            holder.tvNameView.setText(epgInfo.strName);
        } else {
            holder.tvNameView.setText("");
        }

        String strTime = epgInfo.strTime;
        if(epgInfo.strVideoLength != null && !epgInfo.strVideoLength.isEmpty())
            strTime += " | " + epgInfo.strVideoLength;

        if(epgInfo.strMark != null && !epgInfo.strMark.isEmpty())
            strTime += " | " + epgInfo.strMark;

        if (strTime != null && !strTime.isEmpty()) {
            holder.tvTimeView.setText(strTime);
        } else {
            holder.tvTimeView.setText("");
        }

        if (epgInfo.strKind != null && !epgInfo.strKind.isEmpty()) {
            holder.tvKindView.setText(epgInfo.strKind);
        } else {
            holder.tvKindView.setText("");
        }

        if (epgInfo.strDescription != null && !epgInfo.strDescription.isEmpty()) {
            holder.tvDescriptionView.setText(epgInfo.strDescription);
        } else {
            holder.tvDescriptionView.setText("");
        }

        return rowView;
    }

    private static class MenuItemViewHolder {
        public TextView     tvNameView;
        public TextView     tvTimeView;
        public TextView     tvKindView;
        public TextView     tvDescriptionView;
    }
}
