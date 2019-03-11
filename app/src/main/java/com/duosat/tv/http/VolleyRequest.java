package com.duosat.tv.http;

import com.android.volley.AuthFailureError;
import com.android.volley.NetworkResponse;
import com.android.volley.Request;
import com.android.volley.Response;
import com.android.volley.RetryPolicy;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.StringRequest;
import com.duosat.tv.utils.AppConstants;
import com.duosat.tv.utils.Utils;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

public class VolleyRequest {
    public static void  getStringResponse(String url, final VolleyCallback resultCallback, String tag){
        url = Utils.makeAvaiableUrl(url);
        StringRequest stringRequest = new StringRequest(Request.Method.GET, url,
            new Response.Listener<String>() {
                @Override
                public void onResponse(String response) {
                if(resultCallback != null){
                    resultCallback.onSuccess(response);
                }
                }
            }, new Response.ErrorListener() {
                @Override
                public void onErrorResponse(VolleyError error) {
                if(resultCallback != null){
                    resultCallback.onError(error);
                }
                    }
        });

        stringRequest.setRetryPolicy(new RetryPolicy() {
            @Override
            public int getCurrentTimeout() {
                return 60000;
            }

            @Override
            public int getCurrentRetryCount() {
                return 60000;
            }

            @Override
            public void retry(VolleyError error) throws VolleyError {

            }
        });

        // Adding String request to request queue
        AppController.getInstance().addToRequestQueue(stringRequest, tag);
    }

    public static void  getStringResponsePost(String url, final HashMap<String, String> data, final VolleyCallback resultCallback, final String tag){
        StringRequest stringRequest = new StringRequest(Request.Method.POST, url,
            new Response.Listener<String>() {
                @Override
                public void onResponse(String response) {
                    if(resultCallback != null){
                        resultCallback.onSuccess(response);
                    }
                }
            }, new Response.ErrorListener() {
                @Override
                public void onErrorResponse(VolleyError error) {
                    if(resultCallback != null){
                        resultCallback.onError(error);
                    }
                }
        }){
            @Override
            public String getBodyContentType() {
                return "application/json; charset=UTF-8";
            }

            @Override
            public Map<String, String> getHeaders() throws AuthFailureError
            {
                Map<String, String> headers = super.getHeaders();

                if (headers == null
                        || headers.equals(Collections.emptyMap())) {
                    headers = new HashMap<String, String>();
                }

                headers.put("Content-Type", "application/x-www-form-urlencoded");
                if(tag != AppConstants.LOGIN_TAG) {
                    AppController.getInstance().addSessionCookie(headers);
                }
                return headers;
            }

            @Override
            protected Map<String, String> getParams() throws AuthFailureError {
                return data;
            }

            @Override
            protected Response<String> parseNetworkResponse(NetworkResponse response) {
                // since we don't know which of the two underlying network vehicles
                // will Volley use, we have to handle and store session cookies manually
                AppController.getInstance().checkSessionCookie(response.headers);

                return super.parseNetworkResponse(response);
            }
        };

        stringRequest.setRetryPolicy(new RetryPolicy() {
            @Override
            public int getCurrentTimeout() {
                return 60000;
            }

            @Override
            public int getCurrentRetryCount() {
                return 60000;
            }

            @Override
            public void retry(VolleyError error) throws VolleyError {

            }
        });

        // Adding String request to request queue
        AppController.getInstance().addToRequestQueue(stringRequest, tag);
    }
}
