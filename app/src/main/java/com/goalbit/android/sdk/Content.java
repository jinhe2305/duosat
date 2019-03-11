
package com.goalbit.android.sdk;

public class Content {
    private String contentID = null;
    private String hlsServerURL = null;
    private String p2pTrackerURL = null;
    private String p2pManifestURL = null;

    public Content() {
    }

    public String getContentID() {
        return this.contentID;
    }

    public void setContentID(String contentID) {
        this.contentID = contentID;
    }

    public String getHlsServerURL() {
        return this.hlsServerURL;
    }

    public void setHlsServerURL(String hlsServerURL) {
        this.hlsServerURL = hlsServerURL;
    }

    public String getP2pTrackerURL() {
        return this.p2pTrackerURL;
    }

    public void setP2pTrackerURL(String p2pTrackerURL) {
        this.p2pTrackerURL = p2pTrackerURL;
    }

    public String getP2pManifestURL() {
        return this.p2pManifestURL;
    }

    public void setP2pManifestURL(String p2pManifestURL) {
        this.p2pManifestURL = p2pManifestURL;
    }
}
