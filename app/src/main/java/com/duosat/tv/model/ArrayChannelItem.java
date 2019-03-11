package com.duosat.tv.model;

import java.util.ArrayList;

//==============================================================================
public class ArrayChannelItem extends ArrayList<ChannelMenuItem> {

    //------------------------------------------------------------------------------
    public boolean IsSameWith(ArrayChannelItem a_arrItemChannel) {
        ChannelMenuItem channelItem;

        if (this.size() != a_arrItemChannel.size())
            return false;

        for (int i = 0; i < this.size(); i++) {
            channelItem = this.get(i);
            if (!channelItem.IsSameWith(a_arrItemChannel.get(i)))
                return false;
        }
        return true;
    }

    //------------------------------------------------------------------------------
}

