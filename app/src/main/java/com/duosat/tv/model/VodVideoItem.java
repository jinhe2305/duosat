package com.duosat.tv.model;

import java.io.Serializable;
import java.util.ArrayList;

public class VodVideoItem implements Serializable{
    public String   thumbnail;
    public String   videoName;
    public String   videoLongDes;
    public String   videoShortDes;
    public ArrayList<String> arrayScreenShot = new ArrayList<>();
    public String   videoURL;
    public String   directURL;
    public String   category;
    public String   srtUrl;
    public int      nID;
    public boolean  videoisDeu;
    public boolean  videoIsNew;
    public boolean  videoIsLike;
}
