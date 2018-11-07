#include "../include/GXServerReply.h"


#define GBT_DATA_LENGTH_OFFSET  14 //the offset of the reply OCTET length
#define GBT_HEADER_OFFSET 8
#define GBT_HEADER_SIZE 6
#define GBT_BLOCK_CONTROL_OFFSET 1
#define GBT_BLOCK_NUMBER_OFFSET 2
#define GBT_BLOCK_NUMBER_ACK_OFFSET 4

CGXServerReply::CGXServerReply()
{
	m_pLastWindowReplies = NULL;
	m_MaxNumOfRecoveryBlocks = 0;

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
	m_IsLastBlock = true;


   	if(NULL != m_pLastWindowReplies)
   	{
   		delete [] m_pLastWindowReplies;
		m_pLastWindowReplies = NULL;
   	}

	m_MaxNumOfRecoveryBlocks = 0;

    m_IsRecoveringLostBlocks = false;
    m_CurrRecoveryBlockIndex = 1;
    m_RecoveryBlockEndIndex = 1;
    m_StartWindowBlockIndex = 1;
    m_LastWindowRepliesMaxBlockIndex = 0;
    m_Command = DLMS_COMMAND_NONE;
}

void CGXServerReply::SetIsRecoveringLostBlocks(bool value, unsigned short recoveryStartBlockIndex, unsigned char clientWindow,unsigned short clientBlockIndex)
{
	m_IsRecoveringLostBlocks = value;
	if(m_IsRecoveringLostBlocks)
	{
		m_CurrRecoveryBlockIndex = recoveryStartBlockIndex;
		m_RecoveryBlockEndIndex = recoveryStartBlockIndex + clientWindow -1;
		//in case the we have less blocks than the requested window then we will
		//send only the available blocks
		if(m_RecoveryBlockEndIndex > m_LastWindowRepliesMaxBlockIndex)
		{
			m_RecoveryBlockEndIndex = m_LastWindowRepliesMaxBlockIndex;
		}

		m_RecoveryBlockBna = clientBlockIndex;
	}
}


bool CGXServerReply::GetRecoveryBlock(CGXByteBuffer& recoveredBlock)
{

	if((m_IsRecoveringLostBlocks == false) ||
		(m_CurrRecoveryBlockIndex - m_StartWindowBlockIndex >= m_MaxNumOfRecoveryBlocks) ||
		(NULL == m_pLastWindowReplies) ||
		(m_LastWindowRepliesMaxBlockIndex == 0))
	{
		return false;
	}
	else
	{

		recoveredBlock = m_pLastWindowReplies[m_CurrRecoveryBlockIndex - m_StartWindowBlockIndex];
		//update teh recovered bloc GBT header

		//handle the streaming bit (STR bit)
		//check if this is the last block in the recovery and turn of the STR bit
		unsigned char* pCb;
		pCb = recoveredBlock.GetData();
		if(NULL == pCb)
		{
			return false;
		}


		if(m_CurrRecoveryBlockIndex >= m_RecoveryBlockEndIndex)
		{
			//turn off the STR bit (bit 6)
			*(pCb + GBT_BLOCK_CONTROL_OFFSET) &= 0xbf;
		}
		else
		{
			//turn on the STR bit (bit 6)
			*(pCb + GBT_BLOCK_CONTROL_OFFSET) |= 0x40;
		}


		//update the block number Ack
		recoveredBlock.SetUInt16ByIndex(GBT_BLOCK_NUMBER_ACK_OFFSET,m_RecoveryBlockBna);

		//now increase the current recovery index by 1 to point the next recovery block
		m_CurrRecoveryBlockIndex++;

		return true;
	}

}

bool CGXServerReply::SetRecoveryBlock(CGXByteBuffer& block,unsigned short blockIndex)
{
	unsigned short recoveryIndex = blockIndex - m_StartWindowBlockIndex;

	if((NULL == m_pLastWindowReplies) || (recoveryIndex >= m_MaxNumOfRecoveryBlocks))
	{
		return false;
	}

	m_pLastWindowReplies[recoveryIndex] = block;
	m_LastWindowRepliesMaxBlockIndex = blockIndex;
	return true;
}


bool CGXServerReply::SetStartWindowBlockIndex(unsigned short startWindowBlockIndex,unsigned char clientWindow)
{

   	if(NULL != m_pLastWindowReplies)
   	{
   		delete [] m_pLastWindowReplies;
   	}

	m_StartWindowBlockIndex = startWindowBlockIndex;
	m_MaxNumOfRecoveryBlocks = clientWindow;

	m_pLastWindowReplies = new CGXByteBuffer[m_MaxNumOfRecoveryBlocks];

	if(NULL == m_pLastWindowReplies)
	{
		return false;
	}


}

