package com.duosat.tv.utils;

import com.duosat.tv.model.VodVideoItem;

import java.util.ArrayList;

public class DataResult {
    private static DataResult instance;
    private ArrayList<VodVideoItem> mediaData = null;

    protected DataResult() {

    }

    public static DataResult getInstance() {
        if (instance == null) {
            instance = new DataResult();
        }
        return instance;
    }

    public ArrayList<VodVideoItem> getMediaData() { return mediaData; }
    public void setMediaData(ArrayList<VodVideoItem> data) { this.mediaData = data; }
}
