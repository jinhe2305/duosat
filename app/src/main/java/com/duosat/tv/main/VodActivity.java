package com.duosat.tv.main;

import android.app.Activity;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.EditText;
import android.widget.GridView;
import android.widget.ListView;

import com.duosat.tv.adapter.GridVideoAdapter;
import com.duosat.tv.adapter.MenuListAdapter;
import com.duosat.tv.http.AppController;
import com.duosat.tv.http.DuosatAPI;
import com.duosat.tv.http.DuosatAPIConstant;
import com.duosat.tv.http.VolleyCallback;
import com.duosat.tv.model.MenuActionItem;
import com.duosat.tv.model.VodVideoItem;
import com.duosat.tv.utils.AppConstants;
import com.duosat.tv.utils.DataResult;
import com.duosat.tv.utils.Utils;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;

import com.duosat.tv.R;
import com.duosat.tv.view.ViewPagerCarouselView;

public class VodActivity extends FragmentActivity {

    ListView        m_lvMenuList;
    MenuListAdapter m_menuListAdapter;
    GridView        m_gvGridView;

    ArrayList<VodVideoItem> m_liveMoviesList = new ArrayList<>();
    ArrayList<VodVideoItem> m_filteredMovieList = new ArrayList<>();
    GridVideoAdapter        m_gridAdapter;

    ViewPagerCarouselView m_carouselView;
    ArrayList<String> imageResourceIds = new ArrayList<>();
    public static int carouselSlideInterval = 30000; //1hour

    EditText        m_edtSearch;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.vod_layout);

        initControl();
        setEventListener();
        loadMoviesData();
    }

    @Override
    public void onBackPressed() {
        AppController.getInstance().cancelPendingRequests(AppConstants.MOVIE_TAG);
        super.onBackPressed();
        finish();
    }

    private void initControl() {
        m_lvMenuList = (ListView)findViewById(R.id.lvMenuList);
        m_menuListAdapter = new MenuListAdapter(R.layout.item_menu, this, MenuActionItem.values());
        m_lvMenuList.setAdapter(m_menuListAdapter);

        m_gvGridView = (GridView)findViewById(R.id.gvVodList);
        m_gridAdapter = new GridVideoAdapter(this, m_filteredMovieList, "video");
        m_gvGridView.setAdapter(m_gridAdapter);

        m_carouselView = (ViewPagerCarouselView)findViewById(R.id.carousel_view);

        m_edtSearch = (EditText) findViewById(R.id.edtSearch);
    }

    private void setEventListener() {
        m_edtSearch.setOnKeyListener(new View.OnKeyListener() {
            @Override
            public boolean onKey(View v, int keyCode, KeyEvent event) {
                if (keyCode == KeyEvent.KEYCODE_SEARCH || keyCode == KeyEvent.KEYCODE_ENTER) {
                    searchMoviess(m_edtSearch.getText().toString());
                }

                return false;
            }
        });

        m_gvGridView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                DataResult.getInstance().setMediaData(m_filteredMovieList);
                VideoDetailsDialog videoDetailsDialog = new VideoDetailsDialog(VodActivity.this, position);
                videoDetailsDialog.show();
            }
        });

        m_lvMenuList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                m_menuListAdapter.setActiveMenuIndex(position);
                m_menuListAdapter.notifyDataSetChanged();
                filterGenre(position);
            }
        });
    }

    private void loadMoviesData() {
        String userId = Utils.getSharePreferenceValue(this, AppConstants.LOGIN_USER, "");
        String password = Utils.getSharePreferenceValue(this, AppConstants.LOGIN_PASSWORD, "");

        DuosatAPI.getMovies(userId, password, new VolleyCallback() {
            @Override
            public void onSuccess(String result) {
                try {
                    JSONObject jsonObj = new JSONObject(result);
                    JSONArray liveArray = jsonObj.getJSONArray(DuosatAPIConstant.ITEM_NOEDS);
                    for(int i = 0; i < liveArray.length(); i++){
                        JSONObject jsonObject = liveArray.getJSONObject(i).getJSONObject(DuosatAPIConstant.ITEM_NOED);
                        VodVideoItem vodItem = new VodVideoItem();
                        vodItem.videoName = jsonObject.has(DuosatAPIConstant.ITEM_NAME) ? jsonObject.getString(DuosatAPIConstant.ITEM_NAME) : "";
                        vodItem.videoURL = jsonObject.has(DuosatAPIConstant.ITEM_TORRENT_FILE) ? jsonObject.getString(DuosatAPIConstant.ITEM_TORRENT_FILE) : "";
                        vodItem.videoLongDes = jsonObject.has(DuosatAPIConstant.ITEM_LONG_DESCRIPTION) ? jsonObject.getString(DuosatAPIConstant.ITEM_LONG_DESCRIPTION) : "";
                        vodItem.videoShortDes = jsonObject.has(DuosatAPIConstant.ITEM_SHORT_DESCRIPTION) ? jsonObject.getString(DuosatAPIConstant.ITEM_SHORT_DESCRIPTION) : "";
                        JSONObject screenShoot = jsonObject.getJSONObject(DuosatAPIConstant.ITEM_SCREENSHOT_1);
                        if(screenShoot != null && screenShoot.has(DuosatAPIConstant.ITEM_SRC))
                            vodItem.arrayScreenShot.add(screenShoot.getString(DuosatAPIConstant.ITEM_SRC));

                        screenShoot = jsonObject.getJSONObject(DuosatAPIConstant.ITEM_SCREENSHOT_2);
                        if(screenShoot != null && screenShoot.has(DuosatAPIConstant.ITEM_SRC))
                            vodItem.arrayScreenShot.add(screenShoot.getString(DuosatAPIConstant.ITEM_SRC));

                        screenShoot = jsonObject.getJSONObject(DuosatAPIConstant.ITEM_SCREENSHOT_1);
                        if(screenShoot != null && screenShoot.has(DuosatAPIConstant.ITEM_SRC))
                            vodItem.arrayScreenShot.add(screenShoot.getString(DuosatAPIConstant.ITEM_SRC));

                        vodItem.directURL = jsonObject.has(DuosatAPIConstant.ITEM_DIRECT_URL) ? jsonObject.getString(DuosatAPIConstant.ITEM_DIRECT_URL) : "";
                        vodItem.category = jsonObject.has(DuosatAPIConstant.ITEM_CATEGORY_URL) ? jsonObject.getString(DuosatAPIConstant.ITEM_CATEGORY_URL) : "";
                        vodItem.srtUrl = jsonObject.has(DuosatAPIConstant.ITEM_SUBTITLE_URL) ? jsonObject.getString(DuosatAPIConstant.ITEM_SUBTITLE_URL) : "";

                        jsonObject = jsonObject.getJSONObject(DuosatAPIConstant.ITEM_THUMBNAIL_URL);
                        if(jsonObject != null)
                            vodItem.thumbnail = jsonObject.has(DuosatAPIConstant.ITEM_SRC) ? jsonObject.getString(DuosatAPIConstant.ITEM_SRC) : "";

                        vodItem.videoIsLike = false;
                        vodItem.videoIsNew = true;
                        vodItem.videoisDeu = false;

                        m_liveMoviesList.add(vodItem);
                    }

                    Collections.sort(m_liveMoviesList, new ChannelComparator());
                    updateCarouseView();

                    filterGenre(0);
                }catch (JSONException e) {
                    e.printStackTrace();
                }
            }

            @Override
            public void onError(Object error) {

            }
        }, AppConstants.MOVIE_TAG);
    }

    private void filterGenre(int position) {
        String filterString = "";
        switch (position) {
            case 0:
                filterString = getString(R.string.menu_lancamentos);
                break;
            case 1:
                filterString = getString(R.string.menu_films);
                break;
            case 2:
                filterString = getString(R.string.menu_series);
                break;
            case 3:
                filterString = getString(R.string.menu_infantil);
                break;
            case 4:
                filterString = getString(R.string.menu_animes);
                break;
            case 5:
                filterString = getString(R.string.menu_noticas);
                break;
        }

        m_filteredMovieList.clear();
        m_gridAdapter.notifyDataSetChanged();
        for (VodVideoItem item: m_liveMoviesList) {
            if(item.category.equalsIgnoreCase(filterString))
                m_filteredMovieList.add(item);
        }
        m_gridAdapter.notifyDataSetChanged();
    }

    private void updateCarouseView() {
        for(int i = 0; i < AppConstants.MAX_VOD_RECENT_COUNT && i < m_liveMoviesList.size(); i++) {
            imageResourceIds.add(m_liveMoviesList.get(i).thumbnail);
        }
        m_carouselView.setData(getSupportFragmentManager(), imageResourceIds, carouselSlideInterval);
    }

    private void searchMoviess(String filterString) {
        m_filteredMovieList.clear();
        m_gridAdapter.notifyDataSetChanged();
        for (VodVideoItem item: m_liveMoviesList) {
            if(item.videoName.contains(filterString))
                m_filteredMovieList.add(item);
        }
        m_gridAdapter.notifyDataSetChanged();
    }

    public class ChannelComparator implements Comparator<VodVideoItem>
    {
        public int compare(VodVideoItem left, VodVideoItem right) {
            if(left.nID > right.nID)
                return -1;
            else
                return 1;
        }
    }
}
