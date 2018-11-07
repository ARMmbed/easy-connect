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

#pragma once

#ifdef __linux__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <unistd.h>
#include <errno.h>

#include <semaphore.h>
#else // MBED
#include "pal.h"
#include "pal_network.h"
#include "pal_rtos.h"
#endif

#include "comp_defines.h"
#include "enums.h"
#include "GXDLMSSecureServer.h"

#define INVALID_HANDLE_VALUE -1
#define SN_SERVER_PORT_STR		"4060"
#define LN_SERVER_PORT_STR	"4061"
#define SN47_SERVER_PORT_STR	"4062"
#define LN47_SERVER_PORT_STR	"4063"

#define TEMPERATURE_OBJECT	"0.0.96.9.0.255"
#define POWER_OBJECT		"0.0.96.9.1.255"
#define HUMIDITY_OBJECT		"0.0.96.9.2.255"
#define CURRENT_OBJECT		"0.0.96.9.3.255"

#define ACTIVE_ENERGY		"1.1.0.7.0.255"
#define REACTIVE_ENERGY		"1.1.0.7.1.255"
#define SUM_LI_ACTIVE_POWER	"1.1.1.8.0.255"
#define MANUFACTURER_SPECIFIC	"0.0.98.1.1.254"

#define WAIT_TIME 5000
#define RECEIVE_BUFFER_SIZE 200

class CGXDLMSBaseAL : public CGXDLMSSecureServer
{
protected:
	SOCKET     m_ServerSocket;
	THREAD_ID   m_ReceiverThread;
	THREAD_ID   m_TempratureThread;


	DLMS_SERVICE_TYPE m_protocolType;
	 
public:
	SEMAPHORE m_wait_server_start;
	
	bool m_print;
	char *m_drop_receive;
	char *m_drop_send;
	int m_drop_receive_size;
	int m_drop_send_size;
	int m_receive_counter;
	int m_send_counter;
	
 //   GX_TRACE_LEVEL m_Trace;

    /////////////////////////////////////////////////////////////////////////
    //Constructor.
    /////////////////////////////////////////////////////////////////////////
    CGXDLMSBaseAL(
        bool UseLogicalNameReferencing = true,
        DLMS_INTERFACE_TYPE IntefaceType = DLMS_INTERFACE_TYPE_HDLC,
		DLMS_SERVICE_TYPE ProtocolType = DLMS_SERVICE_TYPE_UDP) :
        CGXDLMSSecureServer(UseLogicalNameReferencing, IntefaceType)
    {
        m_ServerSocket = (SOCKET)0;
        m_ReceiverThread = -1;
        m_protocolType = ProtocolType;
#if defined(__linux__) && defined(CLI_MODE)
        sem_init(&m_wait_server_start, 0, 0);
#elif defined(__MBED__)
        pal_osSemaphoreCreate(1, &m_wait_server_start);
#endif

        _ready = false;
		m_print = false;
		m_drop_receive = NULL;
		m_drop_receive_size = 0;
		m_drop_send = NULL;
		m_drop_send_size = 0;
		m_receive_counter = 0;
		m_send_counter = 0;
    }


    /////////////////////////////////////////////////////////////////////////
    //Destructor.
    /////////////////////////////////////////////////////////////////////////
    virtual ~CGXDLMSBaseAL(void)
    {
    	delete[] m_drop_receive;
    	delete[] m_drop_send;
        StopServer();
    }

    virtual bool IsConnected()
    {
		return m_ServerSocket != (SOCKET)0;
    }

    virtual STATUS CreateSocket() = 0;

    virtual STATUS Listen(int backlog) = 0;

	virtual int Read(SOCKET client_socket, CGXByteBuffer &bb, size_t *recievedDataSize) = 0;

	virtual int Write(SOCKET client_socket, CGXByteBuffer &bb, size_t *sentDataSize) = 0;

    virtual STATUS Accept(SOCKET *client_sock, SOCKADDR *client_sock_addr, SOCKLEN *client_sock_addr_len) = 0;

    virtual STATUS CloseSocket(SOCKET socket) = 0;

    virtual int ConnectPort(char *ip_address, const char* pPort) = 0;

    int StartServer(char *address,const char* pPort);

    int StopServer();
    int KillThread();

    int Init(const char* pPort);
    int CreateObjects();

    int Connect(char* ip_address,int port);

    CGXDLMSObject* FindObject(
        DLMS_OBJECT_TYPE objectType,
        int sn,
        std::string& ln);

    void PreRead(
        std::vector<CGXDLMSValueEventArg*>& args);

    void PreWrite(
        std::vector<CGXDLMSValueEventArg*>& args);

    void PreAction(
        std::vector<CGXDLMSValueEventArg*>& args);

    void PostRead(
        std::vector<CGXDLMSValueEventArg*>& args);

    void PostWrite(
        std::vector<CGXDLMSValueEventArg*>& args);

    void PostAction(
        std::vector<CGXDLMSValueEventArg*>& args);

    bool IsTarget(
        unsigned long int serverAddress,
        unsigned long clientAddress);

    DLMS_SOURCE_DIAGNOSTIC ValidateAuthentication(
        DLMS_AUTHENTICATION authentication,
        CGXByteBuffer& password);

    /**
    * Get attribute access mode.
    *
    * @param arg
    *            Value event argument.
    * @return Access mode.
    * @throws Exception
    *             Server handler occurred exceptions.
    */
    DLMS_ACCESS_MODE GetAttributeAccess(CGXDLMSValueEventArg* arg);

    /**
    * Get method access mode.
    *
    * @param arg
    *            Value event argument.
    * @return Method access mode.
    * @throws Exception
    *             Server handler occurred exceptions.
    */
    DLMS_METHOD_ACCESS_MODE GetMethodAccess(CGXDLMSValueEventArg* arg);

    /**
    * Accepted connection is made for the server. All initialization is done
    * here.
    */
    void Connected(
        CGXDLMSConnectionEventArgs& connectionInfo);

    /**
     * Client has try to made invalid connection. Password is incorrect.
     *
     * @param connectionInfo
     *            Connection information.
     */
    void InvalidConnection(
        CGXDLMSConnectionEventArgs& connectionInfo);
    /**
     * Server has close the connection. All clean up is made here.
     */
    void Disconnected(
        CGXDLMSConnectionEventArgs& connectionInfo);

    /**
    * Get selected value(s). This is called when example profile generic
    * request current value.
    *
    * @param type
    *            Update type.
    * @param args
    *            Value event arguments.
    */
    void PreGet(
        std::vector<CGXDLMSValueEventArg*>& args);

    /**
    * Get selected value(s). This is called when example profile generic
    * request current value.
    *
    * @param type
    *            Update type.
    * @param args
    *            Value event arguments.
    */
    void PostGet(
        std::vector<CGXDLMSValueEventArg*>& args);

    void SetState(bool ifReady) {_ready = ifReady;}
    bool GetState() {return _ready;}

public:
    bool _ready;
};

class CGXDLMSUdp : public CGXDLMSBaseAL
{
	SOCKADDR m_from;
	SOCKLEN m_fromLength;

public:

	CGXDLMSUdp(bool UseLogicalNameReferencing = true, DLMS_INTERFACE_TYPE IntefaceType = DLMS_INTERFACE_TYPE_HDLC) :
								CGXDLMSBaseAL(UseLogicalNameReferencing, IntefaceType, DLMS_SERVICE_TYPE_UDP), m_fromLength(sizeof(SOCKADDR))
	{}

	virtual ~CGXDLMSUdp(){};

	STATUS CreateSocket();

	STATUS Listen(int backlog);

	int Read(SOCKET dummy, CGXByteBuffer &bb, size_t *recievedDataSize);

	int Write(SOCKET dummy, CGXByteBuffer &bb, size_t *sentDataSize);

	STATUS Accept(SOCKET *dummy0, SOCKADDR *dummy1, SOCKLEN *dummy2);
	 
	int ConnectPort(char *ip_address, const char* pPort);

	STATUS CloseSocket(SOCKET dummy);
};

class CGXDLMSTcp : public CGXDLMSBaseAL
{
public:
	CGXDLMSTcp(bool UseLogicalNameReferencing = true, DLMS_INTERFACE_TYPE IntefaceType = DLMS_INTERFACE_TYPE_HDLC) :
								CGXDLMSBaseAL(UseLogicalNameReferencing, IntefaceType, DLMS_SERVICE_TYPE_TCP)
	{}

	virtual ~CGXDLMSTcp(){};

	STATUS CreateSocket();
 
	STATUS Listen(int backlog);

	int Read(SOCKET client_socket, CGXByteBuffer &bb, size_t *recievedDataSize);

	int Write(SOCKET client_socket, CGXByteBuffer &bb, size_t *sentDataSize);

	STATUS Accept(SOCKET *client_sock, SOCKADDR *client_sock_addr, SOCKLEN *client_sock_addr_len);

	int ConnectPort(char *ip_address,const char* pPort);

	STATUS CloseSocket(SOCKET socket);
};
