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

int CGXServerReply::GetBlock(unsigned short serverAddress, unsigned short clientAddress, unsigned short maxPduSize, CGXByteBuffer& block,unsigned short blockNum, bool& isLastBlock)
{
	unsigned long offset;
	unsigned long len;
	unsigned short blockSize = maxPduSize;
	isLastBlock = false;
	unsigned char bc = 0x41; //LB=0,  STR=1, W=1

	if(m_reply.GetSize() == 0)
	{
		return -1;
	}

	//block size will have to add place for teh bytes that come before it
	blockSize -= GBT_DATA_LENGTH_OFFSET;

	printf("##205 %d\n",blockSize);
	//now calculate how many bytes are needed to write the length of the block size and substract them from the block size
	blockSize -= GXHelpers::GetObjectCountSizeInBytes(blockSize);
	printf("##206 %d\n",blockSize);

	// Data length.
	m_reply.SetPosition(GBT_DATA_LENGTH_OFFSET);
	GXHelpers::GetObjectCount(m_reply,len);

	printf("##207 %d\n",len);
	//get the offest to the start of the data
	offset = GBT_DATA_LENGTH_OFFSET + GXHelpers::GetObjectCountSizeInBytes(len);

	printf("##208 %d\n",offset);
	//get the offset to the current block to be transmitted
	offset += blockNum * blockSize;

	printf("##209 %d %d\n",blockNum,offset);
	if(offset >=  m_reply.GetSize())
	{
		return -1;
	}

	printf("##209 %d\n",offset);
	m_reply.SetPosition(offset);

	//now start writing the reply data
	//first copy the frame header
	block.Clear();
	// Add version.
	block.SetUInt16(1);
	block.SetUInt16(serverAddress);
	block.SetUInt16(clientAddress);


	len =  ((m_reply.GetSize() - m_reply.GetPosition()) > blockSize ) ? blockSize : (m_reply.GetSize() - m_reply.GetPosition());

	block.SetUInt16((unsigned short)len + GBT_HEADER_SIZE + GXHelpers::GetObjectCountSizeInBytes(len));

	block.SetUInt8(DLMS_COMMAND_GENERAL_BLOCK_TRANSFER);

	if((m_reply.GetPosition() + len) == m_reply.GetSize())
	{
		isLastBlock = true;
		bc = 0x81; //set LB=1,  STR=0, W=1
	}

	block.SetUInt8(bc);
	block.SetUInt16(m_serverBlockNum);
	block.SetUInt16(m_clientBlockNum);

	//set the length of teh octet
	GXHelpers::SetObjectCount(len,block);

    // set the block Data
	block.Set(&m_reply, m_reply.GetPosition(), len);

	return 0;

}
