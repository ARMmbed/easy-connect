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
// and/or modify it under the terms of the GNU General License
// as published by the Free Software Foundation; version 2 of the License.
// Gurux Device Framework is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General License for more details.
//
// More information of Gurux products: http://www.gurux.org
//
// This code is licensed under the GNU General License v2.
// Full text may be retrieved at http://www.gnu.org/licenses/gpl-2.0.txt
//---------------------------------------------------------------------------
#ifndef GXDLMSLNPARAMETERS_H
#define GXDLMSLNPARAMETERS_H

#include "GXBytebuffer.h"
#include "GXDLMSSettings.h"

/**
 * LN Parameters
 */
class CGXDLMSLNParameters
{
private:

    /**
     * DLMS settings.
     */
    CGXDLMSSettings *m_Settings;
    /**
     * DLMS command.
     */
    DLMS_COMMAND m_Command;
    /**
     * Request type.
     */
    int m_RequestType;
    /**
     * Attribute descriptor.
     */
    CGXByteBuffer* m_AttributeDescriptor;
    /**
     * Data.
     */
    CGXByteBuffer* m_Data;
    /**
     * Send date and time. This is used in Data notification messages.
     */
    struct tm* m_Time;
    /**
     * Reply status.
     */
    int m_Status;
    /**
     * Are there more data to send or more data to receive.
     */
    bool m_MultipleBlocks;
    /**
     * Is this last block in send.
     */
    bool m_LastBlock;
    /**
     * Block index.
     */
    unsigned long m_BlockIndex;
    /**
    * Invoke ID.
    */
    unsigned long m_InvokeId;
    /**
     * Block number acknowledged in GBT.
     */
    unsigned short m_BlockNumberAck;
    /**
     * GBT window size.
     */
    unsigned char m_WindowSize;
    /**
     * Is GBT streaming used.
     */
    bool m_Streaming;

public:
    /**
     * Constructor.
     *
     * @param settings
     *            DLMS settings.
     * @param command
     *            Command.
     * @param commandType
     *            DLMS_COMMAND type.
     * @param attributeDescriptor
     *            Attribute descriptor.
     * @param data
     *            Data.
     */
    CGXDLMSLNParameters(CGXDLMSSettings* settings,
        unsigned long invokeId,
        DLMS_COMMAND command,
        int commandType,
        CGXByteBuffer* attributeDescriptor,
        CGXByteBuffer* data,
        int status);

    /**
     * @return DLMS settings.
     */
    CGXDLMSSettings* GetSettings();

    /**
     * @return DLMS command.
     */
    DLMS_COMMAND GetCommand();

    /**
     * @param value
     *            the command to set
      */
     void SetCommand(DLMS_COMMAND value);

    /**
     * @return Request type.
     */
    int GetRequestType();

    /**
     * @param value
     *            the requestType to set
     */
    void SetRequestType(int value);

    /**
     * @return the attributeDescriptor
     */
    CGXByteBuffer* GetAttributeDescriptor();

    /**
     * @return the data
     */
    CGXByteBuffer* GetData();

    /**
     * @return the time
     */
    struct tm* GetTime();

    /**
     * @param value
     *            the time to set
     */
    void SetTime(struct tm* value);

    /**
     * @return Status.
     */
    int GetStatus();

    /**
     * @param value
     *            Status to set
     */
    void SetStatus(int value);

    /**
     * @return the multipleBlocks
     */
    bool IsMultipleBlocks();

    /**
     * @param value
     *            the multipleBlocks to set
     */
    void SetMultipleBlocks(bool value);

    /**
     * @return the lastBlock
     */
    bool IsLastBlock();

    /**
     * @param value
     *            Block index.
     */
    void SetLastBlock(bool value);

    /**
     * @return Block index.
     */
    unsigned long GetBlockIndex();

    /**
     * @param value
     *            the blockIndex to set
     */
    void SetBlockIndex(unsigned long value);

    /**
    * @return Get Invoke ID and priority. This can be used for Priority
    *         Management.
    */
    unsigned long GetInvokeId();

    /**
    * @param value
    *            Set Invoke ID and priority. This can be used for Priority
    *            Management.
    */
    void SetInvokeId(unsigned long value);


    /**
    * @return the Block number acknowledged in GBT. This is used for GBT handling
    *         Management.
    */
    unsigned short GetBlockNumberAck();

    /**
    * @param value
    *            Set the Block number acknowledged in GBT. This is used for GBT handling.
    */
    void SetBlockNumberAck(unsigned short value);


    /**
    * @return the GBT window size. This is used for GBT handling
    */
    unsigned char GetWindowSize();

    /**
    * @param value
    *            Set the GBT window size. This is used for GBT handling.
    */
    void SetWindowSize(unsigned char value);

    /**
    * @return the GBT Streaming parameter. This is used for GBT handling
    */
    bool GetStreaming();

    /**
    * @param value
    *            Set the GBT Streaming parameter. This is used for GBT handling.
    */
    void SetStreaming(bool value);

    /**
    * @return the client GBT window size. This is used for GBT handling
    */
    unsigned char GetClientWindowSize();

    /**
    * @return the client Block number acknowledged in GBT. This is used for GBT handling
    *         Management.
    */
    unsigned short GetClientBlockNumberAck();
};

#endif //GXDLMSLNPARAMETERS_H
