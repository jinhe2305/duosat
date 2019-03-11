package com.duosat.tv.model;

import java.util.ArrayList;

//==============================================================================
public class ArrayItemTopic extends ArrayList<EpgMenuItem> {

    //------------------------------------------------------------------------------
    public boolean IsSameWith(ArrayItemTopic a_arrItemTopic) {
        EpgMenuItem itemTopic;

        if (this.size() != a_arrItemTopic.size())
            return false;
        for (int i = 0; i < this.size(); i++) {
            itemTopic = this.get(i);
            if (!itemTopic.IsSameWith(a_arrItemTopic.get(i)))
                return false;
        }
        return true;
    }

    //------------------------------------------------------------------------------
    public boolean hasSameTimeRange(EpgMenuItem topic) {
        EpgMenuItem itemTopic;
        for (int i = 0; i < this.size(); i++)
        {
            itemTopic = get(i);
            if(itemTopic.m_dateTopicStart.equals(topic.m_dateTopicStart) || itemTopic.m_dateTopicEnd.equals(topic.m_dateTopicEnd))
                return true;
        }
        return false;
    }
    //------------------------------------------------------------------------------
}
