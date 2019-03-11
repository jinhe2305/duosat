package com.duosat.tv.adapter;

import android.app.Activity;
import android.graphics.Color;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.duosat.tv.R;
import com.duosat.tv.model.LiveMenuActionItem;
import com.duosat.tv.model.MenuActionItem;

public class LiveMenuListAdapter extends ArrayAdapter<LiveMenuActionItem> {

    int resource;
    Activity activity;

    int m_activeMenuindex;

    public LiveMenuListAdapter(int resource, Activity activity, LiveMenuActionItem[] items) {
        super(activity, resource, items);

        this.resource = resource;
        this.activity = activity;
        this.m_activeMenuindex = 1;
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

        if(position == LiveMenuActionItem.FAVORITE.ordinal()) {
            holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.favourite));
            if(position == m_activeMenuindex) {
                holder.menuItemTextView.setTextColor(Color.RED);
                holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.favourite_active));
            }
            holder.menuItemTextView.setText(activity.getString(R.string.live_menu_favoritte));
        }
        else if(position == LiveMenuActionItem.ALL.ordinal()) {
            holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.all));
            if(position == m_activeMenuindex) {
                holder.menuItemTextView.setTextColor(Color.RED);
                holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.all_active));
            }
            holder.menuItemTextView.setText(activity.getString(R.string.live_menu_all));
        }
        else if(position == LiveMenuActionItem.NOTICIAS.ordinal()) {
            holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.noticiarios));
            if(position == m_activeMenuindex) {
                holder.menuItemTextView.setTextColor(Color.RED);
                holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.noticiarios_active));
            }
            holder.menuItemTextView.setText(activity.getString(R.string.menu_noticas));
        }
        else if(position == LiveMenuActionItem.ESPORTS.ordinal()) {
            holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.esportes));
            if(position == m_activeMenuindex) {
                holder.menuItemTextView.setTextColor(Color.RED);
                holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.esportes_active));
            }
            holder.menuItemTextView.setText(activity.getString(R.string.live_menu_esports));
        }
        else if(position == LiveMenuActionItem.FILMS_SERIES.ordinal()) {
            holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.tv));
            if(position == m_activeMenuindex) {
                holder.menuItemTextView.setTextColor(Color.RED);
                holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.tv_active));
            }
            holder.menuItemTextView.setText(activity.getString(R.string.live_menu_filmes));
        }else if(position == LiveMenuActionItem.ABERTO.ordinal()) {
            holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.kids));
            if(position == m_activeMenuindex) {
                holder.menuItemTextView.setTextColor(Color.RED);
                holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.kids_active));
            }
            holder.menuItemTextView.setText(activity.getString(R.string.live_menu_aberto));
        }
        else if(position == LiveMenuActionItem.INFANTIL.ordinal()) {
            holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.vector));
            if(position == m_activeMenuindex) {
                holder.menuItemTextView.setTextColor(Color.RED);
                holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.vector_active));
            }
            holder.menuItemTextView.setText(activity.getString(R.string.live_menu_infantil));
        }
        else if(position == LiveMenuActionItem.VARIEDADES.ordinal()) {
            holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.variedades));
            if(position == m_activeMenuindex) {
                holder.menuItemTextView.setTextColor(Color.RED);
                holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.variedades_active));
            }
            holder.menuItemTextView.setText(activity.getString(R.string.live_menu_variedades));
        }
        else if(position == LiveMenuActionItem.DOUCMENTARIOS.ordinal()) {
            holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.documentarios));
            if(position == m_activeMenuindex) {
                holder.menuItemTextView.setTextColor(Color.RED);
                holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.documentarios_active));
            }
            holder.menuItemTextView.setText(activity.getString(R.string.live_menu_documentarios));
        }
        else if(position == LiveMenuActionItem.ADULTO.ordinal()) {
            holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.adulto));
            if(position == m_activeMenuindex) {
                holder.menuItemTextView.setTextColor(Color.RED);
                holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.adulto_active));
            }
            holder.menuItemTextView.setText(activity.getString(R.string.live_menu_adulto));
        }
        else if(position == LiveMenuActionItem.RELIGIOSOS.ordinal()) {
            holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.religiosos));
            if(position == m_activeMenuindex) {
                holder.menuItemTextView.setTextColor(Color.RED);
                holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.religiosos_active));
            }
            holder.menuItemTextView.setText(activity.getString(R.string.live_menu_religiosos));
        }
        else if(position == LiveMenuActionItem.LIVE4K.ordinal()) {
            holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.live_4k));
            if(position == m_activeMenuindex) {
                holder.menuItemTextView.setTextColor(Color.RED);
                holder.menuItemImageView.setImageDrawable(activity.getResources().getDrawable(R.drawable.live_4k_active));
            }
            holder.menuItemTextView.setText(activity.getString(R.string.live_menu_4k));
        }
        return rowView;
    }

    public void setActiveMenuIndex(int activeIndex) {
        m_activeMenuindex = activeIndex;
    }

    private static class MenuItemViewHolder {
        public ImageView menuItemImageView;
        public TextView menuItemTextView;
    }
}
