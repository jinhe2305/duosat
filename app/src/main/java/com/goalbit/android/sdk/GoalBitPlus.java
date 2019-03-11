
package com.goalbit.android.sdk;

import android.content.Context;
import android.content.Intent;
import android.os.Build.VERSION;

import java.io.File;
import java.net.URI;
import java.net.URISyntaxException;

import static android.content.Context.MODE_PRIVATE;
import static android.content.Intent.FLAG_RECEIVER_FOREGROUND;

public class GoalBitPlus {
    private final String TAG = "GoalBitPlusSDK";
    private boolean loadError = false;
    private static GoalBitPlus instance = new GoalBitPlus();

    public native int startGPA(int version, String baseDir, String logDir);

    public native void stopGPA();

    public native void setGPAStreamingType(int var1);

    public native void setStartBuffer(int var1);

    public native void setUrgentBuffer(int var1);

    public native String checkVersion();

    public native int createSession(String var1, String var2, String var3, String var4);

    public native void DeleteSession(int id);

    public native String getPlayerURL(int var1);

    public native int getBufferStatus(int var1);

    public native String getQualities(int var1);

    public native void setQuality(int var1, int var2);

    public native int getQuality(int var1);

    public static GoalBitPlus getInstance() {
        return instance;
    }

    private GoalBitPlus() {
    }

    public boolean initialize(Context context, int verboseLevel) {
        Log.LOG = verboseLevel > 0;
        Log.d("GoalBitPlusSDK", "Initializing the SDK");
        /*
        if(!this.loadLibrary(context, "miniupnpc")) {
            this.loadError = true;
            return false;
        } else if(!this.loadLibrary(context, "gbsp")) {
            this.loadError = true;
            return false;
        } else*/
        if(!this.loadLibrary(context, "mongoose")) {
            this.loadError = true;
            return false;
        } else if(!this.loadLibrary(context, "gpa")) {
            this.loadError = true;
            return false;
        } else {
            String baseDir = this.getContentPath(context);
            Log.d("GoalBitPlusSDK", "baseDir = " + baseDir);
            File baseDirFolder = new File(baseDir);
            if(!baseDirFolder.exists() && !baseDirFolder.mkdirs()) {
                this.loadError = true;
                return false;
            } else {
                Log.d("GoalBitPlusSDK", "baseDir AVAILABLE!!");
                String logDir = null;
                if(verboseLevel > 1) {
                    logDir = this.getLoggingPath(context);
                    Log.d("GoalBitPlusSDK", "logDir = " + logDir);
                    File error = new File(logDir);
                    if(!error.exists() && !error.mkdir()) {
                        logDir = null;
                    }

                    Log.d("GoalBitPlusSDK", "logDir AVAILABLE!!");
                }

                int error1 = this.startGPA(VERSION.SDK_INT, baseDir, logDir);
                if(error1 != 0) {
                    Log.d("GoalBitPlusSDK", "ERROR initializing GPA");
                    this.loadError = true;
                    return false;
                } else {
                    Log.d("GoalBitPlusSDK", "The SDK was initialized");
                    return true;
                }
            }
        }
    }

    public void destroy(Context context) {
        Log.d("GoalBitPlusSDK", "Closing the SDK");
        if(!this.loadError) {
            this.stopGPA();
            String baseDir = this.getContentPath(context);
            File baseDirFolder = new File(baseDir);
            if(baseDirFolder.exists()) {
                baseDirFolder.deleteOnExit();
            }
        }

        Log.d("GoalBitPlusSDK", "The SDK was closed");
    }

    public void setStreamingType(int type) {
        if(!this.loadError) {
            switch(type) {
                case 1:
                case 2:
                    this.setGPAStreamingType(type);
                default:
            }
        }
    }

    public void setInitialBufferSize(int seconds) {
        if(!this.loadError) {
            if(seconds >= 0 && seconds <= 120) {
                this.setStartBuffer(seconds);
            }

        }
    }

    public void setUrgentBufferSize(int seconds) {
        if(!this.loadError) {
            if(seconds >= 0 && seconds <= 120) {
                this.setUrgentBuffer(seconds);
            }

        }
    }

    private boolean loadLibrary(Context context, String filename) {
        System.loadLibrary(filename);
        return true;
    }

    public String getContentPath(Context context) {
        String baseDir = context.getDir("goalbit", MODE_PRIVATE).getAbsolutePath();
        return baseDir;
    }

    public String getLoggingPath(Context context) {
        String logDir = context.getDir("goalbit", MODE_PRIVATE).getAbsolutePath();
        return logDir;
    }
}
