
package com.goalbit.android.sdk;

import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

public class ContentLoader {
    public Content getContent(String contentURL) throws IOException, XmlPullParserException {
        contentURL = contentURL.replace("goalbit://", "http://");
        URL url = new URL(contentURL);
        HttpURLConnection conn = (HttpURLConnection)url.openConnection();
        conn.setReadTimeout(10000);
        conn.setConnectTimeout(15000);
        conn.setRequestMethod("GET");
        conn.setDoInput(true);
        conn.connect();
        InputStream stream = conn.getInputStream();
        return this.parse(stream);
    }

    private Content parse(InputStream xmlContent) throws XmlPullParserException, IOException {
        Content content = new Content();
        XmlPullParserFactory factory = XmlPullParserFactory.newInstance();
        factory.setNamespaceAware(true);
        XmlPullParser parser = factory.newPullParser();
        parser.setInput(xmlContent, null);
        String text = null;
        String tagname = null;

        for(int eventType = parser.getEventType(); eventType != 1; eventType = parser.next()) {
            tagname = parser.getName();
            switch(eventType) {
                case 3:
                    if(tagname.equalsIgnoreCase("id")) {
                        content.setContentID(text);
                    } else if(tagname.equalsIgnoreCase("tracker")) {
                        content.setP2pTrackerURL(text);
                    } else if(tagname.equalsIgnoreCase("hls")) {
                        content.setHlsServerURL(text);
                    } else if(tagname.equalsIgnoreCase("p2p_manifest")) {
                        content.setP2pManifestURL(text);
                    }
                    break;
                case 4:
                    text = parser.getText();
            }
        }

        return content;
    }
}
