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

#include "../include/GXReplyData.h"

CGXReplyData::CGXReplyData(
    DLMS_DATA_REQUEST_TYPES more,
    DLMS_COMMAND cmd,
    CGXByteBuffer& buff,
    bool complete)
{
    Clear();
    m_DataType = DLMS_DATA_TYPE_NONE;
    m_MoreData = more;
    m_Command = cmd;
    m_CommandType = 0;
    m_Data = buff;
    m_Complete = complete;
    m_Time = NULL;
}

CGXReplyData::CGXReplyData()
{
    Clear();
}

DLMS_DATA_TYPE CGXReplyData::GetValueType()
{
    return m_DataType;
}

void CGXReplyData::SetValueType(DLMS_DATA_TYPE value)
{
    m_DataType = value;
}

CGXDLMSVariant& CGXReplyData::GetValue()
{
    return m_DataValue;
}

void CGXReplyData::SetValue(CGXDLMSVariant& value)
{
    m_DataValue = value;
}

unsigned long CGXReplyData::GetReadPosition()
{
    return m_ReadPosition;
}

void CGXReplyData::SetReadPosition(unsigned long value)
{
    m_ReadPosition = value;
}

int CGXReplyData::GetPacketLength()
{
    return m_PacketLength;
}

void CGXReplyData::SetPacketLength(int value)
{
    m_PacketLength = value;
}

void CGXReplyData::SetCommand(DLMS_COMMAND value)
{
    m_Command = value;
}

void CGXReplyData::SetData(CGXByteBuffer& value)
{
    m_Data = value;
}

void CGXReplyData::SetComplete(bool value)
{
    m_Complete = value;
}

void CGXReplyData::SetTotalCount(int value)
{
    m_TotalCount = value;
}

void CGXReplyData::Clear()
{
    m_Time = NULL;
    m_MoreData = DLMS_DATA_REQUEST_TYPES_NONE;
    m_Command = DLMS_COMMAND_NONE;
    m_Data.Clear();
    m_Complete = false;
    m_Peek = false;
    m_TotalCount = 0;
    m_DataValue.Clear();
    m_ReadPosition = 0;
    m_PacketLength = 0;
    m_DataType = DLMS_DATA_TYPE_NONE;
    m_CipherIndex = 0;
}

bool CGXReplyData::IsMoreData()
{
    return m_MoreData != DLMS_DATA_REQUEST_TYPES_NONE;
}

DLMS_DATA_REQUEST_TYPES CGXReplyData::GetMoreData()
{
    return m_MoreData;
}

void CGXReplyData::SetMoreData(DLMS_DATA_REQUEST_TYPES value)
{
    m_MoreData = value;
}

DLMS_COMMAND CGXReplyData::GetCommand()
{
    return m_Command;
}

void CGXReplyData::SetCommandType(unsigned char value)
{
    m_CommandType = value;
}

unsigned char CGXReplyData::GetCommandType()
{
    return m_CommandType;
}


CGXByteBuffer& CGXReplyData::GetData()
{
    return m_Data;
}

bool CGXReplyData::IsComplete()
{
    return m_Complete;
}

int CGXReplyData::GetTotalCount()
{
    return m_TotalCount;
}

int CGXReplyData::GetCount()
{
    if (m_DataValue.vt == DLMS_DATA_TYPE_ARRAY)
    {
        return (int)m_DataValue.Arr.size();
    }
    return 0;
}

bool CGXReplyData::GetPeek()
{
    return m_Peek;
}

void CGXReplyData::SetPeek(bool value)
{
    m_Peek = value;
}

unsigned short CGXReplyData::GetCipherIndex()
{
    return m_CipherIndex;
}

void CGXReplyData::SetCipherIndex(unsigned short value)
{
    m_CipherIndex = value;
}

struct tm* CGXReplyData::GetTime()
{
    return m_Time;
}

void CGXReplyData::SetTime(struct tm* value)
{
    if (value == NULL)
    {
        if (m_Time != NULL)
        {
            free(m_Time);
        }
        m_Time = value;
    }
    else
    {
        if (m_Time == NULL)
        {
            m_Time = (struct tm*) malloc(sizeof(struct tm));
        }
        memcpy(m_Time, value, sizeof(struct tm));
    }
}


unsigned short CGXReplyData::GetBlockNumber()
{
	return m_BlockNumber;
}

void CGXReplyData::SetBlockNumber(unsigned short value)
{
	m_BlockNumber = value;
}

unsigned short CGXReplyData::GetBlockNumberAck()
{
	return m_BlockNumberAck;
}


void CGXReplyData::SetBlockNumberAck(unsigned short value)
{
	m_BlockNumberAck = value;
}

bool CGXReplyData::GetStreaming()
{
	return m_Streaming;
}

void CGXReplyData::SetStreaming(bool value)
{
	m_Streaming = value;
}

unsigned char CGXReplyData::GetWindowSize()
{
	return m_WindowSize;
}

void CGXReplyData::SetWindowSize(unsigned char  value)
{
	m_WindowSize = value;
}

 bool CGXReplyData::IsStreaming()
 {
	return GetStreaming() && (GetBlockNumberAck() * GetWindowSize())
			+ 1 > GetBlockNumber();
}
