//
 // --------------------------------------------------------------------------
 //  Gurux Ltd
 //
 //
 //
 // Filename:        $HeadURL$
 //
 // Version:         $Revision$,
 //                  $Date$
 //                  $Author$
 //
 // Copyright (c) Gurux Ltd
 //
 //---------------------------------------------------------------------------
 //
 //  DESCRIPTION
 //
 // This file is a part of Gurux Device Framework.
 //
 // Gurux Device Framework is Open Source software; you can redistribute it
 // and/or modify it under the terms of the GNU General Public License
 // as published by the Free Software Foundation; version 2 of the License.
 // Gurux Device Framework is distributed in the hope that it will be useful,
 // but WITHOUT ANY WARRANTY; without even the implied warranty of
 // MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 // See the GNU General Public License for more details.
 //
 // More information of Gurux products: http://www.gurux.org
 //
 // This code is licensed under the GNU General Public License v2.
 // Full text may be retrieved at http://www.gnu.org/licenses/gpl-2.0.txt
 //---------------------------------------------------------------------------

#ifndef GXSERVERREPLY_H
#define GXSERVERREPLY_H

#include "GXDLMSConnectionEventArgs.h"
#include "GXByteBuffer.h"

class CGXServerReply {

    /**
     * Connection info.
     */
    CGXDLMSConnectionEventArgs connectionInfo;

    /**
     * Server received data.
     */
    CGXByteBuffer  m_data;

    /**
     * Server reply message.
     */
    CGXByteBuffer m_reply;

    /**
     * Message count to send.
     */
    int m_count;

public:
    /**
     * Constructor.
     *
     * @param value
     *            Received data.
     */

    CGXServerReply(CGXByteBuffer value)
	{
        m_data = value;
        m_count = 0;
    }

    /**
     * @return the data
     */
    CGXByteBuffer GetData()
    {
        return m_data;
    }

    /**
     * @param value
     *            The data to set.
     */
    void SetData(CGXByteBuffer value)
    {
        m_data = value;
    }

    /**
     * @return The reply message.
     */
    CGXByteBuffer GetReply()
    {
        return m_reply;
    }

    /**
     * @param value
     *            the replyMessages to set
     */
    void SetReply(CGXByteBuffer value)
    {
        m_reply = value;
    }

    /**
     * @return Connection info.
     */
    CGXDLMSConnectionEventArgs GetConnectionInfo()
    {
        return connectionInfo;
    }

    /**
     * @param value
     *            Connection info.
     */
     void SetConnectionInfo(CGXDLMSConnectionEventArgs value)
     {
        connectionInfo = value;
    }

    /**
     * @return Is GBT streaming in progress.
     */
     bool IsStreaming()
     {
        return GetCount() != 0;
    }

    /**
     * @return Message count to send.
     */
    int GetCount()
    {
        return m_count;
    }

    /**
     * @param value
     *            Message count to send.
     */
    void SetCount(int value)
    {
        m_count = value;
    }
};



#endif /* GXSERVERREPLYH */
