package com.duosat.tv.model;

import java.io.Serializable;
import java.util.Date;

public class EpgMenuItem implements Serializable{
    public String strName;
    public String strTime;
    public String strVideoLength;
    public String strMark;
    public String strKind;
    public String strDescription;
    public Date   m_dateTopicStart;
    public Date   m_dateTopicEnd;
    public boolean m_bIsSelected;

    //------------------------------------------------------------------------------
    public boolean IsSameWith(EpgMenuItem a_itemTopic) {
        if (strName != a_itemTopic.strName || (strName != null && a_itemTopic.strName != null && !strName.equals(a_itemTopic.strName))) return false;
        if (strTime != a_itemTopic.strTime || (strTime != null && a_itemTopic.strTime != null && !strTime.equals(a_itemTopic.strTime))) return false;
        if (strVideoLength != a_itemTopic.strVideoLength || (strVideoLength != null && a_itemTopic.strVideoLength != null && !strVideoLength.equals(a_itemTopic.strVideoLength))) return false;
        if (strMark != a_itemTopic.strMark || (strMark != null && a_itemTopic.strMark != null && !strMark.equals(a_itemTopic.strMark))) return false;
        if (strKind != a_itemTopic.strKind || (strKind != null && a_itemTopic.strKind != null && !strKind.equals(a_itemTopic.strKind))) return false;
        if (strDescription != a_itemTopic.strDescription || (strDescription != null && a_itemTopic.strDescription != null && !strDescription.equals(a_itemTopic.strDescription))) return false;
        if (m_dateTopicStart != a_itemTopic.m_dateTopicStart || (m_dateTopicStart != null && a_itemTopic.m_dateTopicStart != null && !m_dateTopicStart.equals(a_itemTopic.m_dateTopicStart))) return false;
        if (m_dateTopicEnd != a_itemTopic.m_dateTopicEnd || (m_dateTopicEnd != null && a_itemTopic.m_dateTopicEnd != null && !m_dateTopicEnd.equals(a_itemTopic.m_dateTopicEnd))) return false;

        return true;
    }
}
