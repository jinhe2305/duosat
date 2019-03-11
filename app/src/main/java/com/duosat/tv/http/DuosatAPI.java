package com.duosat.tv.http;

import java.util.HashMap;

public class DuosatAPI {
    public static void loginUser(String userId, String password, final VolleyCallback resultCallback, String tag) {
        String url = DuosatAPIConstant.BASE_URL + DuosatAPIConstant.API_LOGIN;
        HashMap<String, String> data = new HashMap<>();
        data.put(DuosatAPIConstant.ITEM_USERNAME, userId);
        data.put(DuosatAPIConstant.ITEM_PASSWORD, password);

        VolleyRequest.getStringResponsePost(url, data, resultCallback, tag);
    }

    public static void logOutUser(String userId, String password, final VolleyCallback resultCallback, String tag) {
        String url = DuosatAPIConstant.BASE_URL + DuosatAPIConstant.API_LOGOUT;
        HashMap<String, String> data = new HashMap<>();
        data.put(DuosatAPIConstant.ITEM_USERNAME, userId);
        data.put(DuosatAPIConstant.ITEM_PASSWORD, password);

        VolleyRequest.getStringResponsePost(url, data, resultCallback, tag);
    }

    public static void getLiveChannels(String userId, String password, final VolleyCallback resultCallback, String tag) {
        String url = DuosatAPIConstant.BASE_URL + DuosatAPIConstant.API_LIVE_CHANNELS;
        HashMap<String, String> data = new HashMap<>();
        data.put(DuosatAPIConstant.ITEM_USERNAME, userId);
        data.put(DuosatAPIConstant.ITEM_PASSWORD, password);

        VolleyRequest.getStringResponsePost(url, data, resultCallback, tag);
    }

    public static void getMovies(String userId, String password, final VolleyCallback resultCallback, String tag) {
        String url = DuosatAPIConstant.BASE_URL + DuosatAPIConstant.API_MOVIES;
        HashMap<String, String> data = new HashMap<>();
        data.put(DuosatAPIConstant.ITEM_USERNAME, userId);
        data.put(DuosatAPIConstant.ITEM_PASSWORD, password);

        VolleyRequest.getStringResponsePost(url, data, resultCallback, tag);
    }
}
