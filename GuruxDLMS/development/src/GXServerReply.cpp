#include "../include/GXServerReply.h"


#define GBT_DATA_LENGTH_OFFSET  14 //the offset of the reply OCTET length
#define GBT_HEADER_SIZE 6

CGXServerReply::CGXServerReply()
{
	Reset();
}

CGXServerReply::CGXServerReply(CGXByteBuffer value)
{
	Reset();
	m_data = value;
}

void CGXServerReply::Reset()
{
	m_data.Clear();
	m_reply.Clear();
	m_count = 0;
	m_ReplyingToGbt = false;
	m_clientBlockNum = 0;
	m_clientBlockNumAcked = 0;
	m_clientWindowSize = 1;
	m_replyExist = false;
	m_serverBlockNum = 1;
}

