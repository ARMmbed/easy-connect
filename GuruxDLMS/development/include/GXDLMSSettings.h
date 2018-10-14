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

#ifndef GXDLMSSETTINGS_H
#define GXDLMSSETTINGS_H

#include "enums.h"
#include "GXBytebuffer.h"
#include "GXDLMSLimits.h"
#include "GXDLMSObjectCollection.h"
#include "GXCipher.h"

#define PRIVATE_KEY_SIZE 32
#define PUBLIC_KEY_SIZE 64

// Server sender frame sequence starting number.
const unsigned char SERVER_START_SENDER_FRAME_SEQUENCE = 0x1E;

// Server receiver frame sequence starting number.
const unsigned char SERVER_START_RECEIVER_FRAME_SEQUENCE = 0xFE;

// Client sender frame sequence starting number.
const unsigned char CLIENT_START_SENDER_FRAME_SEQUENCE = 0x10;

// Client receiver frame sequence starting number.
const unsigned char CLIENT_START_RCEIVER_FRAME_SEQUENCE = 0xE;
// DLMS version number.
const unsigned char DLMS_VERSION = 6;

const unsigned short MAX_RECEIVE_PDU_SIZE = 0xFFFF;

//[ecdsa]
// originator - always the client
// recipient - always the server
typedef struct keys
{
	unsigned char m_private[PRIVATE_KEY_SIZE];
	unsigned char m_recipient_public[PUBLIC_KEY_SIZE];
	unsigned char m_originator_public[PUBLIC_KEY_SIZE];
}keys_t;

typedef const unsigned char *(*get_private_key_from_settings)(unsigned int *size);

// This class includes DLMS communication Settings.
class CGXDLMSSettings
{
	keys_t m_keys;
    //Is connection made for the server.
    bool m_Connected;

    // Is custom challenges used. If custom challenge is used new challenge is
    // not generated if it is Set. This is for debugging purposes.
    bool m_CustomChallenges;

    // Client to server challenge.
    CGXByteBuffer m_CtoSChallenge;

    // Server to Client challenge.
    CGXByteBuffer m_StoCChallenge;

    CGXByteBuffer m_SourceSystemTitle;

    // Invoke ID.
    unsigned char m_InvokeID;
    //Long Invoke ID.
    int m_LongInvokeID;

    // Priority.
    DLMS_PRIORITY m_Priority;

    // Service class.
    DLMS_SERVICE_CLASS m_ServiceClass;

    // Client address.
    unsigned long m_ClientAddress;

    // Server address.
    unsigned long m_ServerAddress;

    // Is Logical Name referencing used.
    bool m_UseLogicalNameReferencing;

    // Interface type.
    DLMS_INTERFACE_TYPE m_InterfaceType;

    // User authentication.
    DLMS_AUTHENTICATION m_Authentication;

    // User password.
    CGXByteBuffer m_Password;

    /**
     * Key Encrypting Key, also known as Master key.
     */
    CGXByteBuffer m_Kek;

    /**
     * Long data count.
     */
    unsigned short m_Count;

    /**
     * Long data index.
     */
    unsigned short m_Index;

    // DLMS version number.
    unsigned char m_DlmsVersionNumber;

    // Maximum receivers PDU size.
    unsigned short m_MaxReceivePDUSize;

    // Maximum server PDU size.
    unsigned short m_MaxServerPDUSize;

    // HDLC sender frame sequence number.
    unsigned char m_SenderFrame;

    // HDLC receiver block sequence number.
    unsigned char m_ReceiverFrame;

    // Is this server or client.
    bool m_Server;

    // Information from the connection size that server can handle.
    CGXDLMSLimits m_Limits;

    // Block packet index.
    unsigned long m_BlockIndex;

    // List of server or client objects.
    CGXDLMSObjectCollection m_Objects;

    /**
     * Cipher interface that is used to cipher PDU.
     */
    CGXCipher* m_Cipher;

    /**
    * Proposed conformance block. Client asks this funtionality.
    */
    DLMS_CONFORMANCE m_ProposedConformance;

    /**
    * Server tells what functionality is available and client will know it.
    */
    DLMS_CONFORMANCE m_NegotiatedConformance;

    /**
     * Block number acknowledged in GBT.
     */
    unsigned short m_BlockNumberAck;
     /**
     * GBT window size.
     */
    unsigned char m_WindowSize;


public:
    // Constructor.
    CGXDLMSSettings(bool isServer);

    //Destructor.
    ~CGXDLMSSettings();

    // Client to Server challenge.
    CGXByteBuffer& GetCtoSChallenge();

    // Client to Server challenge.
    void SetCtoSChallenge(CGXByteBuffer& value);

    // Get server to Client challenge.
    CGXByteBuffer& GetStoCChallenge();

    // Set server to Client challenge.
    void SetStoCChallenge(CGXByteBuffer& value);

    // Gets used authentication.
    DLMS_AUTHENTICATION GetAuthentication();

    //Sets Used authentication.
    void SetAuthentication(DLMS_AUTHENTICATION value);

    //Gets password.
    CGXByteBuffer& GetPassword();

    // Sets password.
    void SetPassword(CGXByteBuffer& value);

    // Used DLMS version number.
    unsigned char GetDlmsVersionNumber();

    // Used DLMS version number.
    void SetDlmsVersionNumber(unsigned char value);

    // Reset frame sequence.
    void ResetFrameSequence();

    bool CheckFrame(unsigned char frame);

    // Generates I-frame.
    unsigned char GetNextSend(unsigned char first);

    // Generates Receiver Ready S-frame.
    unsigned char GetReceiverReady();

    // Generates Keep Alive S-frame.
    unsigned char GetKeepAlive();

    // Gets current block index.
    unsigned long GetBlockIndex();

    // Sets current block index.
    void SetBlockIndex(unsigned long value);

    // Resets block index to default value.
    void ResetBlockIndex();

    // Increases block index.
    void IncreaseBlockIndex();

    //Is acting as server or client.
    bool IsServer();

    // Information from the frame size that server can handle.
    CGXDLMSLimits& GetLimits();

    // Used interface.
    DLMS_INTERFACE_TYPE GetInterfaceType();

    // Used interface.
    void SetInterfaceType(DLMS_INTERFACE_TYPE value);

    // Gets client address.
    unsigned long GetClientAddress();

    // Sets client address.
    void SetClientAddress(unsigned long value);

    // Server address.
    unsigned long GetServerAddress();

    // Server address.
    void SetServerAddress(unsigned long value);

    // DLMS version number.
    unsigned char GetDLMSVersion();

    // DLMS version number.
    void SetDLMSVersion(unsigned char value);

    // Maximum PDU size.
    unsigned short GetMaxPduSize();

    // Maximum PDU size.
    int SetMaxReceivePDUSize(unsigned short value);

    // Maximum server PDU size.
    unsigned short GetMaxServerPDUSize();

    // Maximum server PDU size.
    int SetMaxServerPDUSize(unsigned short value);

    // Is Logical Name Referencing used.
    bool GetUseLogicalNameReferencing();

    // Is Logical Name Referencing used.
    void SetUseLogicalNameReferencing(bool value);

    // Used priority.
    DLMS_PRIORITY GetPriority();

    // Used priority.
    void SetPriority(DLMS_PRIORITY value);

    // Used service class.
    DLMS_SERVICE_CLASS GetServiceClass();

    // Used service class.
    void SetServiceClass(DLMS_SERVICE_CLASS value);

    // Invoke ID.
    int GetInvokeID();

    // Invoke ID.
    void SetInvokeID(int value);

    /**
       * @return Invoke ID.
       */
    unsigned long GetLongInvokeID();

    /**
     * @param value
     *            Invoke ID.
     */
    int SetLongInvokeID(unsigned long value);

    // Collection of the objects.
    CGXDLMSObjectCollection& GetObjects();

    // Get Is custom challenges used.
    bool IsCustomChallenges();

    // Set is custom challenges used.
    void SetUseCustomChallenge(bool value);

    //Get is connection made for the server.
    bool IsConnected();

    //Set is connection made for the server.
    void SetConnected(bool value);

    /**
    * Cipher interface that is used to cipher PDU.
    */
    CGXCipher* GetCipher();

    /**
    * Cipher interface that is used to cipher PDU.
    */
    void SetCipher(CGXCipher* value);

    /**
     * @return Source system title.
     */
    CGXByteBuffer& GetSourceSystemTitle();

    /**
     * @param value
     *            Source system title.
     */
    int SetSourceSystemTitle(CGXByteBuffer& value);

    /**
     * @return Key Encrypting Key, also known as Master key.
     */
    CGXByteBuffer& GetKek();

    /**
     * @param value
     *            Key Encrypting Key, also known as Master key.
     */
    void SetKek(CGXByteBuffer& value);

    /**
     * @return Long data count.
     */
    unsigned short GetCount();

    /**
     * @param count
     *            Long data count.
     */
    void SetCount(unsigned short value);

    /**
     * @return Long data index.
     */
    unsigned short GetIndex();

    /**
     * @param index
     *            Long data index
     */
    void SetIndex(unsigned short value);

    /**
    * Server will tell what functionality is available. Client will know what functionality server offers.
    * @return Available functionality.
    */
    DLMS_CONFORMANCE GetNegotiatedConformance();

    /**
    * Server will tell what functionality is available. Client will know what functionality server offers from this value.
    *
    * @param value
    *            Available functionality.
    */
    void SetNegotiatedConformance(DLMS_CONFORMANCE value);

    /**
    * Client proposes this functionality. Server checks what it can offer from Conformance and updates it to proposed conformance.
    * @return Functionality.
    */
    DLMS_CONFORMANCE GetProposedConformance();

    /**
    * Client proposes this functionality. Server checks what it can offer from Conformance and updates it to proposed conformance.
    *
    * @param value
    *            Functionality.
    */
    void SetProposedConformance(DLMS_CONFORMANCE value);

	void SetPrivateKey(unsigned char *d);
	void SetOriginatorPublicKey(unsigned char *q);
	void SetRecipientPublicKey(unsigned char *q);
	void SetCB(get_private_key_from_settings cb)
	{
		get_key_cb = cb;
	}

	keys_t& GetKey() {return m_keys;}

    /**
     * @return Block number acknowledged in GBT.
     */
    unsigned short GetBlockNumberAck();

     /**
     * @param value
     *            Block number acknowledged in GBT.
     */
    void SetBlockNumberAck(unsigned short value);

    /**
     * @return GBT window size.
     */
    unsigned char GetWindowSize() {
        return m_WindowSize;
    }

    /**
     * @param value
     *            GBT window size.
     */
    void SetWindowSize(unsigned char value) {
        m_WindowSize = value;
    }

	public:
	get_private_key_from_settings get_key_cb;
};

#endif //GXDLMSSETTINGS_H
