package com.duosat.tv.main;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.duosat.tv.R;
import com.duosat.tv.http.AppController;
import com.duosat.tv.http.DuosatAPI;
import com.duosat.tv.http.DuosatAPIConstant;
import com.duosat.tv.http.VolleyCallback;
import com.duosat.tv.utils.AppConstants;
import com.duosat.tv.utils.Utils;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

public class LoginActivity extends Activity {

    EditText            m_edtUser;
    EditText            m_edtPassword;
    TextView            m_tvUserErr;
    TextView            m_tvPasswordErr;
    LinearLayout        m_llError;
    TextView            m_tvDeviceError;
    ProgressBar         m_pbProgress;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.login_layout);

        initControls();
        setEventListener();

        m_edtUser.setText(Utils.getDeviceID(this));
        m_edtPassword.setText(Utils.getDeviceID(this));
//        if(!m_edtUser.getText().toString().isEmpty() && Boolean.valueOf(Utils.getSharePreferenceValue(this, AppConstants.AUTO_LOGIN, String.valueOf(false))))
            onLogin();
    }

    @Override
    public void onBackPressed() {
        AppController.getInstance().cancelPendingRequests(AppConstants.LOGIN_TAG);
        super.onBackPressed();
        finish();
    }

    private void initControls() {
        m_edtUser = (EditText)findViewById(R.id.edtUser);
        m_edtPassword = (EditText)findViewById(R.id.edtPassword);
        m_tvUserErr = (TextView) findViewById(R.id.tvUserError);
        m_tvPasswordErr = (TextView)findViewById(R.id.tvPasswordError);
        m_pbProgress  = (ProgressBar)findViewById(R.id.pbProgress);
        m_llError = (LinearLayout)findViewById(R.id.llError);
        m_tvDeviceError = (TextView)findViewById(R.id.tvDeviceError);
    }

    private void setEventListener() {
        findViewById(R.id.btnLogin).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onLogin();
            }
        });
    }

    private void onLogin() {
        boolean bError = false;
//        if (m_edtUser.getText().toString().isEmpty()) {
//            m_tvUserErr.setText(getString(R.string.user_empty));
//            bError = true;
//        } else {
//            m_tvUserErr.setText("");
//        }
//
//        if (m_edtPassword.getText().toString().isEmpty()) {
//            m_tvPasswordErr.setText(getString(R.string.password_empty));
//            bError = true;
//        } else {
//            m_tvPasswordErr.setText("");
//        }

        if (!bError) {
            loginProcess();
        }
    }

    private void loginProcess() {
        m_pbProgress.setVisibility(View.VISIBLE);

        final String userId = m_edtUser.getText().toString();
        final String password = m_edtPassword.getText().toString();

//        final String userId = "aa:bb:cc:dd:ee:ff";
//        final String password = "aa:bb:cc:dd:ee:ff";

        DuosatAPI.loginUser(userId, password, new VolleyCallback() {
            @Override
            public void onSuccess(String result) {
                m_pbProgress.setVisibility(View.GONE);

                try {
                    JSONObject jsonObj = new JSONObject(result);
                    JSONObject rolesObj = jsonObj.getJSONObject(DuosatAPIConstant.ITEM_USER).getJSONObject(DuosatAPIConstant.ITEM_FIELD_ROLES);
                    jsonObj = jsonObj.getJSONObject(DuosatAPIConstant.ITEM_USER).getJSONObject(DuosatAPIConstant.ITEM_FIELD_MAC_ADDRESS).getJSONArray(DuosatAPIConstant.ITEM_UND).getJSONObject(0);

                    String macAddress = jsonObj.getString(DuosatAPIConstant.ITEM_VALUE);

                    if(userId.equalsIgnoreCase(macAddress)) {
                        Utils.setSharePreferenceValue(LoginActivity.this, AppConstants.LOGIN_USER, userId);
                        Utils.setSharePreferenceValue(LoginActivity.this, AppConstants.LOGIN_PASSWORD, password);
                        Utils.setSharePreferenceValue(LoginActivity.this, AppConstants.MAC_ADDRESS, macAddress);
//                        Utils.setSharePreferenceValue(LoginActivity.this, AppConstants.AUTO_LOGIN, String.valueOf(true));

                        AppConstants.ROLES_LIST.clear();
                        JSONArray rolesArray = rolesObj.names();
                        for (int i = 0; i < rolesArray.length(); i++) {
                            AppConstants.ROLES_LIST.add(rolesArray.getString(i));
                        }

                        Utils.setSharePreferenceValue(LoginActivity.this, AppConstants.ACCOUNT_SUBSCRIBED, String.valueOf(rolesObj.has("4")));

                        String[] lists = Utils.getSharePreferenceValue(LoginActivity.this, AppConstants.FAVORITE_ARRAY, "").split(",");
                        for (String faourite: lists) {
                            AppConstants.FAVORITE_CHANNELS.add(faourite);
                        }

                        Intent intent = new Intent(LoginActivity.this, MainActivity.class);
                        startActivity(intent);

                        finish();
                    }
                    else {
                        m_pbProgress.setVisibility(View.GONE);
                        m_tvUserErr.setText(getString(R.string.device_error));
                        m_llError.setVisibility(View.VISIBLE);
                        m_tvDeviceError.setText(getString(R.string.device_error));
                    }
                } catch (JSONException e) {
                    e.printStackTrace();
                }
            }

            @Override
            public void onError(Object error) {
                m_pbProgress.setVisibility(View.GONE);
                m_llError.setVisibility(View.VISIBLE);
                m_tvDeviceError.setText(getString(R.string.device_error));
            }
        }, AppConstants.LOGIN_TAG);
    }
}
