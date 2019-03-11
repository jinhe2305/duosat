package com.duosat.tv.adapter;

import android.app.Activity;
import android.graphics.Color;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.duosat.tv.R;
import com.duosat.tv.model.MenuActionItem;

public class MenuListAdapter extends ArrayAdapter<MenuActionItem> {

    int resource;
    Activity activity;
    int m_activeMenuindex;

    public MenuListAdapter(int resource, Activity activity, MenuActionItem[] items) {
        super(activity, resource, items);

        this.resource = resource;
        this.activity = activity;

        this.m_activeMenuindex = 0;
    }

    public View getView (int position, View convertView, ViewGroup parent) {
        View rowView = convertView;

        if(rowView == null) {
            rowView = activity.getLayoutInflater().inflate(resource, null);

            MenuItemViewHolder viewHolder = new MenuItemViewHolder();

            viewHolder.menuItemImageView = (ImageView)rowView.findViewById(R.id.menu_item_image_view);
            viewHolder.menuItemTextView = (TextView)rowView.findViewById(R.id.menu_item_text_view);

            rowView.setTag(viewHolder);
        }

        MenuItemViewHolder holder = (MenuItemViewHolder)rowView.getTag();

        holder.menuItemTextView.setTextColor(Color.WHITE);

        if(position == MenuActionItem.FILMS.ordinal()) {
            if(position == m_activeMenuindex) {
                holder.menuItemTextView.setTextColor(Color.RED);
                holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.filmes_vod_active));
            }
            else
                holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.filmes_vod));
            holder.menuItemTextView.setText(activity.getString(R.string.menu_films));
        }
        else if(position == MenuActionItem.SERIES.ordinal()) {
            if(position == m_activeMenuindex) {
                holder.menuItemTextView.setTextColor(Color.RED);
                holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.series_vod_active));
            }
            else
                holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.series_vod));
            holder.menuItemTextView.setText(activity.getString(R.string.menu_series));
        }
        else if(position == MenuActionItem.INFANTIL.ordinal()) {
            if(position == m_activeMenuindex) {
                holder.menuItemTextView.setTextColor(Color.RED);
                holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.infantil_active));
            }
            else
                holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.infantil));
            holder.menuItemTextView.setText(activity.getString(R.string.menu_infantil));
        }
        else if(position == MenuActionItem.ANIMES.ordinal()) {
            if(position == m_activeMenuindex) {
                holder.menuItemTextView.setTextColor(Color.RED);
                holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.animes_active));
            }
            else
                holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.animes));
            holder.menuItemTextView.setText(activity.getString(R.string.menu_animes));
        }
        else if(position == MenuActionItem.VIDEOTECA.ordinal()) {
            if(position == m_activeMenuindex) {
                holder.menuItemTextView.setTextColor(Color.RED);
                holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.videoteca));
            }
            else
                holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.videoteca_normal));
            holder.menuItemTextView.setText(activity.getString(R.string.menu_noticas));
        }
        else if(position == MenuActionItem.LANCEMEMTOS.ordinal()) {
            if(position == m_activeMenuindex) {
                holder.menuItemTextView.setTextColor(Color.RED);
                holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.lancamentos_active));
            }
            else
                holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.lancamentos));
            holder.menuItemTextView.setText(activity.getString(R.string.menu_lancamentos));
        }

        return rowView;
    }

    private static class MenuItemViewHolder {
        public ImageView menuItemImageView;
        public TextView menuItemTextView;
    }

    public void setActiveMenuIndex(int activeIndex) {
        m_activeMenuindex = activeIndex;
    }
}
