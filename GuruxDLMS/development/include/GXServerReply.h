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
#include "GXBytebuffer.h"
#include "GXDLMSSettings.h"
#include "enums.h"

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

    /**
     * is replying to GBT comand
     */
    bool m_ReplyingToGbt;

    /**
      * the current server block number
      */
     unsigned short m_serverBlockNum;


    /**
     * the current client block number
     */
    unsigned short m_clientBlockNum;

    /**
      * the current block number acknowledged
      */

    unsigned short m_clientBlockNumAcked;

    /**
      * the current client window size
      */
    unsigned char m_clientWindowSize;

    bool m_replyExist;

    /**
      * Is this the last block of a GBT session
      */
    bool m_IsLastBlock;
    
    /**
     * saved GBT blocks of the last window (transmitted by the server)  
     */
    CGXByteBuffer* m_pLastWindowReplies;

    /**
     * Is in mode of recovering lost GBT blocks (client didnt get blocks transmitted by teh server)  
     */
    bool m_IsRecoveringLostBlocks;
    
    /**
     * the current recovery block being tranmitted (this index is handled internally)  
     */
    unsigned short m_CurrRecoveryBlockIndex;
    /**
     * the last recovery block index that need to be tranmitted to teh client   
     */
    unsigned short m_RecoveryBlockEndIndex;
    
    /**
     * the block index of teh first block in the last window (server tranmits the blocks)   
     */
    unsigned short m_StartWindowBlockIndex;
    
    /**
     * the block index of teh first block in the last window (server tranmits the blocks)   
     */
    unsigned char m_MaxNumOfRecoveryBlocks;

    /**
     * the last block index of the block that was transmitted to the client    
     */
    unsigned short m_LastWindowRepliesMaxBlockIndex;
    
    /**
     * the BNA that should be sent for every recovery block that is sent to theclient
     */
    unsigned short m_RecoveryBlockBna;
    
    /**
     * the last recovery block index that need to be tranmitted to teh client   
     */
    //unsigned short m_RecoveryBlockBn;
    DLMS_COMMAND  m_Command;
    
    

public:

    /**
     * Constructor.
     *
     */
    CGXServerReply();
    /**
     * Constructor.
     *
     * @param value
     *            Received data.
     */

    CGXServerReply(CGXByteBuffer value);

    /**
      * reset the DB.
      *
      */

    void Reset();
    /**
     * @return the data
     */
    CGXByteBuffer& GetData()
    {
        return m_data;
    }

    /**
     * @param value
     *            The data to set.
     */
    void SetData(CGXByteBuffer& value)
    {
        m_data = value;
    }

    /**
     * @return The reply message.
     */
    CGXByteBuffer& GetReply()
    {
        return m_reply;
    }

    /**
     * @param value
     *            the replyMessages to set
     */
    void SetReply(CGXByteBuffer& value)
    {
        m_reply = value;
    }

    /**
     * @return Connection info.
     */
    CGXDLMSConnectionEventArgs& GetConnectionInfo()
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



   /**
	 * @return replying to GBT command.
	 */
	bool GetReplyingToGbt()
	{
		return m_ReplyingToGbt;
	}

	/**
	 * @param value
	 *             the replying to GBT command.
	 */
	void SetReplyingToGbt(bool value)
	{
		m_ReplyingToGbt = value;
	}

    /**
 	 * @return the current client block number received by the server.
 	 */
    unsigned short GetClientBlockNum()
 	{
 		return m_clientBlockNum;
 	}

 	/**
 	 * @param value
 	 *             the current client block number received by the server.
 	 */
 	void SetClientBlockNum(unsigned short value)
 	{
 		m_clientBlockNum = value;
 	}

     /**
  	 * @return the current client block number acknowledged.
  	 */
     unsigned short GetClientBlockNumAcked()
  	{
  		return m_clientBlockNumAcked;
  	}

  	/**
  	 * @param value
  	 *             the current client block number acknowledged.
  	 */
  	void SetClientBlockNumAcked(unsigned short value)
  	{
  		m_clientBlockNumAcked = value;
  	}

    /**
 	 * @return the current block number acknowledged.
 	 */
    unsigned char GetClientWindowSize()
 	{
 		return m_clientWindowSize;
 	}

 	/**
 	 * @param value
 	 *             the current block number acknowledged.
 	 */
 	void SetClientWindowSize(unsigned char value)
 	{
 		m_clientWindowSize = value;
 	}


 	void IncreaseserverBlockNum()
 	{
 		m_serverBlockNum++;
 	}


    /**
 	 * @return is this the last block of a GBT session.
 	 */
    bool GetIsLastBlock()
 	{
 		return m_IsLastBlock;
 	}

 	/**
 	 * @param value
 	 *             the last block of a GBT session.
 	 */
 	void SetIsLastBlock(bool value)
 	{
 		m_IsLastBlock = value;
 	}

    /**
 	 * @return if currently in process of recovering lost blocks of GBT session.
 	 */
    bool GetIsRecoveringLostBlocks()
 	{
 		return m_IsRecoveringLostBlocks;
 	}

 	/**
 	 * @param value
 	 *             set if currently in process of recovering lost blocks of GBT sessio.
 	 * @param recoveryStartBlockIndex
 	 * 				the first recovery block index
 	 * @param clientWindow
 	 * 				the client requested window for teh recovery
 	 * @param clientBlockIndex
 	 * 				the client recovery request block index. this number will be used for the recovery blocks BNA 							            
 	 */
 	void SetIsRecoveringLostBlocks(bool value, unsigned short recoveryStartBlockIndex = 1, unsigned char clientWindow = 1,unsigned short clientBlockIndex =1);
 	
    /**
     * @return The appropirate block for recovery.
     */
    bool GetRecoveryBlock(CGXByteBuffer& recoveredBlock);
 
    /**
     * @return true is the current block that was returned is the last recovery blcok.
     */
    bool IsLastRecoveryBlock()
    {
    	return (m_IsRecoveringLostBlocks && (m_CurrRecoveryBlockIndex > m_RecoveryBlockEndIndex));
    }
    
 	/**
 	 * @param startWindowBlockIndex
 	 *             save the block index of the first block of the window (block tranmitteed by the server)
 	 */
    bool SetStartWindowBlockIndex(unsigned short startWindowBlockIndex,unsigned char clientWindow);

  	/**
  	 * @param block
  	 *        	the block to be saved for recovery (block tranmitteed by the server)
  	 * @param blockIndex
  	 * 		    the block index of the block to be saved for recovery (block tranmitteed by the server)
  	 */
    bool SetRecoveryBlock(CGXByteBuffer& block,unsigned short blockIndex);
    
    /**
     * @return the last DLMS_COMMAND that was handled by the server.
     */
    DLMS_COMMAND GetCommand()
    {
        return m_Command;
    }

 	/**
 	 * @param value
 	 *             the last DLMS_COMMAND that was handled by the server
 	 */
    void SetCommand(DLMS_COMMAND value)
    {
        m_Command = value;
    }
};



#endif /* GXSERVERREPLYH */
