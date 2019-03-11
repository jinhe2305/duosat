package com.duosat.tv.main;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

import com.duosat.tv.R;
import com.duosat.tv.utils.Utils;

import java.util.ArrayList;

import static com.duosat.tv.utils.Utils.IMG_TRANSFORM_FILL;

public class ViewPagerCarouselFragment extends Fragment {
    public static final String IMAGE_RESOURCE_ID = "image_resource_id";

    private ImageView ivCarouselImage_1, ivCarouselImage_2, ivCarouselImage_3;
    private ArrayList<String> imageResourceId;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.view_pager_carousel_fragment, container, false);
        ivCarouselImage_1 = (ImageView) v.findViewById(R.id.ivtopFirst);
        ivCarouselImage_2 = (ImageView) v.findViewById(R.id.ivtopSecond);
        ivCarouselImage_3 = (ImageView) v.findViewById(R.id.ivtopThird);
        imageResourceId = getArguments().getStringArrayList(IMAGE_RESOURCE_ID); // default to car1 image resource

        if(imageResourceId.get(0) != null)
            Utils.setNetworkImage(ivCarouselImage_1, imageResourceId.get(0), R.drawable.back_white_rect, IMG_TRANSFORM_FILL, "CAROUSEL");
        if(imageResourceId.get(1) != null)
            Utils.setNetworkImage(ivCarouselImage_2, imageResourceId.get(1), R.drawable.back_white_rect, IMG_TRANSFORM_FILL, "CAROUSEL");
        if(imageResourceId.get(2) != null)
            Utils.setNetworkImage(ivCarouselImage_3, imageResourceId.get(2), R.drawable.back_white_rect, IMG_TRANSFORM_FILL, "CAROUSEL");
        return v;
    }
}