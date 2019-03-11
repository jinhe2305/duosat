package com.duosat.tv.view;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.util.AttributeSet;
import android.util.Log;
import android.view.GestureDetector;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Scroller;

import com.duosat.tv.main.CatchUpPlayerActivity;
import com.duosat.tv.main.CatchUpTVActivity;
import com.duosat.tv.main.VideoPlayerActivity;
import com.squareup.picasso.Picasso;
import com.squareup.picasso.Target;

import com.duosat.tv.R;
import com.duosat.tv.model.ArrayChannelItem;
import com.duosat.tv.model.ArrayItemTopic;
import com.duosat.tv.model.ChannelMenuItem;
import com.duosat.tv.model.EpgMenuItem;
import com.duosat.tv.utils.AppConstants;
import com.duosat.tv.utils.Utils;

import java.util.Calendar;
import java.util.Date;

/**
 * Classic EPG, electronic program guide, that scrolls both horizontal, vertical and diagonal.
 * It utilize onDraw() to draw the graphic on screen. So there are some private helper methods calculating positions etc.
 * Listed on Y-axis are channels and X-axis are programs/events. Data is added to EPG by using setEPGData()
 * and pass in an EPGData implementation. A click listener can be added using setEPGClickListener().
 * Created by Kristoffer, http://kmdev.se
 */
public class EPGView extends ViewGroup {

    public final String TAG = getClass().getSimpleName();
    public static final int DAYS_BACK_MILLIS = 6 * 24 * 60 * 60 * 1000;        // 6 days
    public static final int DAYS_FORWARD_MILLIS = 7 * 24 * 60 * 60 * 1000;     // 7 days
    public static final int HOURS_IN_VIEWPORT_MILLIS = 2 * 60 * 60 * 1000;     // 2 hours
    public static final int TIME_LABEL_SPACING_MILLIS = 30 * 60 * 1000;        // 30 minutes

    private final Rect mClipRect;
    private final Rect mDrawingRect;
    private final Rect mMeasuringRect;
    private final Paint mPaint;
    private final Scroller mScroller;
    private final GestureDetector mGestureDetector;

    private final int mChannelLayoutMargin;
    private final int mChannelLayoutPadding;
    private final int mChannelNumberWidth;
    private final int mChannelLogoWidth;
    private final int mChannelLogoHeight;
    private final int mChannelLayoutHeight;
    private final int mChannelLayoutWidth;
    private final int mTimebarBackground;
    private final int mChannelLayoutBackground;
    private final int mEventLayoutBackground;
    private final int mEventLayoutBackgroundCurrent;
    private final int mEventLayoutTextColor;
    private final int mEventLayoutTextSize;
    private final int mTimeBarLineWidth;
    private final int mTimeBarHeight;
    private final int mTimeBarTextSize;

    private final int mResetButtonSize;
    private final int mResetButtonMargin;

    private EPGClickListener    mClickListener;
    private OnKeyListener       m_keyListener;
    private int mMaxHorizontalScroll;
    private int mMaxVerticalScroll;
    private long mMillisPerPixel;
    private long mTimeOffset;
    private long mTimeLowerBoundary;
    private long mTimeUpperBoundary;

    private int m_iSelectedChannel;
    private int m_iSelectedTopic;

    private Handler timeLineHandler = new Handler();
    private Runnable runnableTimeLineUpdate = new Runnable() {
        @Override
        public void run() {
            redraw();
            timeLineHandler.postDelayed(runnableTimeLineUpdate, 5000);
        }
    };

    private ArrayChannelItem epgData = null;

    public EPGView(Context context) {
        this(context, null);
    }

    public EPGView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public EPGView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);

        setWillNotDraw(false);

        resetBoundaries();

        mDrawingRect = new Rect();
        mClipRect = new Rect();
        mMeasuringRect = new Rect();
        mPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        mGestureDetector = new GestureDetector(context, new OnGestureListener());

        // Adding some friction that makes the epg less flappy.
        mScroller = new Scroller(context);
        mScroller.setFriction(0.2f);

        mChannelLayoutMargin = getResources().getDimensionPixelOffset(R.dimen.pad_2dp);
        mChannelLayoutPadding = getResources().getDimensionPixelSize(R.dimen.pad_5dp);
        mChannelNumberWidth = getResources().getDimensionPixelOffset(R.dimen.img_size_70dp);
        mChannelLayoutHeight = getResources().getDimensionPixelSize(R.dimen.img_size_45dp);
        mChannelLayoutWidth = getResources().getDimensionPixelSize(R.dimen.img_size_70dp) + mChannelNumberWidth;
        mChannelLogoWidth = getResources().getDimensionPixelOffset(R.dimen.img_size_44dp);
        mChannelLogoHeight = getResources().getDimensionPixelOffset(R.dimen.img_size_33dp);

        mTimebarBackground = getResources().getColor(R.color.catchup_back_color);
        mChannelLayoutBackground = getResources().getColor(R.color.vod_back_color);
        mEventLayoutBackground = getResources().getColor(R.color.vod_back_color);
        mEventLayoutBackgroundCurrent = getResources().getColor(R.color.listview_back_color);
        mEventLayoutTextColor = Color.WHITE;
        mEventLayoutTextSize = getResources().getDimensionPixelSize(R.dimen.font_15sp);

        mTimeBarHeight = getResources().getDimensionPixelSize(R.dimen.time_bar_height);
        mTimeBarTextSize = getResources().getDimensionPixelSize(R.dimen.font_15sp);
        mTimeBarLineWidth = getResources().getDimensionPixelSize(R.dimen.img_size_45dp);

        mResetButtonSize = getResources().getDimensionPixelSize(R.dimen.img_size_25dp);
        mResetButtonMargin = getResources().getDimensionPixelSize(R.dimen.margin_15dp);

        BitmapFactory.Options options = new BitmapFactory.Options();
        options.outWidth = mResetButtonSize;
        options.outHeight = mResetButtonSize;

        m_iSelectedChannel = -1;
        m_iSelectedTopic = -1;

        setKeyListener();

        timeLineHandler.postDelayed(runnableTimeLineUpdate, 5000);
    }

    @Override
    protected void onDraw(Canvas canvas) {

        if (epgData != null && epgData.size() > 0) {
            mTimeLowerBoundary = getTimeFrom(getScrollX());
            mTimeUpperBoundary = getTimeFrom(getScrollX() + getWidth());

            Rect drawingRect = mDrawingRect;
            drawingRect.left = getScrollX();
            drawingRect.top = getScrollY();
            drawingRect.right = drawingRect.left + getWidth();
            drawingRect.bottom = drawingRect.top + getHeight();

            drawChannelListItems(canvas, drawingRect);
            drawEvents(canvas, drawingRect);
            drawTimebar(canvas, drawingRect);
            drawTimeLine(canvas, drawingRect);

            // If scroller is scrolling/animating do scroll. This applies when doing a fling.
            if (mScroller.computeScrollOffset()) {
                scrollTo(mScroller.getCurrX(), mScroller.getCurrY());
            }
        }
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
        recalculateAndRedraw(false);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        return mGestureDetector.onTouchEvent(event);
    }

    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {
    }

    public int getSelectedChannelIndex() {
        return m_iSelectedChannel;
    }

    public int getSelectedTopicIndex() {
        return m_iSelectedTopic;
    }

    public boolean processKeyEvent(int keyCode, KeyEvent event) {
        Date dateSelectedTopicStart = new Date();
        dateSelectedTopicStart.setTime(0);
        if((keyCode == KeyEvent.KEYCODE_DPAD_DOWN || keyCode == KeyEvent.KEYCODE_MEDIA_NEXT) && event.getAction() == KeyEvent.ACTION_DOWN) {
            dateSelectedTopicStart = epgData.get(m_iSelectedChannel).m_arrItemTopic.get(m_iSelectedTopic).m_dateTopicStart;
            epgData.get(m_iSelectedChannel).m_arrItemTopic.get(m_iSelectedTopic).m_bIsSelected = false;

            m_iSelectedChannel++;
            if(m_iSelectedChannel > epgData.size() - 1)
            {
                m_iSelectedChannel = 0;
            }

            selectTopicByTime(dateSelectedTopicStart);

            return true;
        }
        else if((keyCode == KeyEvent.KEYCODE_DPAD_UP || keyCode == KeyEvent.KEYCODE_MEDIA_PREVIOUS) && event.getAction() == KeyEvent.ACTION_DOWN) {
            dateSelectedTopicStart = epgData.get(m_iSelectedChannel).m_arrItemTopic.get(m_iSelectedTopic).m_dateTopicStart;
            epgData.get(m_iSelectedChannel).m_arrItemTopic.get(m_iSelectedTopic).m_bIsSelected = false;

            m_iSelectedChannel--;
            if(m_iSelectedChannel < 0)
            {
                m_iSelectedChannel = epgData.size() - 1;
            }

            selectTopicByTime(dateSelectedTopicStart);

            return true;
        }
        else if(keyCode == KeyEvent.KEYCODE_DPAD_LEFT && event.getAction() == KeyEvent.ACTION_DOWN) {
            m_iSelectedTopic--;
            if(m_iSelectedTopic > -1 &&  epgData.get(m_iSelectedChannel).m_arrItemTopic.size() > 1){
                epgData.get(m_iSelectedChannel).m_arrItemTopic.get(m_iSelectedTopic + 1).m_bIsSelected = false;
                dateSelectedTopicStart = epgData.get(m_iSelectedChannel).m_arrItemTopic.get(m_iSelectedTopic).m_dateTopicStart;
                selectTopicByTime(dateSelectedTopicStart);
            }
            else {
                m_iSelectedTopic = 0;
                CatchUpTVActivity activity = (CatchUpTVActivity) getContext();
                if(activity != null) {
                    activity.setFocusOnMenu();
                }
            }
            return  true;
        }
        else if(keyCode == KeyEvent.KEYCODE_DPAD_RIGHT && event.getAction() == KeyEvent.ACTION_DOWN) {
            m_iSelectedTopic++;
            if(m_iSelectedTopic < epgData.get(m_iSelectedChannel).m_arrItemTopic.size() && epgData.get(m_iSelectedChannel).m_arrItemTopic.size() > 1){
                epgData.get(m_iSelectedChannel).m_arrItemTopic.get(m_iSelectedTopic - 1).m_bIsSelected = false;
                dateSelectedTopicStart = epgData.get(m_iSelectedChannel).m_arrItemTopic.get(m_iSelectedTopic).m_dateTopicStart;
                selectTopicByTime(dateSelectedTopicStart);
            }
            else {
                m_iSelectedTopic--;
            }
            return true;
        }
        else if(keyCode == KeyEvent.KEYCODE_MENU && event.getAction() == KeyEvent.ACTION_DOWN) {
            CatchUpTVActivity activity = (CatchUpTVActivity) getContext();
            if(activity != null) {
                activity.setFocusOnMenu();
            }
            return true;
        }
        else if((keyCode == KeyEvent.KEYCODE_DPAD_CENTER || keyCode == KeyEvent.KEYCODE_ENTER) && event.getAction() == KeyEvent.ACTION_DOWN) {
            Intent intent = new Intent(getContext(), CatchUpPlayerActivity.class);
            String videoUrl = epgData.get(m_iSelectedChannel).channelURL;
            videoUrl = "https://edge.mc-amc.com:30443/catchuptv/warnerchannel/playlist_dvr.m3u8";
            EpgMenuItem epgMenuItem = epgData.get(m_iSelectedChannel).m_arrItemTopic.get(m_iSelectedTopic);
            int shiftTime = (int) ((Utils.CurrentTime().getTime() - epgMenuItem.m_dateTopicStart.getTime()) / 1000);
            videoUrl = videoUrl.substring(0, videoUrl.indexOf(".m3u8")) + "_timeshift-" + String.valueOf(shiftTime) + ".m3u8";
            Log.i("videoUrl", videoUrl);
            if(!videoUrl.isEmpty())
                intent.putExtra(AppConstants.DIRECT_URL, videoUrl);

            intent.putExtra(AppConstants.PROGRAMME_NAME, epgData.get(m_iSelectedChannel).m_arrItemTopic.get(m_iSelectedTopic).strName);

            getContext().startActivity(intent);
            return true;
        }

        return false;
    }

    public void setKeyListener() {
        m_keyListener = new OnKeyListener() {
            @Override
            public boolean onKey(View v, int keyCode, KeyEvent event) {
                return processKeyEvent(keyCode, event);
            }
        };

        setOnKeyListener(m_keyListener);
    }

    private int getNearestTopicItem(Date selectedTopicStart) {
        if (epgData.get(m_iSelectedChannel).m_arrItemTopic.size() <= 0)
            return -1;

        int			i;
        int			iIndex = 0;
        long		lOffset;
        long		lOffsetNew;
        EpgMenuItem	itemTopic = epgData.get(m_iSelectedChannel).m_arrItemTopic.get(0);

        if(itemTopic.m_dateTopicStart == null && itemTopic.m_dateTopicEnd == null)
            return 0;

        lOffset = selectedTopicStart == null? Utils.CurrentTime().getTime() : selectedTopicStart.getTime();
        lOffset -= itemTopic.m_dateTopicStart.getTime();

        // When First Item is at the Right of Offset
        if (lOffset < 0)
            return iIndex;

        for (i = 1; i < epgData.get(m_iSelectedChannel).m_arrItemTopic.size(); i++) {
            itemTopic = epgData.get(m_iSelectedChannel).m_arrItemTopic.get(i);
            lOffsetNew = selectedTopicStart == null? Utils.CurrentTime().getTime() : selectedTopicStart.getTime();
            lOffsetNew -= itemTopic.m_dateTopicStart.getTime();

            // When Two Items are at the Left of Offset
            if (lOffset > 0 && lOffsetNew > 0) {
                iIndex = i;
                lOffset = lOffsetNew;
                continue;
            }

            // When First Item is at the Left of Offset
            // When Second Item is at the Right of Offset
            if (lOffset >= 0 && lOffsetNew <= 0) {
                lOffsetNew *= -1;
                if (lOffset > lOffsetNew)
                    iIndex = i;
                break;
            }
        }

        return iIndex;
    }

    private void selectTopicByTime(Date selectedTopicStart) {
        ChannelMenuItem channelItem = epgData.get(m_iSelectedChannel);
        m_iSelectedTopic = getNearestTopicItem(selectedTopicStart);

        CatchUpTVActivity activity = (CatchUpTVActivity) getContext();
        if(activity != null) {
            activity.updateSelectedTopicInfo();
        }

        channelItem.m_arrItemTopic.get(m_iSelectedTopic).m_bIsSelected = true;

        int orgx = getScrollX(), orgy = getScrollY();

        int left = channelItem.m_arrItemTopic.get(m_iSelectedTopic).m_dateTopicStart == null ? orgx : getXFrom(channelItem.m_arrItemTopic.get(m_iSelectedTopic).m_dateTopicStart.getTime()) - mChannelLayoutWidth - mChannelLayoutMargin,
                right = channelItem.m_arrItemTopic.get(m_iSelectedTopic).m_dateTopicEnd == null ? orgx + getWidth() : getXFrom(channelItem.m_arrItemTopic.get(m_iSelectedTopic).m_dateTopicEnd.getTime()) - mChannelLayoutWidth - mChannelLayoutMargin;
        int top = getTopFrom(m_iSelectedChannel) - mTimeBarHeight - mChannelLayoutMargin, bottom = top + mChannelLayoutHeight + mChannelLayoutMargin;

        int dx = left - orgx, dy = top - orgy, dxright = right - orgx - (getWidth() - mChannelLayoutWidth - mChannelLayoutMargin),
                dybottom = bottom - orgy - (getHeight() - mTimeBarHeight - mChannelLayoutMargin);

        int offsetx = 0, offsety = 0;
        if(dx < 0)
            offsetx = dx;
        else {
            if(dx > getWidth() - mChannelLayoutWidth - mChannelLayoutMargin)
                offsetx = dx;
            else if(dxright > 0)
                offsetx = dx < dxright? dx : dxright;
        }

        if(dy < 0)
            offsety = dy;
        else {
            if(dy > getHeight() - mTimeBarHeight - mChannelLayoutMargin) {
                if(dy > mMaxVerticalScroll)
                    offsety = mMaxVerticalScroll;
                else
                    offsety = dy;
            }
            else if(dybottom > 0)
                offsety = dy < dybottom? dy : dybottom;
        }

        scrollBy(offsetx, offsety);
        redraw();
    }

    public void setCurrentChannel(int selectedChannel) {
        ChannelMenuItem channelItem = epgData.get(m_iSelectedChannel);
        channelItem.m_arrItemTopic.get(m_iSelectedTopic).m_bIsSelected = false;

        m_iSelectedChannel = selectedChannel;
        channelItem = epgData.get(m_iSelectedChannel);
        m_iSelectedTopic = channelItem.getCurrentTopicIndex();

        selectTopicByTime(channelItem.m_arrItemTopic.get(m_iSelectedTopic).m_dateTopicStart);
    }

    private void drawTimebar(Canvas canvas, Rect drawingRect) {
        drawingRect.left = getScrollX() + mChannelLayoutWidth;
        drawingRect.top = getScrollY();
        drawingRect.right = drawingRect.left + getWidth();
        drawingRect.bottom = drawingRect.top + mTimeBarHeight;

        mClipRect.left = getScrollX() + mChannelLayoutWidth;
        mClipRect.top = getScrollY();
        mClipRect.right = getScrollX() + getWidth();
        mClipRect.bottom = mClipRect.top + mTimeBarHeight;

        canvas.save();
        canvas.clipRect(mClipRect);

        // Background
        mPaint.setColor(mTimebarBackground);
        canvas.drawRect(drawingRect, mPaint);

        // Time stamps
        mPaint.setColor(mEventLayoutTextColor);
        mPaint.setTextSize(mTimeBarTextSize);

        for (int i = 0; i < HOURS_IN_VIEWPORT_MILLIS / TIME_LABEL_SPACING_MILLIS; i++) {
            // Get time and round to nearest half hour
            final long time = TIME_LABEL_SPACING_MILLIS *
                    (((mTimeLowerBoundary + (TIME_LABEL_SPACING_MILLIS * i)) +
                            (TIME_LABEL_SPACING_MILLIS / 2)) / TIME_LABEL_SPACING_MILLIS);

            canvas.drawText(Utils.getShortTime(time),
                    getXFrom(time),
                    drawingRect.top + (((drawingRect.bottom - drawingRect.top) / 2) + (mTimeBarTextSize / 2)), mPaint);
        }

        canvas.restore();

        drawTimebarDayIndicator(canvas, drawingRect);
    }

    private void drawTimebarDayIndicator(Canvas canvas, Rect drawingRect) {
        drawingRect.left = getScrollX();
        drawingRect.top = getScrollY();
        drawingRect.right = drawingRect.left + mChannelLayoutWidth;
        drawingRect.bottom = drawingRect.top + mTimeBarHeight;

        // Background
        mPaint.setColor(mTimebarBackground);
        canvas.drawRect(drawingRect, mPaint);

        // Text
        mPaint.setColor(mEventLayoutTextColor);
        mPaint.setTextSize(mTimeBarTextSize);
        mPaint.setTextAlign(Paint.Align.CENTER);
        canvas.drawText(getResources().getString(R.string.dvr),//Utils.getWeekdayName(mTimeLowerBoundary),
                drawingRect.left + ((drawingRect.right - drawingRect.left) / 2),
                drawingRect.top + (((drawingRect.bottom - drawingRect.top) / 2) + (mTimeBarTextSize / 2)), mPaint);

        mPaint.setTextAlign(Paint.Align.LEFT);
    }

    private void drawTimeLine(Canvas canvas, Rect drawingRect) {
        long now = System.currentTimeMillis();

        if (shouldDrawTimeLine(now)) {
            drawingRect.left = getXFrom(now) - mTimeBarLineWidth;
            drawingRect.top = getScrollY();
            drawingRect.right = drawingRect.left + mTimeBarLineWidth;
            drawingRect.bottom = drawingRect.top + getHeight();

//            mPaint.setColor(mTimeBarLineColor);
//            mPaint.setColor(Color.TRANSPARENT);
//            Bitmap bitmap = ((BitmapDrawable)getResources().getDrawable(R.drawable.timelinebar)).getBitmap();
//            canvas.drawBitmap(bitmap, new Rect(0,0,bitmap.getWidth(), bitmap.getHeight()), drawingRect, mPaint);
//            canvas.drawRect(drawingRect, mPaint);
        }
    }

    private void drawEvents(Canvas canvas, Rect drawingRect) {
        final int firstPos = getFirstVisibleChannelPosition();
        final int lastPos = getLastVisibleChannelPosition();

        for (int pos = firstPos; pos <= lastPos; pos++) {

            // Set clip rectangle
            mClipRect.left = getScrollX() + mChannelLayoutWidth + mChannelLayoutMargin;
            mClipRect.top = getTopFrom(pos);
            mClipRect.right = getScrollX() + getWidth();
            mClipRect.bottom = mClipRect.top + mChannelLayoutHeight;

            canvas.save();
            canvas.clipRect(mClipRect);

            // Draw each event
            boolean foundFirst = false;

            ChannelMenuItem epgEvents = epgData.get(pos);

            for (EpgMenuItem event : epgEvents.m_arrItemTopic) {
                if (event.m_dateTopicStart == null || event.m_dateTopicEnd == null || isEventVisible(event.m_dateTopicStart.getTime(), event.m_dateTopicEnd.getTime())) {
                    drawEvent(canvas, pos, event, drawingRect);
                    foundFirst = true;
                } else if (foundFirst) {
                    break;
                }
            }

            canvas.restore();
        }
    }

    private void drawEvent(final Canvas canvas, final int channelPosition, final EpgMenuItem event, final Rect drawingRect) {
        if(event.m_dateTopicStart != null && event.m_dateTopicEnd != null)
            setEventDrawingRectangle(channelPosition, event.m_dateTopicStart.getTime(), event.m_dateTopicEnd.getTime(), drawingRect);
        else {
            drawingRect.left = getScrollX() + mChannelLayoutWidth + mChannelLayoutMargin;
            drawingRect.top = getTopFrom(channelPosition);
            drawingRect.right = getScrollX() + getWidth();
            drawingRect.bottom = drawingRect.top + mChannelLayoutHeight;
        }

        // Background
        mPaint.setColor(event.m_bIsSelected ? mEventLayoutBackgroundCurrent : mEventLayoutBackground);
        canvas.drawRect(drawingRect, mPaint);

        // Add left and right inner padding
        drawingRect.left += mChannelLayoutPadding;
        drawingRect.right -= mChannelLayoutPadding;

        // Text
        mPaint.setColor(mEventLayoutTextColor);
        mPaint.setTextSize(mEventLayoutTextSize);

        // Move drawing.top so text will be centered (text is drawn bottom>up)
        mPaint.getTextBounds(event.strName, 0, event.strName.length(), mMeasuringRect);
        drawingRect.top += (((drawingRect.bottom - drawingRect.top) / 2) + (mMeasuringRect.height()/2));

        String title = event.strName;
        title = title.substring(0,
                mPaint.breakText(title, true, drawingRect.right - drawingRect.left, null));
        canvas.drawText(title, drawingRect.left, drawingRect.top, mPaint);
    }

    private void setEventDrawingRectangle(final int channelPosition, final long start, final long end, final Rect drawingRect) {
        drawingRect.left = getXFrom(start);
        drawingRect.top = getTopFrom(channelPosition);
        drawingRect.right = getXFrom(end) - mChannelLayoutMargin;
        drawingRect.bottom = drawingRect.top + mChannelLayoutHeight;
    }

    private void drawChannelListItems(Canvas canvas, Rect drawingRect) {
        // Background
        mMeasuringRect.left = getScrollX();
        mMeasuringRect.top = getScrollY();
        mMeasuringRect.right = drawingRect.left + mChannelLayoutWidth;
        mMeasuringRect.bottom = mMeasuringRect.top + getHeight();

        mPaint.setColor(mChannelLayoutBackground);

        final int firstPos = getFirstVisibleChannelPosition();
        final int lastPos = getLastVisibleChannelPosition();

        for (int pos = firstPos; pos <= lastPos; pos++) {
            drawChannelItem(canvas, pos, drawingRect);
        }
    }

    private void drawChannelItem(final Canvas canvas, int position, Rect drawingRect) {
        drawingRect.left = getScrollX();
        drawingRect.top = getTopFrom(position);
        drawingRect.right = drawingRect.left + mChannelLayoutWidth;
        drawingRect.bottom = drawingRect.top + mChannelLayoutHeight;

        //draw Channel Background
        if(position == m_iSelectedChannel)
            mPaint.setColor(mEventLayoutBackgroundCurrent);
        else
            mPaint.setColor(mChannelLayoutBackground);
        canvas.drawRect(drawingRect, mPaint);

        // Loading channel image into target for
        drawingRect.left += mChannelLayoutPadding;
        drawingRect.right -= mChannelNumberWidth;
        final String imageURL = epgData.get(position).channelSrc;

        if (AppConstants.CHANNEL_IMAGE_CACHE.containsKey(imageURL)) {
            Bitmap image = AppConstants.CHANNEL_IMAGE_CACHE.get(imageURL);
            drawingRect = getDrawingRectForChannelImage(drawingRect, image);
            canvas.drawBitmap(image, null, drawingRect, null);
        } else {
            if (!AppConstants.CHANNEL_IMAGE_CACHE.containsKey(imageURL)) {
                AppConstants.CHANNEL_IMAGE_TARGET_CACHE.put(imageURL, new Target() {
                    @Override
                    public void onBitmapLoaded(Bitmap bitmap, Picasso.LoadedFrom from) {
                        AppConstants.CHANNEL_IMAGE_CACHE.put(imageURL, bitmap);
                        redraw();
                        AppConstants.CHANNEL_IMAGE_TARGET_CACHE.remove(imageURL);
                    }

                    @Override
                    public void onBitmapFailed(Exception e, Drawable errorDrawable) {

                    }

                    @Override
                    public void onPrepareLoad(Drawable placeHolderDrawable) {

                    }
                });

                if(imageURL != null &&!imageURL.isEmpty())
                    Utils.loadImageInto(getContext(), imageURL, mChannelLogoWidth, mChannelLogoHeight, AppConstants.CHANNEL_IMAGE_TARGET_CACHE.get(imageURL));
            }

        }

        //drawChannel Number
        drawingRect.left = getScrollX() + mChannelLayoutWidth - mChannelNumberWidth;
        drawingRect.right = drawingRect.left + mChannelNumberWidth;
        String channelNumber = String.valueOf(epgData.get(position).channelNumber);
        mPaint.getTextBounds(channelNumber, 0, channelNumber.length(), mMeasuringRect);
        int drawTextTop = drawingRect.top + (((drawingRect.bottom - drawingRect.top) / 2) + (mMeasuringRect.height()/2));

        channelNumber = channelNumber.substring(0,
                mPaint.breakText(channelNumber, true, drawingRect.right - drawingRect.left, null));
        mPaint.setColor(mEventLayoutTextColor);
        canvas.drawText(channelNumber, drawingRect.left + (mChannelNumberWidth - mMeasuringRect.width()) / 2, drawTextTop, mPaint);
    }

    private Rect getDrawingRectForChannelImage(Rect drawingRect, Bitmap image) {
        drawingRect.left += mChannelLayoutPadding;
        drawingRect.top += mChannelLayoutPadding;
        drawingRect.right -= mChannelLayoutPadding;
        drawingRect.bottom -= mChannelLayoutPadding;

        final int imageWidth = image.getWidth();
        final int imageHeight = image.getHeight();
        final float imageRatio = imageHeight / (float) imageWidth;

        final int rectWidth = drawingRect.right - drawingRect.left;
        final int rectHeight = drawingRect.bottom - drawingRect.top;

        // Keep aspect ratio.
        if (imageWidth > imageHeight) {
            final int padding = (int) (rectHeight - (rectWidth * imageRatio)) / 2;
            drawingRect.top += padding;
            drawingRect.bottom -= padding;
        } else if (imageWidth <= imageHeight) {
            final int padding = (int) (rectWidth - (rectHeight / imageRatio)) / 2;
            drawingRect.left += padding;
            drawingRect.right -= padding;
        }

        return drawingRect;
    }

    private boolean shouldDrawTimeLine(long now) {
        return now >= mTimeLowerBoundary && now < mTimeUpperBoundary;
    }

    private boolean isEventVisible(final long start, final long end) {
        return (start >= mTimeLowerBoundary && start <= mTimeUpperBoundary)
                || (end >= mTimeLowerBoundary && end <= mTimeUpperBoundary)
                || (start <= mTimeLowerBoundary && end >= mTimeUpperBoundary);
    }

    private long calculatedBaseLine() {
        return Calendar.getInstance().getTimeInMillis() - DAYS_BACK_MILLIS;
    }

    private int getFirstVisibleChannelPosition() {
        final int y = getScrollY();

        int position = (y - mChannelLayoutMargin - mTimeBarHeight)
                / (mChannelLayoutHeight + mChannelLayoutMargin);

        if (position < 0) {
            position = 0;
        }
        return position;
    }

    private int getLastVisibleChannelPosition() {
        final int y = getScrollY();
        final int totalChannelCount = epgData.size();
        final int screenHeight = getHeight();
        int position = (y + screenHeight + mTimeBarHeight - mChannelLayoutMargin)
                / (mChannelLayoutHeight + mChannelLayoutMargin);

        if (position > totalChannelCount - 1) {
            position = totalChannelCount - 1;
        }

        // Add one extra row if we don't fill screen with current..
        return (y + screenHeight) > (position * mChannelLayoutHeight) && position < totalChannelCount - 1 ? position + 1 : position;
    }

    private void calculateMaxHorizontalScroll() {
        mMaxHorizontalScroll = (int) ((DAYS_BACK_MILLIS + DAYS_FORWARD_MILLIS - HOURS_IN_VIEWPORT_MILLIS) / mMillisPerPixel);
    }

    private void calculateMaxVerticalScroll() {
        final int maxVerticalScroll = getTopFrom(epgData.size() - 1) + mChannelLayoutHeight + mChannelLayoutMargin;
        mMaxVerticalScroll = maxVerticalScroll < getHeight() ? 0 : maxVerticalScroll - getHeight();
    }

    private int getXFrom(long time) {
        return (int) ((time - mTimeOffset) / mMillisPerPixel) + mChannelLayoutMargin
                + mChannelLayoutWidth + mChannelLayoutMargin;
    }

    private int getTopFrom(int position) {
        int y = position * (mChannelLayoutHeight + mChannelLayoutMargin)
                + mChannelLayoutMargin + mTimeBarHeight;
        return y;
    }

    private long getTimeFrom(int x) {
        return (x * mMillisPerPixel) + mTimeOffset;
    }

    private long calculateMillisPerPixel() {
        return HOURS_IN_VIEWPORT_MILLIS / (getResources().getDisplayMetrics().widthPixels - mChannelLayoutWidth - mChannelLayoutMargin);
    }

    private int getXPositionStart() {
        return getXFrom(System.currentTimeMillis() - (HOURS_IN_VIEWPORT_MILLIS / 2));
    }

    private void resetBoundaries() {
        mMillisPerPixel = calculateMillisPerPixel();
        mTimeOffset = calculatedBaseLine();
        mTimeLowerBoundary = getTimeFrom(0);
        mTimeUpperBoundary = getTimeFrom(getWidth());
    }

    private Rect calculateChannelsHitArea() {
        mMeasuringRect.top = mTimeBarHeight;
        int visibleChannelsHeight = epgData.size() * (mChannelLayoutHeight + mChannelLayoutMargin);
        mMeasuringRect.bottom = visibleChannelsHeight < getHeight() ? visibleChannelsHeight : getHeight();
        mMeasuringRect.left = 0;
        mMeasuringRect.right = mChannelLayoutWidth;
        return mMeasuringRect;
    }

    private Rect calculateProgramsHitArea() {
        mMeasuringRect.top = mTimeBarHeight;
        int visibleChannelsHeight = epgData.size() * (mChannelLayoutHeight + mChannelLayoutMargin);
        mMeasuringRect.bottom = visibleChannelsHeight < getHeight() ? visibleChannelsHeight : getHeight();
        mMeasuringRect.left = mChannelLayoutWidth;
        mMeasuringRect.right = getWidth();
        return mMeasuringRect;
    }

    private Rect calculateResetButtonHitArea() {
        mMeasuringRect.left = getScrollX() + getWidth() - mResetButtonSize - mResetButtonMargin;
        mMeasuringRect.top = getScrollY() + getHeight() - mResetButtonSize - mResetButtonMargin;
        mMeasuringRect.right = mMeasuringRect.left + mResetButtonSize;
        mMeasuringRect.bottom = mMeasuringRect.top + mResetButtonSize;
        return mMeasuringRect;
    }

    private int getChannelPosition(int y) {
        y -= mTimeBarHeight;
        int channelPosition = (y + mChannelLayoutMargin)
                / (mChannelLayoutHeight + mChannelLayoutMargin);

        return epgData.size() == 0 ? -1 : channelPosition;
    }

    private int getProgramPosition(int channelPosition, long time) {
        ArrayItemTopic arrayItemTopic = epgData.get(channelPosition).m_arrItemTopic;

        if (arrayItemTopic != null) {

            for (int eventPos = 0; eventPos < arrayItemTopic.size(); eventPos++) {
                EpgMenuItem event = arrayItemTopic.get(eventPos);

                if (event.m_dateTopicStart.getTime() <= time && event.m_dateTopicEnd.getTime() >= time) {
                    return eventPos;
                }
            }
        }
        return -1;
    }

    /**
     * Add click listener to the EPG.
     * @param epgClickListener to add.
     */
    public void setEPGClickListener(EPGClickListener epgClickListener) {
        mClickListener = epgClickListener;
    }

    /**
     * Add data to EPG. This must be set for EPG to able to draw something.
     * @param epgData pass in any implementation of EPGData.
     */
    public void setEPGData(ArrayChannelItem epgData) {
        this.epgData = epgData;
    }

    /**
     * This will recalculate boundaries, maximal scroll and scroll to start position which is current time.
     * To be used on device rotation etc since the device height and width will change.
     * @param withAnimation true if scroll to current position should be animated.
     */
    public void recalculateAndRedraw(boolean withAnimation) {
        if (epgData != null) {
            resetBoundaries();

            calculateMaxVerticalScroll();
            calculateMaxHorizontalScroll();

//            mScroller.startScroll(0, 0,
//                    getXPositionStart() - 0,
//                    0, withAnimation ? 600 : 0);

            if(epgData.size() > 0) {
                if(m_iSelectedChannel == -1 || m_iSelectedChannel > epgData.size() - 1)
                    m_iSelectedChannel = 0;
                else {
                    if(m_iSelectedTopic != -1)
                        epgData.get(m_iSelectedChannel).m_arrItemTopic.get(m_iSelectedTopic).m_bIsSelected = false;
                }
                if(m_iSelectedChannel > -1 && epgData.get(m_iSelectedChannel).getCurrentTopicIndex() > -1)
                    selectTopicByTime(epgData.get(m_iSelectedChannel).m_arrItemTopic.get(epgData.get(m_iSelectedChannel).getCurrentTopicIndex()).m_dateTopicStart);
            }
            else {
                m_iSelectedChannel = -1;
                m_iSelectedTopic = -1;
            }
            redraw();
        }
    }

    /**
     * Does a invalidate() and requestLayout() which causes a redraw of screen.
     */
    public void redraw() {
        invalidate();
        requestLayout();
    }

    /**
     * Clears the local image cache for channel images. Can be used when leaving epg and you want to
     * free some memory. Images will be fetched again when loading EPG next time.
     */
    public void clearEPGImageCache() {
        AppConstants.CHANNEL_IMAGE_CACHE.clear();
    }

    public void clearSelection() {
        if(m_iSelectedChannel > -1 && m_iSelectedChannel < epgData.size())
        {
            if(m_iSelectedTopic > -1 && m_iSelectedTopic < epgData.get(m_iSelectedChannel).m_arrItemTopic.size())
                epgData.get(m_iSelectedChannel).m_arrItemTopic.get(m_iSelectedTopic).m_bIsSelected = false;
        }
        m_iSelectedChannel = -1;
        m_iSelectedTopic = -1;
    }

    private class OnGestureListener extends GestureDetector.SimpleOnGestureListener {

        @Override
        public boolean onSingleTapUp(MotionEvent e) {

            // This is absolute coordinate on screen not taking scroll into account.
            int x = (int) e.getX();
            int y = (int) e.getY();

            // Adding scroll to clicked coordinate
            int scrollX = getScrollX() + x;
            int scrollY = getScrollY() + y;

            int channelPosition = getChannelPosition(scrollY);
            if (channelPosition != -1 && mClickListener != null) {
                if (calculateResetButtonHitArea().contains(scrollX,scrollY)) {
                    // Reset button clicked
                    mClickListener.onResetButtonClicked();
                } else if (calculateChannelsHitArea().contains(x, y)) {
                    // Channel area is clicked
                    mClickListener.onChannelClicked(channelPosition, epgData.get(channelPosition));
                } else if (calculateProgramsHitArea().contains(x, y)) {
                    // Event area is clicked
                    int programPosition = getProgramPosition(channelPosition, getTimeFrom(getScrollX() + x - calculateProgramsHitArea().left));
                    if (programPosition != -1) {
                        mClickListener.onEventClicked(channelPosition, programPosition, epgData.get(channelPosition).m_arrItemTopic.get(programPosition));
                    }
                }
            }

            return true;
        }

        @Override
        public boolean onScroll(MotionEvent e1, MotionEvent e2,
                                float distanceX, float distanceY) {
            int dx = (int) distanceX;
            int dy = (int) distanceY;
            int x = getScrollX();
            int y = getScrollY();


            // Avoid over scrolling
            if (x + dx < 0) {
                dx = 0 - x;
            }
            if (y + dy < 0) {
                dy = 0 - y;
            }
            if (x + dx > mMaxHorizontalScroll) {
                dx = mMaxHorizontalScroll - x;
            }
            if (y + dy > mMaxVerticalScroll) {
                dy = mMaxVerticalScroll - y;
            }

            scrollBy(dx, dy);
            return true;
        }

        @Override
        public boolean onFling(MotionEvent e1, MotionEvent e2,
                               float vX, float vY) {

            mScroller.fling(getScrollX(), getScrollY(), -(int) vX,
                    -(int) vY, 0, mMaxHorizontalScroll, 0, mMaxVerticalScroll);

            redraw();
            return true;
        }

        @Override
        public boolean onDown(MotionEvent e) {
            if (!mScroller.isFinished()) {
                mScroller.forceFinished(true);
                return true;
            }
            return true;
        }
    }

    private interface EPGClickListener {

        void onChannelClicked(int channelPosition, ChannelMenuItem epgChannel);

        void onEventClicked(int channelPosition, int programPosition, EpgMenuItem epgEvent);

        void onResetButtonClicked();
    }
}
