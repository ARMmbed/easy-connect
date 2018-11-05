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

#ifdef __MBED__
#include <AnalogIn.h>
#include "mbed.h"
#include "../../mbed-os/rtos/Thread.h"
#else //Linux includes
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h> //Add support for sockets
#include <pthread.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif


#define MAX_MEMORY 1
#define LISTENER_THREAD_STACK_SIZE 2048
#define SIMULATION_THREAD_STACK_SIZE 512

#ifdef __MBED__

DigitalOut led1(LED1);
DigitalOut led2(PD_9);
DigitalOut led3(PD_8);
DigitalOut led4(PD_11);
DigitalOut led5(PD_12);

static int prev_led;
using namespace rtos;
Thread listener(osPriorityHigh, sizeof(uint32_t) * LISTENER_THREAD_STACK_SIZE);
Thread sensorThread(osPriorityHigh, sizeof(uint32_t) * SIMULATION_THREAD_STACK_SIZE);
#endif


#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <netdb.h>


#ifdef __MBED__
#include <AnalogIn.h>
#include "../mbed-os/drivers/RawSerial.h"
#endif

#include "GXDLMSBaseAL.h"
#include "GXTime.h"
#include "GXDate.h"
#include "GXDLMSClient.h"
#include "GXDLMSData.h"
#include "GXDLMSRegister.h"
#include "GXDLMSExtendedRegister.h"
#include "GXDLMSClock.h"
#include "GXDLMSTcpUdpSetup.h"
#include "GXDLMSProfileGeneric.h"
#include "GXDLMSAutoConnect.h"
#include "GXDLMSIECOpticalPortSetup.h"
#include "GXDLMSActivityCalendar.h"
#include "GXDLMSDemandRegister.h"
#include "GXDLMSRegisterMonitor.h"
#include "GXDLMSActionSchedule.h"
#include "GXDLMSSapAssignment.h"
#include "GXDLMSAutoAnswer.h"
#include "GXDLMSModemConfiguration.h"
#include "GXDLMSMacAddressSetup.h"
#include "GXDLMSModemInitialisation.h"
#include "GXDLMSActionSet.h"
#include "GXDLMSIp4Setup.h"
#include "GXDLMSPushSetup.h"
#include "GXDLMSAssociationLogicalName.h"
#include "GXDLMSAssociationShortName.h"

#define MAX_PACKET_PRINT 170

using namespace std;
static const char* DATAFILE = "data.csv";



#ifdef __MBED__

/*
 * CDC Interface 1 - UART 1 (UART1_TX = PTC4 [J2-4], UART1_RX = PTC3 [J1-16])
CDC Interface 2 - UART 2 (UART2_TX = PTD3 [J2-10], UART2_RX = PTD2 [J2-8])
CDC Interface 3 - UART 3 (UART3_TX = PTB11 [J4-8], UART3_RX = PTB10 [J4-6])
CDC Interface 4 - UART 4 (UART4_TX = PTC15 [J199-4], UART4_RX = PTC14 [J199-3] - Bluetooth connector)
 * */
#define uartRx USBRX	//PTC3
#define uartTx USBTX	//PTC4

#endif

static void PrintfBuff(unsigned char *ptr, int size)
{
	printf("#########################\n");
	for(int i = 0 ; i < size ; ++i)
	{
		printf("%02x ", *ptr++);
		if((i+1)% 8 == 0) printf("\n");
	}
	if(size% 8 != 0) printf("\n");
	printf("#########################\n\n");
}

static void PrintfBuff(CGXByteBuffer *bb)
{
	PrintfBuff(bb->GetData(), bb->GetSize() > MAX_PACKET_PRINT ? MAX_PACKET_PRINT : bb->GetSize());
}

static bool DropRec(CGXDLMSBaseAL *server)
{
	bool ret = false;

	if(server->m_drop_receive != NULL)
	{
		char *drop_receive = server->m_drop_receive;
		// 'counter' represents the packet number. first packet is index '0'
		int counter = server->m_receive_counter;
		int arr_i = counter / 8;

		if(arr_i < server->m_drop_receive_size)
		{
			ret = (drop_receive[arr_i] >> (counter % 8)) & 0x1;
		}
	}

	++server->m_receive_counter;

	return ret;
}

static bool DropSend(CGXDLMSBaseAL *server)
{
	bool ret = false;

	if(server->m_drop_send != NULL)
	{
		char *drop_send = server->m_drop_send;
		// 'counter' represents the packet number. first packet is index '0'
		int counter = server->m_send_counter;
		int arr_i = counter / 8;

		if(arr_i < server->m_drop_send_size)
		{
			ret = (drop_send[arr_i] >> (counter % 8)) & 0x1;
		}
	}

	++server->m_send_counter;

	return ret;
}

static void ListenerThread(const void* pVoid)
{
    CGXByteBuffer reply;
	STATUS result = SUCCESS;
    CGXDLMSBaseAL* server = (CGXDLMSBaseAL*)pVoid;
    int ret;
    CGXByteBuffer bb;
    bb.Capacity(2048);
	SOCKET client_sock = { 0 };
	SOCKADDR client_sock_addr = { 0 };
	SOCKLEN client_sock_addr_len;
	bool first_packet = true;

    while (server->IsConnected())
    {
    	result = server->Accept(&client_sock, &client_sock_addr, &client_sock_addr_len);

    	if (result != SUCCESS)
		{
    		printf("failed to accept socket result = %d\r\n", (int)result);
			continue;
		}
    	printf("DLMS server connected\n");

#ifdef CLI_MODE
#ifdef __linux__
		sem_post(&(server->m_wait_server_start));
#else // __MBED__
		pal_osSemaphoreRelease(server->m_wait_server_start);
#endif
#endif // CLI_MODE

		while (server->IsConnected())
		{
			size_t len;
			//If client is left wait for next client.
			ret = server->Read(client_sock, bb, &len);

			if(ret == SUCCESS && first_packet)
			{
				printf("First Packet Received\n\n");
				first_packet = false;
			}

			if(DropRec(server) == true)
			{
				printf("drop received packet %d\n", server->m_receive_counter);

				if(server->m_print)
				{
					PrintfBuff(bb.GetData(), len);
				}
			}

			else
			{
				if(server->m_print)
				{
					printf("packet received\n");
					PrintfBuff(bb.GetData(), len);
				}

				if ( ret == 0 && len == 0 )
				{
					server->SetState(false);
					//Notify error.
					server->Reset();
					server->CloseSocket(client_sock); client_sock = (SOCKET)-1;
					break;
				}

				//If client is closed the connection.
				if (ret != SUCCESS)
				{
					server->SetState(false);
					server->Reset();
					server->CloseSocket(client_sock); client_sock = (SOCKET)-1;
					printf("PAL_ERR_SOCKET_CONNECTION_CLOSED, close socket\n");
					break;
				}

				bb.SetSize(bb.GetSize() + len);

				if (server->HandleRequest(bb, reply) != 0)
				{
					printf("\n\nServer: Error!!!\n\n");
					PrintfBuff(reply.GetData(), reply.GetSize() - reply.GetPosition());
					server->SetState(false);
					server->CloseSocket(client_sock); client_sock = (SOCKET)-1;
				}
				bb.SetSize(0);

				if (reply.GetSize() != 0)
				{
					if(DropSend(server) == true)
					{
						printf("drop sent packet %d\n", server->m_send_counter);

						if(server->m_print)
						{
							PrintfBuff(reply.GetData(), reply.GetSize() - reply.GetPosition());
						}
					}

					else
					{
						if(server->m_print)
						{
							printf("packet sent\n");
							PrintfBuff(reply.GetData(), reply.GetSize() - reply.GetPosition());
						}

						ret = server->Write(client_sock, reply, &len);

						if (ret == -1)
						{
							//If error has occured
							server->Reset();
							server->CloseSocket(client_sock); client_sock = (SOCKET)-1;
						}
					}

					server->SetState(true);
					reply.Clear();
				}
	#ifdef __linux__
				sleep(1);
	#else
				osThreadYield();
				pal_osDelay(5000);
	#endif
			}

		}
		server->SetState(false);
		server->Reset();

#ifdef __MBED__
		osThreadYield();
#endif
    }

}

#ifdef __linux__
static void * UnixListenerThread(void * pVoid)
{
	ListenerThread(pVoid);
	return NULL;
}
#endif

static float random_between_two_int(float min, float max)
{
	return (min + 1) + (((float)rand()) / (float)RAND_MAX) * (max - (min + 1));
}


#ifdef __MBED__
static void sensor_thread(void const *pVoid)
{
	printf("start sensor thread \r\n");
	CGXDLMSBaseAL* pDLMSBase=(CGXDLMSBaseAL*)pVoid;
	CGXDLMSObjectCollection& items = pDLMSBase->GetItems();
	CGXDLMSObject* obj;
	string str_id;
//	CGXDLMSVariant hum = 0.0;
	CGXDLMSVariant curr = 0.0;
	CGXDLMSVariant power = 0.0;

	CGXDLMSVariant active((int)0);
	CGXDLMSVariant reactive((unsigned long)0);
	CGXDLMSVariant sum_li = CGXDLMSVariant((unsigned long)0);
	CGXDLMSVariant custom((bool)0);

	CGXDLMSVariant new_value;

	while(1)
	{
		printf("*********************************************************************\r\n");
#if 0
		//don't change value of the HUMIDITY_OBJ
		// it should stay constant for system tests
		/* HUMIDITY_OBJECT  */
		str_id= HUMIDITY_OBJECT;
		obj= items.FindByLN(DLMS_OBJECT_TYPE_DATA, str_id);
		if(obj!=NULL)
		{
			hum = ((CGXDLMSData*)obj)->GetValue();
			new_value = (hum.fltVal + 1 > 225) ? 225 :hum.fltVal + 1;
			((CGXDLMSData*)obj)->SetValue(new_value);
			printf("Humidity: prev value = %f   new value = %f\n", hum.fltVal, new_value.fltVal);

		}
#endif
		/* CURRENT_OBJECT  */
		str_id = CURRENT_OBJECT;
		obj = items.FindByLN(DLMS_OBJECT_TYPE_DATA, str_id);
		if (obj != NULL)
		{
			curr = ((CGXDLMSData*)obj)->GetValue();
			new_value = (curr.fltVal + 1 > 30) ? 30 :curr.fltVal + 1;
			((CGXDLMSData*)obj)->SetValue(new_value);
			printf("current: prev value = %f   new value = %f\n", curr.fltVal, new_value.fltVal);
		}
		/* POWER_OBJECT  */
		str_id = POWER_OBJECT;
		obj = items.FindByLN(DLMS_OBJECT_TYPE_DATA, str_id);
		if (obj != NULL)
		{
			power = ((CGXDLMSData*)obj)->GetValue();
			new_value = (power.fltVal + 1 > 100) ? 100 : power.fltVal + 1;
			((CGXDLMSData*)obj)->SetValue(new_value);
			printf("power:   prev value = %f   new value = %f\n", power.fltVal, new_value.fltVal);
		}

		/* ACTIVE_ENERGY  */
		str_id = ACTIVE_ENERGY;
		obj = items.FindByLN(DLMS_OBJECT_TYPE_DATA, str_id);
		if (obj != NULL)
		{
			active = ((CGXDLMSData*)obj)->GetValue();
			if(active.lVal + 2 < 10)
				new_value = CGXDLMSVariant((int)active.lVal + 2);
			else
				new_value = CGXDLMSVariant((int)1);

			((CGXDLMSData*)obj)->SetValue(new_value);
			printf("Active energy: prev value = %ld   new value = %ld\n", active.lVal, new_value.lVal);
		}

		/* REACTIVE_ENERGY  */
		str_id = REACTIVE_ENERGY;
		obj = items.FindByLN(DLMS_OBJECT_TYPE_REGISTER, str_id);
		if (obj != NULL)
		{
			reactive = ((CGXDLMSRegister*)obj)->GetValue();
			if(reactive.ulVal + 1 < 10)
				new_value = CGXDLMSVariant((unsigned long)reactive.ulVal + 1);
			else
				new_value = CGXDLMSVariant((unsigned long)2);
			((CGXDLMSData*)obj)->SetValue(new_value);
			printf("Reactive energy: prev value = %lu   new value = %lu\n", reactive.ulVal, new_value.ulVal);
		}

		/* SUM_LI_ACTIVE_POWER  */
		str_id = SUM_LI_ACTIVE_POWER;
		obj = items.FindByLN(DLMS_OBJECT_TYPE_EXTENDED_REGISTER, str_id);
		if (obj != NULL)
		{
			sum_li = ((CGXDLMSRegister*)obj)->GetValue();
			if(sum_li.ulVal + 3 < 15)
				new_value = CGXDLMSVariant((unsigned long)sum_li.ulVal + 3);
			else
				new_value = CGXDLMSVariant((unsigned long)1);

			((CGXDLMSData*)obj)->SetValue(new_value);
			printf("Sum Li Active Power: prev value = %ld   new value = %ld\n", (long)sum_li.ulVal, (long)new_value.ulVal);
		}


		/* MANUFACTURER_SPECIFIC  */
		str_id = MANUFACTURER_SPECIFIC;
		obj = items.FindByLN(DLMS_OBJECT_TYPE_DATA, str_id);
		if (obj != NULL)
		{
			custom = ((CGXDLMSRegister*)obj)->GetValue();
			/*if(custom.lVal != prev_led) {*/
				prev_led = custom.boolVal;

				led1 = !led1;
				led2 = !led2;
				led3 = (int)custom.boolVal;
				led4 = !led4;
				led5 = !led5;
				printf("Custom: value = %d\n", custom.boolVal);
				printf("Leds: led1 = %d led2 = %d led3 = %d\n", (int)led1, (int)led2, (int)led3);
			//}
		}

		pal_osDelay(10000);
	}

}
#else
static void *temperature_thread(void *pVoid)

{
	CGXDLMSVariant active((int)0);
	CGXDLMSVariant reactive((unsigned long)0);
	CGXDLMSVariant sum_li = CGXDLMSVariant((unsigned long)0);
	CGXDLMSVariant custom((bool)0);

	CGXDLMSVariant new_value;

	printf("start temprature thread \r\n");
	CGXDLMSBaseAL* pDLMSBase=(CGXDLMSBaseAL*)pVoid;
	while(1)
	{

		string str_id= TEMPERATURE_OBJECT;
		CGXDLMSObject* obj=pDLMSBase->GetItems().FindByLN(DLMS_OBJECT_TYPE_REGISTER,str_id);
		if(obj!=NULL)
		{
			//temp_value=31;
			CGXDLMSVariant temp_value=((CGXDLMSRegister*)obj)->GetValue();
			if(temp_value.iVal == 40)
				temp_value=30;
			temp_value=temp_value.iVal+1;
			printf("New temp value %d\r\n",temp_value.iVal);
			((CGXDLMSRegister*)obj)->SetValue(temp_value);
		}
		/* ACTIVE_ENERGY  */
		str_id = ACTIVE_ENERGY;
		obj = pDLMSBase->GetItems().FindByLN(DLMS_OBJECT_TYPE_DATA, str_id);
		if (obj != NULL)
		{
			active = ((CGXDLMSData*)obj)->GetValue();
			if(active.lVal + 2 < 10)
				new_value = CGXDLMSVariant((int)active.lVal + 2);
			else
				new_value = CGXDLMSVariant((int)1);

			((CGXDLMSData*)obj)->SetValue(new_value);
			printf("Active energy: prev value = %ld   new value = %ld\n", active.lVal, new_value.lVal);
		}

		/* REACTIVE_ENERGY  */
		str_id = REACTIVE_ENERGY;
		obj = pDLMSBase->GetItems().FindByLN(DLMS_OBJECT_TYPE_REGISTER, str_id);
		if (obj != NULL)
		{
			reactive = ((CGXDLMSRegister*)obj)->GetValue();
			if(reactive.ulVal + 1 < 10)
				new_value = CGXDLMSVariant((unsigned long)reactive.ulVal + 1);
			else
				new_value = CGXDLMSVariant((unsigned long)2);
			((CGXDLMSData*)obj)->SetValue(new_value);
			printf("Reactive energy: prev value = %lu   new value = %lu\n", reactive.ulVal, new_value.ulVal);
		}

		/* SUM_LI_ACTIVE_POWER  */
		str_id = SUM_LI_ACTIVE_POWER;
		obj = pDLMSBase->GetItems().FindByLN(DLMS_OBJECT_TYPE_EXTENDED_REGISTER, str_id);
		if (obj != NULL)
		{
			sum_li = ((CGXDLMSRegister*)obj)->GetValue();
			if(sum_li.ulVal + 3 < 15)
				new_value = CGXDLMSVariant((unsigned long)sum_li.ulVal + 3);
			else
				new_value = CGXDLMSVariant((unsigned long)1);

			((CGXDLMSData*)obj)->SetValue(new_value);
			printf("Sum Li Active Power: prev value = %ld   new value = %ld\n", (long)sum_li.ulVal, (long)new_value.ulVal);
		}


		/* MANUFACTURER_SPECIFIC  */
		str_id = MANUFACTURER_SPECIFIC;
		obj = pDLMSBase->GetItems().FindByLN(DLMS_OBJECT_TYPE_DATA, str_id);
		if (obj != NULL)
		{
			custom = ((CGXDLMSRegister*)obj)->GetValue();
			printf("Custom: value = %d\n", custom.boolVal);
		}
		sleep(1);
	}

}
#endif

int CGXDLMSBaseAL::Connect(char *ip_address, int port)
{
	NETADD_INFO interfaceInfo = { 0 };

	if (CreateSocket() != SUCCESS)
    {
		printf("failed to create socket\n");
        assert(0);
        return -1;
    }

    if (!IsConnected()) return -1;

    int fFlag = 1;

#ifdef __linux__
	if (setsockopt(m_ServerSocket, SOL_SOCKET, SO_REUSEADDR, &fFlag, sizeof(fFlag)) == -1) return -1;

	interfaceInfo.sin_port = htons(port);
	if(NULL == ip_address) {
		interfaceInfo.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else {
		//inet_aton("127.0.0.1", &interfaceInfo.sin_addr.s_addr);
		interfaceInfo.sin_addr.s_addr = inet_addr(ip_address);
	}
	interfaceInfo.sin_family = AF_INET;

	printf("port = %d\n", port);

	if ((::bind(m_ServerSocket, (sockaddr*)&interfaceInfo, sizeof(interfaceInfo))) == -1) return -1;
#else // MBED
	if (pal_setSocketOptions(m_ServerSocket, PAL_SO_REUSEADDR, (char *)&fFlag, sizeof(fFlag)) == -1) return -1;

	if (pal_getNetInterfaceInfo(0, &interfaceInfo) != PAL_SUCCESS)return -1;

    printf("interface addr: %u.%u.%u.%u \r\n",
    (unsigned char)interfaceInfo.address.addressData[2],
    (unsigned char)interfaceInfo.address.addressData[3],
    (unsigned char)interfaceInfo.address.addressData[4],
    (unsigned char)interfaceInfo.address.addressData[5]);

	if (pal_setSockAddrPort(&(interfaceInfo.address), port) != PAL_SUCCESS) return -1;

    if (pal_bind(m_ServerSocket, &(interfaceInfo.address), interfaceInfo.addressSize) == -1) return -1;
#endif

    if (Listen(1) == -1) return -1;

    return 0;
}

int CGXDLMSBaseAL::StartServer(char *ip_address, const char* pPort)
{
    int ret = 0;

    if ((ret = StopServer()) != 0)
    {
        return ret;
    }

    if ((ret = ConnectPort(ip_address, pPort)) != 0)
    {
    	printf("Failed to open port\n");
        return ret;
    }

//    printf("Port Connected\n");

#ifdef __linux__
	ret = pthread_create(&m_ReceiverThread, NULL, UnixListenerThread, (void *)this);
#ifndef CLI_MODE
	pthread_join(m_ReceiverThread, NULL);
#else
	sem_wait(&m_wait_server_start);
#endif // __linux__

	ret = pthread_create(&m_TempratureThread, NULL, temperature_thread, (void *)this);

#else // MBED
	listener.start(mbed::callback(ListenerThread, (void*)this));
	pal_osSemaphoreWait(m_wait_server_start, 1000, NULL);
	sensorThread.start(mbed::callback(sensor_thread, (void*)this));
#endif

    return ret;
}

int CGXDLMSBaseAL::StopServer()
{
    if (IsConnected())
    {
    	CloseSocket(m_ServerSocket);
    }

    return 0;
}

int CGXDLMSBaseAL::KillThread()
{
#ifdef __linux__
	if(pthread_cancel(m_ReceiverThread) != 0)
	{
		return -1;
	}
#else
	if( listener.terminate() != 0 ||
		sensorThread.terminate() != 0 ||
		pal_osSemaphoreDelete(&m_wait_server_start) != 0)
	{
		return -1;
	}
#endif
	return 0;
}

static int GetIpAddressAL(std::string& address)
{
#ifdef __linux__
    int ret;
    struct hostent *phe;
    char ac[80];
    if ((ret = gethostname(ac, sizeof(ac))) == 0)
    {
        phe = gethostbyname(ac);
        if (phe == 0)
        {
            ret = -1;
        }
        else
        {
            struct in_addr* addr = (struct in_addr*)phe->h_addr_list[0];
            address = inet_ntoa(*addr);
        }
    }
    return ret;
#else
	  palNetInterfaceInfo_t interfaceInfo;
	  memset(&interfaceInfo,0,sizeof(interfaceInfo));
	  pal_getNetInterfaceInfo(0, &interfaceInfo);

	  return 0;
#endif
}

#if MAX_MEMORY

///////////////////////////////////////////////////////////////////////
//Add Logical Device Name. 123456 is meter serial number.
///////////////////////////////////////////////////////////////////////
// COSEM Logical Device Name is defined as an octet-string of 16 octets.
// The first three octets uniquely identify the manufacturer of the device and it corresponds
// to the manufacturer's identification in IEC 62056-21.
// The following 13 octets are assigned by the manufacturer.
//The manufacturer is responsible for guaranteeing the uniqueness of these octets.
static CGXDLMSData* AddLogicalDeviceNameAL(CGXDLMSObjectCollection& items, unsigned long sn)
{
    char buff[17];
    sprintf(buff, "GRX%.13lu", sn);
    CGXDLMSVariant id;
    id.Add((const char*)buff, 16);
    CGXDLMSData* ldn = new CGXDLMSData("0.0.42.0.0.255");
	ldn->SetValue(id);
    items.push_back(ldn);
    return ldn;
}

/*
* Add firmware version.
*/
static void AddFirmwareVersionAL(CGXDLMSObjectCollection& items)
{
    CGXDLMSVariant version;
    version = "Gurux FW 0.0.1";
    CGXDLMSData* fw = new CGXDLMSData("1.0.0.2.0.255");
    fw->SetValue(version);
    items.push_back(fw);
}

/*
* Add Electricity ID 1.
*/
static void AddElectricityID1AL(CGXDLMSObjectCollection& items, unsigned long sn)
{
    char buff[17];
    sprintf(buff, "GRX%.13lu", sn);
    CGXDLMSVariant id;
    id.Add((const char*)buff, 16);
	CGXDLMSData* d = new CGXDLMSData("1.1.0.0.0.255");
	d->SetValue(id);
    d->GetAttributes().push_back(CGXDLMSAttribute(2, DLMS_DATA_TYPE_STRING));
    items.push_back(d);
}

/*
* Add Electricity ID 2.
*/
static void AddElectricityID2AL(CGXDLMSObjectCollection& items, unsigned long sn)
{
    CGXDLMSVariant id2(sn);
	CGXDLMSData* d = new CGXDLMSData("1.1.0.0.1.255");
	d->SetValue(id2);
    d->GetAttributes().push_back(CGXDLMSAttribute(2, DLMS_DATA_TYPE_UINT32));
    items.push_back(d);
}

/*
* Add Auto connect object.
*/
static void AddAutoConnectAL(CGXDLMSObjectCollection& items)
{
    CGXDLMSAutoConnect* pAC = new CGXDLMSAutoConnect();
    pAC->SetMode(AUTO_CONNECT_MODE_AUTO_DIALLING_ALLOWED_ANYTIME);
    pAC->SetRepetitions(10);
    pAC->SetRepetitionDelay(60);
    //Calling is allowed between 1am to 6am.
    pAC->GetCallingWindow().push_back(std::make_pair(CGXTime(1, 0, 0, -1), CGXTime(6, 0, 0, -1)));
    pAC->GetDestinations().push_back("www.gurux.org");
    items.push_back(pAC);
}

/*
* Add Activity Calendar object.
*/
static void AddActivityCalendarAL(CGXDLMSObjectCollection& items)
{
    CGXDLMSActivityCalendar* pActivity = new CGXDLMSActivityCalendar();
    pActivity->SetCalendarNameActive("Active");
    pActivity->GetSeasonProfileActive().push_back(new CGXDLMSSeasonProfile("Summer time", CGXDate(-1, 3, 31), ""));
    pActivity->GetWeekProfileTableActive().push_back(new CGXDLMSWeekProfile("Monday", 1, 1, 1, 1, 1, 1, 1));
    CGXDLMSDayProfile *aDp = new CGXDLMSDayProfile();
    aDp->SetDayId(1);
    CGXDateTime now = CGXDateTime::Now();
    CGXTime time = now;
    aDp->GetDaySchedules().push_back(new CGXDLMSDayProfileAction(time, "test", 1));
    pActivity->GetDayProfileTableActive().push_back(aDp);
    pActivity->SetCalendarNamePassive("Passive");
    pActivity->GetSeasonProfilePassive().push_back(new CGXDLMSSeasonProfile("Winter time", CGXDate(-1, 10, 30), ""));
    pActivity->GetWeekProfileTablePassive().push_back(new CGXDLMSWeekProfile("Tuesday", 1, 1, 1, 1, 1, 1, 1));

    CGXDLMSDayProfile* passive = new CGXDLMSDayProfile();
    passive->SetDayId(1);
    passive->GetDaySchedules().push_back(new CGXDLMSDayProfileAction(time, "0.0.1.0.0.255", 1));
    pActivity->GetDayProfileTablePassive().push_back(passive);
    CGXDateTime dt(CGXDateTime::Now());
    pActivity->SetTime(dt);
    items.push_back(pActivity);
}

/*
* Add Optical Port Setup object.
*/
static void AddOpticalPortSetupAL(CGXDLMSObjectCollection& items)
{
    CGXDLMSIECOpticalPortSetup* pOptical = new CGXDLMSIECOpticalPortSetup();
    pOptical->SetDefaultMode(DLMS_OPTICAL_PROTOCOL_MODE_DEFAULT);
    pOptical->SetProposedBaudrate(DLMS_BAUD_RATE_9600);
    pOptical->SetDefaultBaudrate(DLMS_BAUD_RATE_300);
    pOptical->SetResponseTime(DLMS_LOCAL_PORT_RESPONSE_TIME_200_MS);
    pOptical->SetDeviceAddress("Gurux");
    pOptical->SetPassword1("Gurux1");
    pOptical->SetPassword2("Gurux2");
    pOptical->SetPassword5("Gurux5");
    items.push_back(pOptical);
}

/*
* Add Demand Register object.
*/
static void AddDemandRegisterAL(CGXDLMSObjectCollection& items)
{
    CGXDLMSDemandRegister* pDr = new CGXDLMSDemandRegister("0.0.1.0.0.255");
    pDr->SetCurrentAvarageValue(10);
    pDr->SetLastAvarageValue(20);
    pDr->SetStatus(1);
    pDr->SetStartTimeCurrent(CGXDateTime::Now());
    pDr->SetCaptureTime(CGXDateTime::Now());
    pDr->SetPeriod(10);
    pDr->SetNumberOfPeriods(1);
    items.push_back(pDr);
}

/*
* Add Register Monitor object.
*/
static void AddRegisterMonitorAL(CGXDLMSObjectCollection& items, CGXDLMSRegister* pRegister)
{
    CGXDLMSRegisterMonitor* pRm = new CGXDLMSRegisterMonitor("0.0.1.0.0.255");
    CGXDLMSVariant threshold;
    vector<CGXDLMSVariant> thresholds;
    threshold.Add("Gurux1", 6);
    thresholds.push_back(threshold);
    threshold.Clear();
    threshold.Add("Gurux2", 6);
    thresholds.push_back(threshold);
    pRm->SetThresholds(thresholds);
    CGXDLMSMonitoredValue mv;
    mv.Update(pRegister, 2);
    pRm->SetMonitoredValue(mv);
    CGXDLMSActionSet * action = new CGXDLMSActionSet();
    string ln;
    pRm->GetLogicalName(ln);
    action->GetActionDown().SetLogicalName(ln);
    action->GetActionDown().SetScriptSelector(1);
    pRm->GetLogicalName(ln);
    action->GetActionUp().SetLogicalName(ln);
    action->GetActionUp().SetScriptSelector(1);
    pRm->GetActions().push_back(action);
    items.push_back(pRm);
}

/*
* Add action schedule object.
*/
static void AddActionScheduleAL(CGXDLMSObjectCollection& items)
{
    CGXDLMSActionSchedule* pActionS = new CGXDLMSActionSchedule("0.0.1.0.0.255");
    pActionS->SetExecutedScriptLogicalName("1.2.3.4.5.6");
    pActionS->SetExecutedScriptSelector(1);
    pActionS->SetType(DLMS_SINGLE_ACTION_SCHEDULE_TYPE1);
    pActionS->GetExecutionTime().push_back(CGXDateTime::Now());
    items.push_back(pActionS);
}

/*
* Add SAP Assignment object.
*/
static void AddSapAssignmentAL(CGXDLMSObjectCollection& items)
{
    CGXDLMSSapAssignment* pSap = new CGXDLMSSapAssignment();
    std::map<int, basic_string<char> > list;
    list[1] = "Gurux";
    list[16] = "Gurux-2";
    pSap->SetSapAssignmentList(list);
    items.push_back(pSap);
}

/**
* Add Auto Answer object.
*/
static void AddAutoAnswerAL(CGXDLMSObjectCollection& items)
{
    CGXDLMSAutoAnswer* pAa = new CGXDLMSAutoAnswer();
    pAa->SetMode(AUTO_CONNECT_MODE_EMAIL_SENDING);
    pAa->GetListeningWindow().push_back(std::pair<CGXDateTime, CGXDateTime>(CGXDateTime(-1, -1, -1, 6, -1, -1, -1), CGXDateTime(-1, -1, -1, 8, -1, -1, -1)));
    pAa->SetStatus(AUTO_ANSWER_STATUS_INACTIVE);
    pAa->SetNumberOfCalls(0);
    pAa->SetNumberOfRingsInListeningWindow(1);
    pAa->SetNumberOfRingsOutListeningWindow(2);
    items.push_back(pAa);
}

/*
* Add Modem Configuration object.
*/
static void AddModemConfigurationAL(CGXDLMSObjectCollection& items)
{
    CGXDLMSModemConfiguration* pMc = new CGXDLMSModemConfiguration();
    pMc->SetCommunicationSpeed(DLMS_BAUD_RATE_38400);
    CGXDLMSModemInitialisation init;
    vector<CGXDLMSModemInitialisation> initialisationStrings;
    init.SetRequest("AT");
    init.SetResponse("OK");
    init.SetDelay(0);
    initialisationStrings.push_back(init);
    pMc->SetInitialisationStrings(initialisationStrings);
    items.push_back(pMc);
}

/**
* Add MAC Address Setup object.
*/
static void AddMacAddressSetupAL(CGXDLMSObjectCollection& items)
{
    CGXDLMSMacAddressSetup* pMac = new CGXDLMSMacAddressSetup();
    pMac->SetMacAddress("00:11:22:33:44:55:66");
    items.push_back(pMac);
}

/**
* Add IP4 setup object.
*/
/*static CGXDLMSIp4Setup* AddIp4SetupAL(CGXDLMSObjectCollection& items, std::string& address)
{
    CGXDLMSIp4Setup* pIp4 = new CGXDLMSIp4Setup();
    pIp4->SetIPAddress(address);
    items.push_back(pIp4);
    return pIp4;
}*/

#endif //MAX_MEMORY

/*
* Generic initialize for all servers.
*/

int CGXDLMSBaseAL::CreateObjects()
{
    int ret;
     //Get local IP address.
    std::string address;
    GetIpAddressAL(address);

#if MAX_MEMORY
    unsigned long sn = 123456;
    CGXDLMSData* ldn = AddLogicalDeviceNameAL(GetItems(), sn);
    //Add firmaware.
    AddFirmwareVersionAL(GetItems());
    AddElectricityID1AL(GetItems(), sn);
    AddElectricityID2AL(GetItems(), sn);

    //Add Last avarage.
    CGXDLMSRegister* pRegister = new CGXDLMSRegister("1.1.21.25.0.255");
    //Set access right. Client can't change Device name.
    pRegister->SetAccess(2, DLMS_ACCESS_MODE_READ);
    GetItems().push_back(pRegister);
#endif  //MAX_MEMORY

	int count = GetItems().size();

	/* HUMIDITY_OBJECT */
	unsigned char test_humidity_val[] = {0xDE, 0xAD, 0xBE, 0xEF};
	CGXDLMSVariant humidity_value(test_humidity_val, 4, DLMS_DATA_TYPE_OCTET_STRING);
    CGXDLMSData* pDataHumidity = new CGXDLMSData(HUMIDITY_OBJECT);
	pDataHumidity->SetValue(humidity_value);
    GetItems().push_back(pDataHumidity);

	/* POWER_OBJECT */
	CGXDLMSVariant power_value = 0;
	CGXDLMSData* pDataPower = new CGXDLMSData(POWER_OBJECT);
	pDataPower->SetValue(power_value);
	GetItems().push_back(pDataPower);

	/* CURRENT_OBJECT */
	CGXDLMSVariant current_value = 0;
	CGXDLMSData* pDataCurrent = new CGXDLMSData(CURRENT_OBJECT);
	pDataCurrent->SetValue(current_value);
	GetItems().push_back(pDataCurrent);

    CGXDLMSVariant temp_value=30;
    //add temp value
    CGXDLMSRegister* pRegisterTemp = new CGXDLMSRegister(TEMPERATURE_OBJECT);
    pRegisterTemp->SetValue(temp_value);
    GetItems().push_back(pRegisterTemp);

    /* ACTIVE_ENERGY - data*/
    int energy = 10;
	CGXDLMSVariant active_energy(energy);
    CGXDLMSData* pDataActiveEnergy = new CGXDLMSData(ACTIVE_ENERGY);
    pDataActiveEnergy->SetValue(active_energy);
    GetItems().push_back(pDataActiveEnergy);

	/* REACTIVE_ENERGY - register*/
    unsigned long reactive = 2;
	CGXDLMSVariant reactive_energy(reactive);
	CGXDLMSRegister* pReactiveEnergy = new CGXDLMSRegister(REACTIVE_ENERGY);
	pReactiveEnergy->SetValue(reactive_energy);
	pReactiveEnergy->SetScaler(1.0);
	pReactiveEnergy->SetUnit(1);
	GetItems().push_back(pReactiveEnergy);

	/* SUM_LI_ACTIVE_POWER - extended register */
	unsigned long sum = 5;
	CGXDLMSVariant sum_li(sum);
	CGXDLMSExtendedRegister* pSumLi = new CGXDLMSExtendedRegister(SUM_LI_ACTIVE_POWER);
	pDataCurrent->SetValue(sum_li);
	pReactiveEnergy->SetScaler(10.0);
	pReactiveEnergy->SetUnit(1);
	GetItems().push_back(pSumLi);

	/* MANUFACTURER_SPECIFIC - data */
    bool on_off = 1;
    CGXDLMSVariant custom_value(on_off);
    CGXDLMSData* pCustomObj = new CGXDLMSData(MANUFACTURER_SPECIFIC);
    pCustomObj->SetValue(custom_value);
    GetItems().push_back(pCustomObj);
    count = GetItems().size();

#ifdef __MBED__
    prev_led = on_off;
    led1 = 0;
    led2 = 0;
    led3 = 1;
#endif

#if MAX_MEMORY

    //Add default clock. Clock's Logical Name is 0.0.1.0.0.255.
    CGXDLMSClock* pClock = new CGXDLMSClock();
    CGXDateTime begin(-1, 9, 1, -1, -1, -1, -1);
    pClock->SetBegin(begin);
    CGXDateTime end(-1, 3, 1, -1, -1, -1, -1);
    pClock->SetEnd(end);
    GetItems().push_back(pClock);
    //Add Tcp/Udp setup. Default Logical Name is 0.0.25.0.0.255.
    GetItems().push_back(new CGXDLMSTcpUdpSetup());

    ///////////////////////////////////////////////////////////////////////
    //Add profile generic (historical data) object.
    CGXDLMSProfileGeneric* profileGeneric = new CGXDLMSProfileGeneric("1.0.99.1.0.255");
    //Set capture period to 60 second.
    profileGeneric->SetCapturePeriod(60);
    profileGeneric->SetSortMethod(DLMS_SORT_METHOD_FIFO);
    profileGeneric->SetSortObject(pClock);
    //Add colums.
    //Set saved attribute index.
    CGXDLMSCaptureObject * capture = new CGXDLMSCaptureObject(2, 0);
    profileGeneric->GetCaptureObjects().push_back(std::pair<CGXDLMSObject*, CGXDLMSCaptureObject*>(pClock, capture));
    //Set saved attribute index.
    capture = new CGXDLMSCaptureObject(2, 0);
    profileGeneric->GetCaptureObjects().push_back(std::pair<CGXDLMSObject*, CGXDLMSCaptureObject*>(pRegister, capture));
    GetItems().push_back(profileGeneric);

    // Create 10 000 rows for profile generic file.
    // In example profile generic we have two columns.
    // Date time and integer value.
    int rowCount = 10000;
    CGXDateTime tm = CGXDateTime::Now();
    tm.AddMinutes(-tm.GetValue().tm_min);
    tm.AddSeconds(-tm.GetValue().tm_sec);
    tm.AddHours(-(rowCount - 1));

	//mbed has no file system
    /*FILE* f = fopen(DATAFILE, "w");
    for (int pos = 0; pos != rowCount; ++pos) {
        fprintf(f, "%s;%d\n", tm.ToString().c_str(), pos + 1);
        tm.AddHours(1);
    }
    fclose(f);*/
    //Maximum row count.
    profileGeneric->SetEntriesInUse(rowCount);
    profileGeneric->SetProfileEntries(rowCount);

    ///////////////////////////////////////////////////////////////////////
    //Add Auto connect object.
    AddAutoConnectAL(GetItems());

    ///////////////////////////////////////////////////////////////////////
    //Add Activity Calendar object.
    AddActivityCalendarAL(GetItems());

    ///////////////////////////////////////////////////////////////////////
    //Add Optical Port Setup object.
    AddOpticalPortSetupAL(GetItems());
    ///////////////////////////////////////////////////////////////////////
    //Add Demand Register object.
    AddDemandRegisterAL(GetItems());

    ///////////////////////////////////////////////////////////////////////
    //Add Register Monitor object.
    AddRegisterMonitorAL(GetItems(), pRegister);

    ///////////////////////////////////////////////////////////////////////
    //Add action schedule object.
    AddActionScheduleAL(GetItems());

    ///////////////////////////////////////////////////////////////////////
    //Add SAP Assignment object.
    AddSapAssignmentAL(GetItems());

    ///////////////////////////////////////////////////////////////////////
    //Add Auto Answer object.
    AddAutoAnswerAL(GetItems());

    ///////////////////////////////////////////////////////////////////////
    //Add Modem Configuration object.
    AddModemConfigurationAL(GetItems());

    ///////////////////////////////////////////////////////////////////////
    //Add Mac Address Setup object.
    AddMacAddressSetupAL(GetItems());
    ///////////////////////////////////////////////////////////////////////
    //Add IP4 Setup object.
    //CGXDLMSIp4Setup* pIp4 = AddIp4Setup(GetItems(), address);

    ///////////////////////////////////////////////////////////////////////
    //Add Push Setup object.
    CGXDLMSPushSetup* pPush = new CGXDLMSPushSetup();
    pPush->SetDestination(address);
    GetItems().push_back(pPush);

    // Add push object itself. This is needed to tell structure of data to
    // the Push listener.
    pPush->GetPushObjectList().push_back(std::pair<CGXDLMSObject*, CGXDLMSCaptureObject>(pPush, CGXDLMSCaptureObject(2, 0)));
    //Add logical device name.
    pPush->GetPushObjectList().push_back(std::pair<CGXDLMSObject*, CGXDLMSCaptureObject>(ldn, CGXDLMSCaptureObject(2, 0)));
    // Add 0.0.25.1.0.255 Ch. 0 IPv4 setup IP address.
    //pPush->GetPushObjectList().push_back(std::pair<CGXDLMSObject*, CGXDLMSCaptureObject>(pIp4, CGXDLMSCaptureObject(3, 0)));
    count = GetItems().size();
#endif //MAX_MEMORY
	///////////////////////////////////////////////////////////////////////
    //Server must initialize after all objects are added.
    ret = Initialize();
    if (ret != DLMS_ERROR_CODE_OK)
    {
        return ret;
    }
    return DLMS_ERROR_CODE_OK;
}

int CGXDLMSBaseAL::Init(const char* pPort)
{
    int ret;
    if ((ret = StartServer(NULL, pPort)) != 0)
    {
        return ret;
    }

    return ret;
}

CGXDLMSObject* CGXDLMSBaseAL::FindObject(
    DLMS_OBJECT_TYPE objectType,
    int sn,
    std::string& ln)
{
    return NULL;
}

/**
* Return data using start and end indexes.
*
* @param p
*            ProfileGeneric
* @param index
* @param count
* @return Add data Rows
*/
static void GetProfileGenericDataByEntryAL(CGXDLMSProfileGeneric* p, long index, unsigned long count)
{
    int len, month = 0, day = 0, year = 0, hour = 0, minute = 0, second = 0, value = 0;
    // Clear old data. It's already serialized.
    p->GetBuffer().clear();
    if (count != 0)
    {
        FILE* f = fopen(DATAFILE, "r");
        if (f != NULL)
        {
            while ((len = fscanf(f, "%d/%d/%d %d:%d:%d;%d", &month, &day, &year, &hour, &minute, &second, &value)) != -1)
            {
                // Skip row
                if (index > 0) {
                    --index;
                }
                else if (len == 7)
                {
                    if (p->GetBuffer().size() == count) {
                        break;
                    }
                    CGXDateTime tm(2000 + year, month, day, hour, minute, second, 0, 0x8000);
                    std::vector<CGXDLMSVariant> row;
                    row.push_back(tm);
                    row.push_back(value);
                    p->GetBuffer().push_back(row);
                }
                if (p->GetBuffer().size() == count) {
                    break;
                }
            }
            fclose(f);
        }
    }
}

/**
* Find start index and row count using start and end date time.
*
* @param start
*            Start time.
* @param end
*            End time
* @param index
*            Start index.
* @param count
*            Item count.
*/
static void GetProfileGenericDataByRangeAL(CGXDLMSValueEventArg* e)
{
    int len, month = 0, day = 0, year = 0, hour = 0, minute = 0, second = 0, value = 0;
    CGXDLMSVariant start, end;
    CGXByteBuffer bb;
    bb.Set(e->GetParameters().Arr[1].byteArr, e->GetParameters().Arr[1].size);
    CGXDLMSClient::ChangeType(bb, DLMS_DATA_TYPE_DATETIME, start);
    bb.Clear();
    bb.Set(e->GetParameters().Arr[2].byteArr, e->GetParameters().Arr[2].size);
    CGXDLMSClient::ChangeType(bb, DLMS_DATA_TYPE_DATETIME, end);
    FILE* f = fopen(DATAFILE, "r");
    if (f != NULL)
    {
        while ((len = fscanf(f, "%d/%d/%d %d:%d:%d;%d", &month, &day, &year, &hour, &minute, &second, &value)) != -1)
        {
            CGXDateTime tm(2000 + year, month, day, hour, minute, second, 0, 0x8000);
            if (tm.CompareTo(end.dateTime) > 0) {
                // If all data is read.
                break;
            }
            if (tm.CompareTo(start.dateTime) < 0) {
                // If we have not find first item.
                e->SetRowBeginIndex(e->GetRowBeginIndex() + 1);
            }
            e->SetRowEndIndex(e->GetRowEndIndex() + 1);
        }
        fclose(f);
    }
}

/**
* Get row count.
*
* @return
*/
static int GetProfileGenericDataCountAL() {
    int rows = 0;
    int ch;
    FILE* f = fopen(DATAFILE, "r");
    if (f != NULL)
    {
        while ((ch = fgetc(f)) != EOF)
        {
            if (ch == '\n')
            {
                ++rows;
            }
        }
        fclose(f);
    }
    return rows;
}


/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void CGXDLMSBaseAL::PreRead(std::vector<CGXDLMSValueEventArg*>& args)
{
    CGXDLMSVariant value;
    CGXDLMSObject* pObj;
    int ret, index;
    DLMS_OBJECT_TYPE type;
    std::string ln;
    for (std::vector<CGXDLMSValueEventArg*>::iterator it = args.begin(); it != args.end(); ++it)
    {
        //Let framework handle Logical Name read.
        if ((*it)->GetIndex() == 1)
        {
            continue;
        }
        //Get attribute index.
        index = (*it)->GetIndex();
        pObj = (*it)->GetTarget();
        //Get target type.
        type = pObj->GetObjectType();
        if (type == DLMS_OBJECT_TYPE_PROFILE_GENERIC)
        {
            CGXDLMSProfileGeneric* p = (CGXDLMSProfileGeneric*)pObj;
            // If buffer is read and we want to save memory.
            if (index == 6) {
                // If client wants to know EntriesInUse.
                p->SetEntriesInUse(GetProfileGenericDataCountAL());
            }
            else if (index == 2)
            {
                // Read rows from file.
                // If reading first time.
                if ((*it)->GetRowEndIndex() == 0) {
                    if ((*it)->GetSelector() == 0) {
                        (*it)->SetRowEndIndex(GetProfileGenericDataCountAL());
                    }
                    else if ((*it)->GetSelector() == 1) {
                        // Read by entry.
                        GetProfileGenericDataByRangeAL((*it));
                    }
                    else if ((*it)->GetSelector() == 2) {
                        // Read by range.
                        unsigned int begin = (*it)->GetParameters().Arr[0].ulVal;
                        (*it)->SetRowBeginIndex(begin);
                        (*it)->SetRowEndIndex(begin + (*it)->GetParameters().Arr[1].ulVal);
                        // If client wants to read more data what we have.
                        int cnt = GetProfileGenericDataCountAL();
                        if ((*it)->GetRowEndIndex() - (*it)->GetRowBeginIndex() > cnt - (*it)->GetRowBeginIndex())
                        {
                            (*it)->SetRowEndIndex(cnt - (*it)->GetRowBeginIndex());
                          //  if ((*it)->GetRowEndIndex() < 0) {
                           //     (*it)->SetRowEndIndex(0);
                           // }
                        }
                    }
                }
                long count = (*it)->GetRowEndIndex() - (*it)->GetRowBeginIndex();
                // Read only rows that can fit to one PDU.
                if ((*it)->GetRowEndIndex() - (*it)->GetRowBeginIndex() > (*it)->GetRowToPdu()) {
                    count = (*it)->GetRowToPdu();
                }
                GetProfileGenericDataByEntryAL(p, (*it)->GetRowBeginIndex(), count);
            }
            continue;
        }
        //Framework will handle Association objects automatically.
        if (type == DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME ||
            type == DLMS_OBJECT_TYPE_ASSOCIATION_SHORT_NAME ||
            //Framework will handle profile generic automatically.
            type == DLMS_OBJECT_TYPE_PROFILE_GENERIC)
        {
        	//printf("continue \n");
            continue;
        }
        DLMS_DATA_TYPE ui, dt;
        (*it)->GetTarget()->GetUIDataType(index, ui);
        (*it)->GetTarget()->GetDataType(index, dt);
        //Update date and time of clock object.
        if (type == DLMS_OBJECT_TYPE_CLOCK && index == 2)
        {
            CGXDateTime tm = CGXDateTime::Now();
            ((CGXDLMSClock*)pObj)->SetTime(tm);
            continue;
        }
        else if (type == DLMS_OBJECT_TYPE_REGISTER_MONITOR)
        {
            CGXDLMSRegisterMonitor* pRm = (CGXDLMSRegisterMonitor*)pObj;
            if (index == 2)
            {
                //Initialize random seed.
                srand((unsigned int)time(NULL));
                pRm->GetThresholds().clear();
                pRm->GetThresholds().push_back(rand() % 100 + 1);
                continue;
            }
        }
        else
        {
            CGXDLMSVariant null;
            CGXDLMSValueEventArg e(pObj, index);
            ret = ((IGXDLMSBase*)pObj)->GetValue(m_Settings, e);
            if (ret != DLMS_ERROR_CODE_OK)
            {
                //TODO: Show error.
                continue;
            }
            //If data is not assigned and value type is unknown return number.
            DLMS_DATA_TYPE tp = e.GetValue().vt;
            if (tp == DLMS_DATA_TYPE_INT8 ||
                tp == DLMS_DATA_TYPE_INT16 ||
                tp == DLMS_DATA_TYPE_INT32 ||
                tp == DLMS_DATA_TYPE_INT64 ||
                tp == DLMS_DATA_TYPE_UINT8 ||
                tp == DLMS_DATA_TYPE_UINT16 ||
                tp == DLMS_DATA_TYPE_UINT32 ||
                tp == DLMS_DATA_TYPE_UINT64)
            {
                //Initialize random seed.
                srand((unsigned int)time(NULL));
                value = rand() % 100 + 1;
                value.vt = tp;
                e.SetValue(value);
            }
        }
    }
}

void CGXDLMSBaseAL::PostRead(std::vector<CGXDLMSValueEventArg*>& args)
{
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void CGXDLMSBaseAL::PreWrite(std::vector<CGXDLMSValueEventArg*>& args)
{
    std::string ln;
    for (std::vector<CGXDLMSValueEventArg*>::iterator it = args.begin(); it != args.end(); ++it)
    {
        (*it)->GetTarget()->GetLogicalName(ln);
        printf("Writing: %s \r\n", ln.c_str());
        ln.clear();
    }
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void CGXDLMSBaseAL::PostWrite(std::vector<CGXDLMSValueEventArg*>& args)
{
}


/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void CGXDLMSBaseAL::PreAction(std::vector<CGXDLMSValueEventArg*>& args)
{
}

static void HandleProfileGenericActionsAL(CGXDLMSValueEventArg* it)
{
    CGXDLMSProfileGeneric* pg = (CGXDLMSProfileGeneric*)it->GetTarget();
    if (it->GetIndex() == 1) {
        // Profile generic clear is called. Clear data.
        FILE* f = fopen(DATAFILE, "w");
        fclose(f);
    }
    else if (it->GetIndex() == 2) {
        // Profile generic Capture is called.
        FILE* f = fopen(DATAFILE, "a");
        for (unsigned int pos = pg->GetBuffer().size() - 1; pos != pg->GetBuffer().size(); ++pos)
        {
            CGXDateTime tm = pg->GetBuffer().at(0).at(0).dateTime;
            int value = pg->GetBuffer().at(0).at(1).ToInteger();
            fprintf(f, "%s;%d\n", tm.ToString().c_str(), value);
        }
        fclose(f);
    }
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void CGXDLMSBaseAL::PostAction(std::vector<CGXDLMSValueEventArg*>& args)
{
    for (std::vector<CGXDLMSValueEventArg*>::iterator it = args.begin(); it != args.end(); ++it)
    {
        if ((*it)->GetTarget()->GetObjectType() == DLMS_OBJECT_TYPE_PROFILE_GENERIC)
        {
            HandleProfileGenericActionsAL(*it);
        }
    }
}


bool CGXDLMSBaseAL::IsTarget(
    unsigned long int serverAddress,
    unsigned long clientAddress)
{
    return true;
}

DLMS_SOURCE_DIAGNOSTIC CGXDLMSBaseAL::ValidateAuthentication(
    DLMS_AUTHENTICATION authentication,
    CGXByteBuffer& password)
{
    if (authentication == DLMS_AUTHENTICATION_NONE)
    {
        //Uncomment this if authentication is always required.
        //return DLMS_SOURCE_DIAGNOSTIC_AUTHENTICATION_MECHANISM_NAME_REQUIRED;
    }

    if (authentication == DLMS_AUTHENTICATION_LOW)
    {
        CGXByteBuffer expected;
        std::string name = "0.0.40.0.0.255";
        if (GetUseLogicalNameReferencing())
        {
            CGXDLMSAssociationLogicalName* ln =
                (CGXDLMSAssociationLogicalName*)GetItems().FindByLN(
                    DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME, name);
            expected = ln->GetSecret();
        }
        else
        {
            CGXDLMSAssociationShortName* sn =
                (CGXDLMSAssociationShortName*)GetItems().FindByLN(
                    DLMS_OBJECT_TYPE_ASSOCIATION_SHORT_NAME, name);
            expected = sn->GetSecret();
        }
        if (expected.Compare(password.GetData(), password.GetSize()))
        {
            return DLMS_SOURCE_DIAGNOSTIC_NONE;
        }
        return DLMS_SOURCE_DIAGNOSTIC_AUTHENTICATION_FAILURE;
    }
    // Other authentication levels are check on phase two.
    return DLMS_SOURCE_DIAGNOSTIC_NONE;
}

DLMS_ACCESS_MODE CGXDLMSBaseAL::GetAttributeAccess(CGXDLMSValueEventArg* arg)
{
// TODO - This logic will need to be re-enabled after we add the support for APDU encryption in M6
#ifdef M6_FUNCTIONALITY
    // Only read is allowed
    if (arg->GetSettings()->GetAuthentication() == DLMS_AUTHENTICATION_NONE)
    {
        return DLMS_ACCESS_MODE_READ;
    }
    // Only clock write is allowed.
    if (arg->GetSettings()->GetAuthentication() == DLMS_AUTHENTICATION_LOW)
    {
        if (arg->GetTarget()->GetObjectType() == DLMS_OBJECT_TYPE_CLOCK)
        {
            return DLMS_ACCESS_MODE_READ_WRITE;
        }
        return DLMS_ACCESS_MODE_READ;
    }
#endif
    // All writes are allowed.
    return DLMS_ACCESS_MODE_READ_WRITE;
}

/**
* Get method access mode.
*
* @param arg
*            Value event argument.
* @return Method access mode.
* @throws Exception
*             Server handler occurred exceptions.
*/
DLMS_METHOD_ACCESS_MODE CGXDLMSBaseAL::GetMethodAccess(CGXDLMSValueEventArg* arg)
{
    // Methods are not allowed.
    if (arg->GetSettings()->GetAuthentication() == DLMS_AUTHENTICATION_NONE)
    {
        return DLMS_METHOD_ACCESS_MODE_NONE;
    }
    // Only clock methods are allowed.
    if (arg->GetSettings()->GetAuthentication() == DLMS_AUTHENTICATION_LOW)
    {
        if (arg->GetTarget()->GetObjectType() == DLMS_OBJECT_TYPE_CLOCK)
        {
            return DLMS_METHOD_ACCESS_MODE_ACCESS;
        }
        return DLMS_METHOD_ACCESS_MODE_NONE;
    }
    return DLMS_METHOD_ACCESS_MODE_ACCESS;
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void CGXDLMSBaseAL::Connected(
    CGXDLMSConnectionEventArgs& connectionInfo)
{
    //printf("Connected.\r\n");
}

void CGXDLMSBaseAL::InvalidConnection(
    CGXDLMSConnectionEventArgs& connectionInfo)
{
    printf("InvalidConnection.\r\n");

}
/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void CGXDLMSBaseAL::Disconnected(
    CGXDLMSConnectionEventArgs& connectionInfo)
{
    printf("Disconnected.\r\n");
}

void CGXDLMSBaseAL::PreGet(
    std::vector<CGXDLMSValueEventArg*>& args)
{
    for (std::vector<CGXDLMSValueEventArg*>::iterator it = args.begin(); it != args.end(); ++it)
    {
        if ((*it)->GetTarget()->GetObjectType() == DLMS_OBJECT_TYPE_PROFILE_GENERIC)
        {
            CGXDLMSProfileGeneric* pg = (CGXDLMSProfileGeneric*)(*it)->GetTarget();
            pg->GetBuffer().clear();
            int cnt = GetProfileGenericDataCountAL() + 1;
            // Update last average value.
            CGXDateTime tm = CGXDateTime::Now();
            std::vector<CGXDLMSVariant> row;
            row.push_back(tm);
            row.push_back(cnt);
            pg->GetBuffer().push_back(row);
            (*it)->SetHandled(true);
        }
    }
}

void CGXDLMSBaseAL::PostGet(
    std::vector<CGXDLMSValueEventArg*>& args)
{

}


STATUS CGXDLMSUdp::CreateSocket()
{
#ifdef __linux__
	m_ServerSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_ServerSocket > 0)
		return SUCCESS;
	else
	{
		printf("m_ServerSocket = %d\n", m_ServerSocket);
		return -1;
	}
#else // MBED
	return pal_socket(PAL_AF_INET, PAL_SOCK_DGRAM, false, 0, &m_ServerSocket);
#endif
}


STATUS CGXDLMSUdp::Listen(int backlog)
{
	return 0;
}

int CGXDLMSUdp::Read(SOCKET dummy, CGXByteBuffer &bb, size_t *recievedDataSize)
{
	m_fromLength = sizeof(SOCKADDR);
#ifdef __linux__
	int bytes_received = recvfrom(m_ServerSocket, (char*)bb.GetData() + bb.GetSize(), bb.Capacity() - bb.GetSize(), 0, &m_from, &m_fromLength);

	if (bytes_received != -1)
	{
		*recievedDataSize = bytes_received;
		return SUCCESS;
	}
	else return -1;
#else // MBED
	return pal_receiveFrom(m_ServerSocket, (char*)bb.GetData() + bb.GetSize(), bb.Capacity() - bb.GetSize(), &m_from, &m_fromLength, recievedDataSize);
#endif
}

int CGXDLMSUdp::Write(SOCKET dummy, CGXByteBuffer &bb, size_t *sentDataSize)
{
#ifdef __linux__
	return sendto(m_ServerSocket, bb.GetData(), bb.GetSize() - bb.GetPosition(), 0, &m_from, m_fromLength);
#else // MBED
	return pal_sendTo(m_ServerSocket, bb.GetData(), bb.GetSize() - bb.GetPosition(), &m_from, m_fromLength, sentDataSize);
#endif
}

STATUS CGXDLMSUdp::Accept(SOCKET *dummy0, SOCKADDR *dummy1, SOCKLEN *dummy2)
{
	return SUCCESS;
}

int CGXDLMSUdp::ConnectPort(char *ip_address, const char* pPort)
{
	int ret = 0;
	int port = atoi(pPort);
	if ((ret = Connect(ip_address,port)) != 0)
	{
		printf("Failed to connect port\n");
		return ret;
	}
	return ret;
}

STATUS CGXDLMSUdp::CloseSocket(SOCKET dummy)
{
#ifdef __linux__
	STATUS ret = close(m_ServerSocket);
#else // MBED
	STATUS ret = pal_close(&m_ServerSocket);
#endif
	if (ret == SUCCESS)
		m_ServerSocket = (STATUS)0;
	return ret;
}

STATUS CGXDLMSTcp::CreateSocket()
{
#ifdef __linux__
	m_ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_ServerSocket > 0)
		return SUCCESS;
	else
		return -1;
#else // MBED
	return pal_socket(PAL_AF_INET, PAL_SOCK_STREAM_SERVER, false, 0, &m_ServerSocket);
#endif
}

STATUS CGXDLMSTcp::Listen(int backlog)
{
#ifdef __linux__
	return listen(m_ServerSocket, backlog);
#else
	return pal_listen(m_ServerSocket, backlog);
#endif
}

int CGXDLMSTcp::Read(SOCKET client_sock, CGXByteBuffer &bb, size_t *recievedDataSize)
{
#ifdef __linux__
	int bytes_received = recv(client_sock, (char*)bb.GetData() + bb.GetSize(), bb.Capacity() - bb.GetSize(), 0);
	if (bytes_received != -1)
	{
		*recievedDataSize = bytes_received;
		return SUCCESS;
	}
	else return -1;
#else
	return pal_recv(client_sock, (char*)bb.GetData() + bb.GetSize(), bb.Capacity() - bb.GetSize(), recievedDataSize);
#endif
}

int CGXDLMSTcp::Write(SOCKET clinet_socket, CGXByteBuffer &bb, size_t *sentDataSize)
{
#ifdef __linux__
	*sentDataSize = send(clinet_socket, bb.GetData(), bb.GetSize() - bb.GetPosition(), 0);
	return *sentDataSize;
#else
	return pal_send(clinet_socket, (const char*)bb.GetData(), bb.GetSize() - bb.GetPosition(), sentDataSize);
#endif
}

STATUS CGXDLMSTcp::Accept(SOCKET *client_sock, SOCKADDR *client_sock_addr, SOCKLEN *client_sock_addr_len)
{
#ifdef __linux__
	*client_sock = accept(m_ServerSocket, client_sock_addr, client_sock_addr_len);
	if (*client_sock > 0) return SUCCESS;
	else return -1;

#else // MBED
	return pal_accept(m_ServerSocket, client_sock_addr, client_sock_addr_len, client_sock);
#endif
}

int CGXDLMSTcp::ConnectPort(char *ip_address,const char* pPort)
{
	int ret = 0;
	int port = atoi(pPort);
	if ((ret = Connect(ip_address,port)) != 0)
	{
		printf("Failed to connect port\n");
		return ret;
	}

	return ret;
}

STATUS CGXDLMSTcp::CloseSocket(SOCKET socket)
{
#ifdef __linux__
	STATUS ret = close(socket);
#else
	return pal_close(&socket);
#endif
}
