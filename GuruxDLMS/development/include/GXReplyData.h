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

#ifndef GXREPLYDATA_H
#define GXREPLYDATA_H

#include "GXDLMSConverter.h"
#include "GXBytebuffer.h"
#include "GXDLMSVariant.h"

class CGXReplyData
{
private:
    /**
     * Is more data available.
     */
    DLMS_DATA_REQUEST_TYPES m_MoreData;
    /**
     * Received command.
     */
    DLMS_COMMAND m_Command;

    /**
    * Received command type.
    */
    unsigned char m_CommandType;

    /**
     * Received data.
     */
    CGXByteBuffer m_Data;
    /**
     * Is frame complete.
     */
    bool m_Complete;

    /**
     * Read value.
     */
    CGXDLMSVariant m_DataValue;

    /**
     * Expected count of element in the array.
     */
    int m_TotalCount;

    /**
     * Last read position. This is used in peek to solve how far data is read.
     */
    unsigned long m_ReadPosition;

    /**
     * Packet Length.
     */
    int m_PacketLength;

    /**
     * Try Get value.
     */
    bool m_Peek;

    DLMS_DATA_TYPE m_DataType;

    /**
     * Cipher index is position where data is decrypted.
     */
    unsigned short m_CipherIndex;

    /**
     * Data notification date time.
     */
    struct tm* m_Time;

    /**
     * GBT block number.
     */
    unsigned short m_BlockNumber;
    /**
     * GBT block number ACK.
     */
    unsigned short m_BlockNumberAck;
    /**
     * Is GBT streaming in use.
     */
    bool m_Streaming;
    /**
     * GBT Window size. This is for internal use.
     */
    unsigned char m_WindowSize;


public:
    /**
     * Constructor.
     *
     * @param more
     *            Is more data available.
     * @param cmd
     *            Received command.
     * @param buff
     *            Received data.
     * @param forComplete
     *            Is frame complete.
     */
    CGXReplyData(
        DLMS_DATA_REQUEST_TYPES more,
        DLMS_COMMAND cmd,
        CGXByteBuffer& buff,
        bool complete);

    /**
     * Constructor.
     */
    CGXReplyData();

    DLMS_DATA_TYPE GetValueType();

    void SetValueType(DLMS_DATA_TYPE value);


    CGXDLMSVariant& GetValue();


    void SetValue(CGXDLMSVariant& value);

    unsigned long GetReadPosition();

    void SetReadPosition(unsigned long value);

    int GetPacketLength();

    void SetPacketLength(int value);

    void SetCommand(DLMS_COMMAND value);

    void SetCommandType(unsigned char value);

    unsigned char GetCommandType();


    void SetData(CGXByteBuffer& value);

    void SetComplete(bool value);

    void SetTotalCount(int value);

    /**
     * Reset data values to default.
     */
    void Clear();

    /**
     * @return Is more data available.
     */
    bool IsMoreData();

    /**
     * Is more data available.
     *
     * @return Return None if more data is not available or Frame or Block type.
     */
    DLMS_DATA_REQUEST_TYPES GetMoreData();

    void SetMoreData(DLMS_DATA_REQUEST_TYPES value);


    /**
     * Get received command.
     *
     * @return Received command.
     */
    DLMS_COMMAND GetCommand();

    /**
     * Get received data.
     *
     * @return Received data.
     */
    CGXByteBuffer& GetData();

    /**
     * Is frame complete.
     *
     * @return Returns true if frame is complete or false if bytes is missing.
     */
    bool IsComplete();

    /**
     * Get total count of element in the array. If this method is used peek must
     * be Set true.
     *
     * @return Count of element in the array.
     * @see peek
     * @see GetCount
     */
    int GetTotalCount();

    /**
     * Get count of read elements. If this method is used peek must be Set true.
     *
     * @return Count of read elements.
     * @see peek
     * @see GetTotalCount
     */
    int GetCount();

    /**
     * Get is value try to peek.
     *
     * @return Is value try to peek.
     * @see GetCount
     * @see GetTotalCount
     */
    bool GetPeek();

    /**
     * Set is value try to peek.
     *
     * @param value
     *            Is value try to peek.
     */
    void SetPeek(bool value);

    /**
     * @return Cipher index is position where data is decrypted.
     */
    unsigned short GetCipherIndex();

    /**
     * @param cipherIndex
     *            Cipher index is position where data is decrypted.
     */
    void SetCipherIndex(unsigned short value);

     /**
     * @return Data notification date time.
     */
    struct tm* GetTime();


    /**
     * @param time
     *            Data notification date time.
     */
    void SetTime(struct tm* value);


    /**
     * @return GBT block number.
     */
    unsigned short GetBlockNumber();

     /**
     * @param value
     *            GBT block number.
     */
    void SetBlockNumber(unsigned short value);

     /**
     * @return GBT block number ACK.
     */
    unsigned short int GetBlockNumberAck();

     /**
     * @param value
     *            GBT block number ACK.
     */
    void SetBlockNumberAck(unsigned short value);

     /**
     * @return Is GBT streaming in use.
     */
    bool GetStreaming();

     /**
     * @param value
     *            Is GBT streaming in use.
     */
     void SetStreaming(bool value);

     /**
     * @return GBT Window size. This is for internal use.
     */
     unsigned char GetWindowSize();

     /**
     * @param value
     *            GBT Window size. This is for internal use.
     */
    void SetWindowSize(unsigned char value);

     /**
     * @return Is GBT streaming.
     */
    bool IsStreaming();
};

#endif //GXREPLYDATA_H
