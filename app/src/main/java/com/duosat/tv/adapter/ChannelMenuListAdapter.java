package com.duosat.tv.adapter;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.util.TypedValue;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.duosat.tv.R;
import com.duosat.tv.model.ChannelMenuItem;
import com.duosat.tv.model.LiveMenuActionItem;
import com.duosat.tv.model.VodVideoItem;
import com.duosat.tv.utils.AppConstants;
import com.duosat.tv.utils.Utils;
import com.squareup.picasso.Picasso;
import com.squareup.picasso.Target;

import org.json.JSONArray;
import org.json.JSONException;

import java.util.ArrayList;

public class ChannelMenuListAdapter extends BaseAdapter {

    Activity activity;
    private ArrayList<ChannelMenuItem> channelInfoList;

    String  tag;
    int     m_activeChannel;

    public ChannelMenuListAdapter(Activity activity, ArrayList<ChannelMenuItem> channelInfoList, String tag) {
        this.activity = activity;
        this.channelInfoList = channelInfoList;
        this.tag = tag;
        this.m_activeChannel = 0;
    }

    @Override
    public int getCount() {
        return channelInfoList.size();
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
            rowView = activity.getLayoutInflater().inflate(R.layout.live_channel_menu_item, null);

            MenuItemViewHolder viewHolder = new MenuItemViewHolder();

            viewHolder.ivchannelThumbView = (ImageView)rowView.findViewById(R.id.ivChannelThumb);
            viewHolder.tvchannelNumberView = (TextView)rowView.findViewById(R.id.tvchannelNumber);
            viewHolder.tvchannelNameView = (TextView)rowView.findViewById(R.id.tvchannelName);
            viewHolder.ivisLikeView = (ImageView)rowView.findViewById(R.id.ivLike);
            viewHolder.tvtitleView = (TextView) rowView.findViewById(R.id.tvTitle);
            viewHolder.tvsubTitleView = (TextView) rowView.findViewById(R.id.tvSubTitle);
            viewHolder.tvtimeView = (TextView)rowView.findViewById(R.id.tvTime);
            viewHolder.llepgView = (LinearLayout)rowView.findViewById(R.id.lldetails);

            rowView.setTag(viewHolder);
        }

        final MenuItemViewHolder holder = (MenuItemViewHolder)rowView.getTag();

        final ChannelMenuItem channelInfo = channelInfoList.get(position);

        if (channelInfo.channelSrc != null && !channelInfo.channelSrc.isEmpty()) {
            if (AppConstants.CHANNEL_IMAGE_CACHE.containsKey(channelInfo.channelSrc)) {
                Bitmap image = AppConstants.CHANNEL_IMAGE_CACHE.get(channelInfo.channelSrc);
                holder.ivchannelThumbView.setImageBitmap(image);
            }
            else {
                if (!AppConstants.CHANNEL_IMAGE_CACHE.containsKey(channelInfo.channelSrc)) {
                    AppConstants.CHANNEL_IMAGE_TARGET_CACHE.put(channelInfo.channelSrc, new Target() {
                        @Override
                        public void onBitmapLoaded(Bitmap bitmap, Picasso.LoadedFrom from) {
                            AppConstants.CHANNEL_IMAGE_CACHE.put(channelInfo.channelSrc, bitmap);
                            holder.ivchannelThumbView.setImageBitmap(bitmap);
                            AppConstants.CHANNEL_IMAGE_TARGET_CACHE.remove(channelInfo.channelSrc);
                        }

                        @Override
                        public void onBitmapFailed(Exception e, Drawable errorDrawable) {

                        }

                        @Override
                        public void onPrepareLoad(Drawable placeHolderDrawable) {

                        }
                    });

                    if (channelInfo.channelSrc != null && !channelInfo.channelSrc.isEmpty())
                        Utils.loadImageInto(activity, channelInfo.channelSrc, activity.getResources().getDimensionPixelOffset(R.dimen.img_size_44dp),
                                activity.getResources().getDimensionPixelOffset(R.dimen.img_size_33dp), AppConstants.CHANNEL_IMAGE_TARGET_CACHE.get(channelInfo.channelSrc));
                }
            }
        }
        else {
            holder.ivchannelThumbView.setImageResource(R.drawable.logo_middle);
        }

        if (channelInfo.channelNumber != null && !channelInfo.channelNumber.isEmpty()) {
            holder.tvchannelNumberView.setText(channelInfo.channelNumber);
        } else {
            holder.tvchannelNumberView.setText("");
        }

        if (channelInfo.channelName != null && !channelInfo.channelName.isEmpty()) {
            holder.tvchannelNameView.setText(channelInfo.channelName);
        } else {
            holder.tvchannelNameView.setText("");
        }

        if (AppConstants.FAVORITE_CHANNELS.contains(channelInfo.channelName)) {
            holder.ivisLikeView.setImageResource(R.drawable.videoteca);
        } else {
            holder.ivisLikeView.setImageResource(R.drawable.videoteca_normal);
        }

        if (channelInfo.strTitle != null && !channelInfo.strTitle.isEmpty()) {
            holder.tvtitleView.setText(channelInfo.strTitle);
        } else {
            holder.tvtitleView.setText("No Programme");
        }

        if (channelInfo.strSubTitle != null && !channelInfo.strSubTitle.isEmpty()) {
            holder.tvsubTitleView.setText(channelInfo.strSubTitle);
        } else {
            holder.tvsubTitleView.setText("");
        }

        if (channelInfo.strTime != null && !channelInfo.strTime.isEmpty()) {
            holder.tvtimeView.setText(channelInfo.strTime);
        } else {
            holder.tvtimeView.setText("");
        }

        if(position == m_activeChannel) {
            holder.tvchannelNumberView.setTextSize(TypedValue.COMPLEX_UNIT_PX, activity.getResources().getDimension(R.dimen.font_40sp));
            holder.tvchannelNameView.setTextSize(TypedValue.COMPLEX_UNIT_PX, activity.getResources().getDimension(R.dimen.font_15sp));

            holder.tvtitleView.setTextAppearance(activity, R.style.fontBold);
            holder.tvsubTitleView.setTextAppearance(activity, R.style.fontBold);
            holder.tvtimeView.setTextAppearance(activity, R.style.fontBold);
        }
        else {
            holder.tvchannelNumberView.setTextSize(TypedValue.COMPLEX_UNIT_PX, activity.getResources().getDimension(R.dimen.font_10sp));
            holder.tvchannelNameView.setTextSize(TypedValue.COMPLEX_UNIT_PX, activity.getResources().getDimension(R.dimen.font_10sp));
            holder.tvtitleView.setTextAppearance(activity, R.style.fontNormal);
            holder.tvsubTitleView.setTextAppearance(activity, R.style.fontNormal);
            holder.tvtimeView.setTextAppearance(activity, R.style.fontNormal);
        }

        return rowView;
    }

    public void setActiveChannel(int activeChannel) {
        m_activeChannel = activeChannel;
    }

    private static class MenuItemViewHolder {
        public ImageView    ivchannelThumbView;
        public TextView     tvchannelNumberView;
        public TextView     tvchannelNameView;
        public ImageView    ivisLikeView;
        public TextView     tvtitleView;
        public TextView     tvsubTitleView;
        public TextView     tvtimeView;
        public LinearLayout llepgView;
    }
}
