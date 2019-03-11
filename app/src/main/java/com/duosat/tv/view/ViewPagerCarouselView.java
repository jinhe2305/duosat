package com.duosat.tv.view;

import android.content.Context;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.support.v4.app.FragmentManager;
import android.support.v4.view.ViewPager;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.RelativeLayout;

import com.duosat.tv.R;
import com.duosat.tv.adapter.ViewPagerCarouselAdapter;

import java.util.ArrayList;

public class ViewPagerCarouselView extends RelativeLayout {
    private FragmentManager fragmentManager;                // FragmentManager for managing the fragments withing the ViewPager
    private ViewPager vpCarousel;                           // ViewPager for the Carousel view
    private ArrayList<String> imageResourceIds;                        // Carousel view background image
    private long carouselSlideInterval;                     // Carousel view item sliding interval
    private Handler carouselHandler;                        // Carousel view item sliding interval automation handler
    private int nCount;

    public ViewPagerCarouselView (Context context) {
        super(context);
        initView(context);
    }

    public ViewPagerCarouselView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initView(context);
    }

    public ViewPagerCarouselView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        initView(context);
    }

    public void initView(Context context) {
        LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        inflater.inflate(R.layout.view_pager_carousel_view, this);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        vpCarousel = (ViewPager) this.findViewById(R.id.vp_carousel);
        vpCarousel.setFocusable(false);
    }

    /**
     * Set the data and initialize the carousel view
     * @param fragmentManager
     * @param imageResourceIds
     * @param carouselSlideInterval
     */
    public void setData(FragmentManager fragmentManager, ArrayList<String> imageResourceIds, long carouselSlideInterval) {
        this.fragmentManager = fragmentManager;
        this.imageResourceIds = imageResourceIds;
        this.carouselSlideInterval = carouselSlideInterval;
        initData();
        initCarousel();
        initCarouselSlide();
    }

    /**
     * Initialize the data for the carousel
     */
    private void initData() {
        nCount = ViewPagerCarouselAdapter.MAX_COUNT; //imageResourceIds.size() / 3 + (imageResourceIds.size() % 3 == 0 ? 0 : 1);
    }

    /**
     * Initialize carousel views, each item in the carousel view is a fragment
     */
    private void initCarousel() {
        // Update the carousel page indicator on change
        vpCarousel.addOnPageChangeListener(new ViewPager.OnPageChangeListener() {
            @Override
            public void onPageScrolled(int position, float positionOffset, int positionOffsetPixels) {

            }

            @Override
            public void onPageSelected(int position) {

            }

            @Override
            public void onPageScrollStateChanged(int state) {
            }
        });

        ViewPagerCarouselAdapter viewPagerCarouselAdapter = new ViewPagerCarouselAdapter(fragmentManager, imageResourceIds);
        vpCarousel.setPageTransformer(false, new ViewPager.PageTransformer() {
            @Override
            public void transformPage(@NonNull View page, float position) {

            }
        });
        vpCarousel.setAdapter(viewPagerCarouselAdapter);

    }

    /**
     * Handler to make the view pager to slide automatically
     */

    Runnable runnable = new Runnable() {
        @Override
        public void run() {
            int curPos = vpCarousel.getCurrentItem();
            curPos++;
            if (curPos == nCount) curPos = 0;
            vpCarousel.setCurrentItem(curPos, true);
            carouselHandler.postDelayed(this, carouselSlideInterval);
        }
    };

    private void initCarouselSlide() {
        final int nCount = imageResourceIds.size();
        try {
            carouselHandler = new Handler();
            carouselHandler.postDelayed(runnable, carouselSlideInterval);

        } catch (Exception e) {
            Log.d("MainActivity", e.getMessage());
        }
    }

    public void onDestroy() {
        if (carouselHandler != null) carouselHandler.removeCallbacks(runnable); // remove call backs to prevent memory leaks
    }
}