package com.duosat.tv.model;

import com.duosat.tv.utils.Utils;

import java.util.Date;

public class ChannelMenuItem{
    public String               channelMark;
    public String               channelName;
    public String               channelGenre;
    public String               channelURL;
    public String               channelSrc;
    public String               channelDescription;
    public String               channelEPGData;
    public String               channelNumber;
    public boolean              isLike;
    public String               channelFeed;
    public String               channelPackage;
    public String               channelP2PUrl;
    public String               strTitle;
    public String               strSubTitle;
    public String               strTime;
    public ArrayItemTopic		m_arrItemTopic = new ArrayItemTopic();

    public boolean IsSameWith(ChannelMenuItem a_itemChannel) {
        if (!channelMark.equals(a_itemChannel.channelMark))					        return false;
        if (!channelName.equals(a_itemChannel.channelName)) 		                return false;
        if (!channelGenre.equals(a_itemChannel.channelGenre))					    return false;
        if (!channelURL.equals(a_itemChannel.channelURL))					        return false;
        if (!channelSrc.equals(a_itemChannel.channelSrc)) 		                    return false;
        if (!channelDescription.equals(a_itemChannel.channelDescription))			return false;
        if (!channelEPGData.equals(a_itemChannel.channelEPGData)) 		            return false;
        if (!channelNumber.equals(a_itemChannel.channelNumber))				        return false;
        if (isLike != a_itemChannel.isLike) 		                                return false;
        if (channelFeed.equals(a_itemChannel.channelFeed)) 		                                return false;
        if (channelPackage.equals(a_itemChannel.channelPackage)) 		                    return false;
        if (!channelP2PUrl.equals(a_itemChannel.channelP2PUrl)) 		            return false;
        if (!strTitle.equals(a_itemChannel.strTitle)) 		                        return false;
        if (!strSubTitle.equals(a_itemChannel.strSubTitle)) 	                    return false;
        if (!m_arrItemTopic.IsSameWith(a_itemChannel.m_arrItemTopic))	            return false;
        return true;
    }

    public int getCurrentTopicIndex() {
        if(m_arrItemTopic.size() == 0)
            return -1;

        Date currentDate = Utils.CurrentTime();
        int index = 0;
        for (EpgMenuItem topic : m_arrItemTopic) {
            if(topic.m_dateTopicStart != null && topic.m_dateTopicEnd != null && topic.m_dateTopicStart.before(currentDate) && topic.m_dateTopicEnd.after(currentDate))
                break;
            index++;
        }

        if(index == m_arrItemTopic.size())
            index = 0;

        return index;
    }
}
