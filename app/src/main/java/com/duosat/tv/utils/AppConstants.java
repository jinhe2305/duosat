package com.duosat.tv.utils;

import android.graphics.Bitmap;

import com.duosat.tv.model.ArrayChannelItem;
import com.duosat.tv.model.ChannelMenuItem;
import com.google.common.collect.Maps;
import com.squareup.picasso.Target;

import java.util.ArrayList;
import java.util.Map;

public class AppConstants {
    public static final String APP_NAME                     = "NextTV";
    public static final String LOGIN_USER                   = "login_user";
    public static final String LOGIN_PASSWORD               = "password";
    public static final String MAC_ADDRESS                  = "mac";
    public static final String AUTO_LOGIN                   = "auto_login";
    public static final String ACCOUNT_PASSWORD             = "account_password";
    public static final String ADULT_PASSWORD               = "adult_password";
    public static final String ACCOUNT_SUBSCRIBED           = "subscribed";

    public static final String LIVE_LAAST_CHANNEL_URI       = "live_last_channel_uri";
    public static final String LIVE_CHANNEL_ADAPTER         = "live_channel_adapter";
    public static final String EPG_LIST                     = "epg_list";

    public final static String LOGIN_TAG                    = "LoginActivity";
    public final static String LIVE_TAG                     = "LiveChannel";
    public final static String MOVIE_TAG                    = "Movie";

    public final static String CURRENT_LIVE_CHANNEL         = "CurrentLiveChannel";
    public final static String TORRENT_URL                  = "TorrentUrl";
    public final static String DIRECT_URL                   = "TorrentUrl";
    public final static String PROGRAMME_NAME               = "ProgrammeName";
    public final static String CURRENT_POSITION             = "CurrentPosition";

    public final static int LIVE_REQUEST_CODE               = 1;

    public static final String FAVORITE_ARRAY               = "FavoriteArray";
    public static final ArrayList<String> FAVORITE_CHANNELS = new ArrayList<>();
    public static final int    MAX_VOD_RECENT_COUNT         = 15;

    public static final ArrayChannelItem CHANNEL_LIST       = new ArrayChannelItem();

    public static boolean isP2P                             = true;

    public static final int SEEK_OFFSET                     = 10000;
    public static final int RECONNECT_NEXT_TIME_OUT         = 5000;

    private static final int MAX_HEAP_SIZE              = (int) Runtime.getRuntime().maxMemory();
    public static final int MAX_DISK_CACHE_SIZE         = 40 * 1024 * 1024;
    public static final int MAX_MEMORY_CACHE_SIZE       = MAX_HEAP_SIZE / 4;

    public static final ArrayChannelItem CATCHUP_TV_CHANNEL_LIST            = new ArrayChannelItem();
    public static final ArrayChannelItem CATCHUP_TV_FILTERED_CAHNNEL_LIST   = new ArrayChannelItem();
    public static final Map<String, Bitmap> CHANNEL_IMAGE_CACHE             = Maps.newHashMap();
    public static final Map<String, Target> CHANNEL_IMAGE_TARGET_CACHE      = Maps.newHashMap();

    public static final ArrayList<String> ROLES_LIST                        = new ArrayList<>();
}
