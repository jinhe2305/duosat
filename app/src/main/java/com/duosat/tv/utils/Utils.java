package com.duosat.tv.utils;

import com.duosat.tv.R;
import com.duosat.tv.image.CircleTransform;
import com.duosat.tv.image.RoundRectTransform;
import com.duosat.tv.image.SquareTransform;
import com.squareup.picasso.Callback;
import com.squareup.picasso.LruCache;
import com.squareup.picasso.OkHttp3Downloader;
import com.squareup.picasso.Picasso;
import com.squareup.picasso.Target;
import com.squareup.picasso.Transformation;
import com.vatata.tools.DeviceIDUtil;

import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Rect;
import android.graphics.drawable.BitmapDrawable;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.text.format.DateFormat;
import android.widget.ImageView;
import android.widget.Toast;

import java.net.NetworkInterface;
import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Collections;
import java.util.Date;
import java.util.List;
import java.util.TimeZone;

import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

import static android.content.Context.MODE_PRIVATE;

public class Utils {

    public final static int     IMG_TRANSFORM_CIRCLE    = 0;
    public final static int     IMG_TRANSFORM_FIT       = 1;
    public final static int     IMG_TRANSFORM_ROUND     = 2;
    public final static int     IMG_TRANSFORM_FILL      = 3;

    public static void setNetworkImage(final ImageView target, String url, int errRes, int tranform, final String tag) {
        Transformation transformation;
        switch (tranform) {
            case IMG_TRANSFORM_CIRCLE:
                transformation = new CircleTransform();
                break;
            case IMG_TRANSFORM_FIT:
            case IMG_TRANSFORM_FILL:
                transformation = new SquareTransform();
                break;
            case IMG_TRANSFORM_ROUND:
                transformation = new RoundRectTransform();
                break;
            default:
                transformation = null;
        }

        if (transformation == null || tranform == IMG_TRANSFORM_FIT) {
            Picasso.get()
                    .load(url)
                    //.memoryPolicy(MemoryPolicy.NO_CACHE, MemoryPolicy.NO_CACHE)
                    .placeholder(R.drawable.progress_icon)
                    .error(errRes)
                    .fit()
                    .tag(tag)
                    .centerCrop()
                    .into(target);
        } else if (tranform == IMG_TRANSFORM_FILL) {
            Picasso.get()
                    .load(url)
                    //.memoryPolicy(MemoryPolicy.NO_CACHE, MemoryPolicy.NO_CACHE)
                    .placeholder(R.drawable.progress_icon)
                    .error(errRes)
                    .fit()
                    .tag(tag)
                    .into(target);
        } else {
            Picasso.get()
                    .load(url)
                    .transform(transformation)
                    //.memoryPolicy(MemoryPolicy.NO_CACHE, MemoryPolicy.NO_CACHE)
                    .placeholder(R.drawable.progress_icon)
                    .error(errRes)
                    .tag(tag)
                    .into(target);
        }
    }

    public static String makeAvaiableUrl(String url) {
        String retUrl = url.replaceAll(" ", "%20");
        if (!retUrl.contains("https://"))
            retUrl = retUrl.replaceAll("https:/", "https://");
        if (!retUrl.contains("http://"))
            retUrl = retUrl.replaceAll("http:/", "http://");

        return retUrl;
    }

    public static void disableSSLCertificateChecking() {
        TrustManager[] trustAllCerts = new TrustManager[] { new X509TrustManager() {
            public X509Certificate[] getAcceptedIssuers() {
                return null;
            }

            @Override
            public void checkClientTrusted(X509Certificate[] arg0, String arg1) throws CertificateException {
                // Not implemented
            }

            @Override
            public void checkServerTrusted(X509Certificate[] arg0, String arg1) throws CertificateException {
                // Not implemented
            }
        } };

        try {
            SSLContext sc = SSLContext.getInstance("TLS");

            sc.init(null, trustAllCerts, new java.security.SecureRandom());

            HttpsURLConnection.setDefaultSSLSocketFactory(sc.getSocketFactory());
        } catch (KeyManagementException e) {
            e.printStackTrace();
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        }
    }

    public static void setSharePreferenceValue(Context context, String key, String value) {
        SharedPreferences prefs = context.getApplicationContext().getSharedPreferences(AppConstants.APP_NAME, MODE_PRIVATE);
        SharedPreferences.Editor editor = prefs.edit();
        editor.putString(key, value);
        editor.apply();
    }

    public static String getSharePreferenceValue(Context context, String key, String defaultValue) {
        SharedPreferences prefs = context.getApplicationContext().getSharedPreferences(AppConstants.APP_NAME, MODE_PRIVATE);
        String value = prefs.getString(key, defaultValue);

        return value;
    }

    public static String getMacAddr() {
        try {
            List<NetworkInterface> all = Collections.list(NetworkInterface.getNetworkInterfaces());
            for (NetworkInterface nif : all) {
                if (!nif.getName().equalsIgnoreCase("wlan0")) continue;

                byte[] macBytes = nif.getHardwareAddress();
                if (macBytes == null) {
                    return "";
                }

                StringBuilder res1 = new StringBuilder();
                for (byte b : macBytes) {
                    res1.append(Integer.toHexString(b & 0xFF) + ":");
                }

                if (res1.length() > 0) {
                    res1.deleteCharAt(res1.length() - 1);
                }
                return res1.toString();
            }
        } catch (Exception ex) {
            //handle exception
        }
        return "";
    }

    public static String getDeviceID(Context context) {
        DeviceIDUtil deviceIDUtil = new DeviceIDUtil(context);
        String device_id = deviceIDUtil.getEthMac().substring(1);
        //String device_id = getMacAddr().substring(1);

        String returnStr = device_id.substring(0, 2) + ":" + device_id.substring(2, 4) + ":" + device_id.substring(4, 6) + ":" + device_id.substring(6, 8) + ":" +
                device_id.substring(8, 10) + ":" + device_id.substring(10);

        return returnStr;
    }

    public static String getShortTime(long timeMillis) {
        Calendar calendar = Calendar.getInstance();
        calendar.setTimeInMillis(timeMillis);
        SimpleDateFormat formatter = new SimpleDateFormat("h:mm a");
        return formatter.format(calendar.getTime());
    }

    public static String getWeekdayName(long dateMillis) {
        Calendar calendar = Calendar.getInstance();
        calendar.setTimeInMillis(dateMillis);
        String dayOfTheWeek = (String) DateFormat.format("EEEE", calendar.getTime()); // Thursday
        return dayOfTheWeek;
    }

    public static Date CurrentTime() {
        return Calendar.getInstance().getTime();
    }

    public static String UTCStringToLocalString(String strTime) {
        SimpleDateFormat df = new SimpleDateFormat("yyyyMMddHHmmss Z");
        try {
            Date date = df.parse(strTime);
            df.setTimeZone(TimeZone.getDefault());
            return df.format(date);
        } catch (ParseException e) {
            e.printStackTrace();
            return null;
        }
    }

    private static Picasso picasso = null;

    public static void loadImageInto(Context context, String url, int width, int height, Target target) {
        initPicasso(context);
        picasso.load(url)
                .resize(width, height)
                .centerInside()
                .into(target);
    }

    private static void initPicasso(Context context) {
        if (picasso == null) {
            picasso = new Picasso.Builder(context)
                    .downloader(new OkHttp3Downloader(context, AppConstants.MAX_DISK_CACHE_SIZE))
                    .memoryCache(new LruCache(AppConstants.MAX_MEMORY_CACHE_SIZE))
                    .build();
        }
    }

    public static int TYPE_WIFI = 1;
    public static int TYPE_ETHERNET = 2;
    public static int TYPE_NOT_CONNECTED = 0;

    public static int getConnectivityStatus(Context context) {
        ConnectivityManager cm = (ConnectivityManager) context
                .getSystemService(Context.CONNECTIVITY_SERVICE);

        NetworkInfo activeNetwork = cm.getActiveNetworkInfo();
        if (null != activeNetwork) {
            if(activeNetwork.getType() == ConnectivityManager.TYPE_WIFI)
                return TYPE_WIFI;

            if(activeNetwork.getType() == ConnectivityManager.TYPE_ETHERNET)
                return TYPE_ETHERNET;
        }
        return TYPE_NOT_CONNECTED;
    }

    public static void showToast(Context context, int id) {
        Toast.makeText(context, id, Toast.LENGTH_LONG).show();
    }
}
