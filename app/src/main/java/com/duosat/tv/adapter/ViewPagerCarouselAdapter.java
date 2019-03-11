package com.duosat.tv.adapter;

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentStatePagerAdapter;

import com.duosat.tv.main.ViewPagerCarouselFragment;
import com.duosat.tv.main.VodActivity;

import java.util.ArrayList;

public class ViewPagerCarouselAdapter extends FragmentStatePagerAdapter {
    public static final int MAX_COUNT = 60 * 24 * (60000 / VodActivity.carouselSlideInterval); //24hour
    private ArrayList<String> imageResourceIds;
    private int lastIndex = 0;

    public ViewPagerCarouselAdapter(FragmentManager fm, ArrayList<String> imageResourceIds) {
        super(fm);
        this.imageResourceIds = imageResourceIds;
    }

    @Override
    public Fragment getItem(int position) {
        Bundle bundle = new Bundle();

        ArrayList<String> urlList = new ArrayList<>();
        int index = 0;
        if(position == 0)
            lastIndex = 0;
        for(int i = 0; i < 3; i++ ){
            index = lastIndex + i;
            if(index > imageResourceIds.size() - 1)
                index -= imageResourceIds.size();
            urlList.add(imageResourceIds.get(index));
        }
        lastIndex = index + 1;
        bundle.putStringArrayList(ViewPagerCarouselFragment.IMAGE_RESOURCE_ID, urlList);
        ViewPagerCarouselFragment frag = new ViewPagerCarouselFragment();
        frag.setArguments(bundle);

        return frag;
    }

    @Override
    public int getCount() {
        return MAX_COUNT; //(imageResourceIds == null) ? 0: imageResourceIds.size() / 3 + (imageResourceIds.size() % 3 == 0 ? 0 : 1);
    }

}