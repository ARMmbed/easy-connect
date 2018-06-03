//----------------------------------------------------------------------------
// The confidential and proprietary information contained in this file may
// only be used by a person authorised under and to the extent permitted
// by a subsisting licensing agreement from ARM Limited or its affiliates.
//
// (C) COPYRIGHT 2016 ARM Limited or its affiliates.
// ALL RIGHTS RESERVED
//
// This entire notice must be reproduced on all copies of this file
// and copies of this file may only be made by a person if such person is
// permitted to do so under the terms of a subsisting license agreement
// from ARM Limited or its affiliates.
//----------------------------------------------------------------------------

#ifdef __MBED__
#include "source/setup.h"
#include "mbed-os/features/FEATURE_LWIP/lwip-interface/EthernetInterface.h"
#endif

#if defined(CLI_MODE) || defined(__MBED__)
#include "init_plat.h"
#endif // defined(CLI_MODE) || defined(__MBED__)

#include <cstring>

#include "comp_defines.h"
#include "GXDLMSBaseAL.h"
#include "GXDLMSServerFactory.h"


#include "GXTime.h"
#include "GXDate.h"
#include "GXDLMSClient.h"
#include "GXDLMSData.h"
#include "GXDLMSRegister.h"
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


static int get_hex_value(unsigned char c);
static int hex_string_to_int(char *s);

static CGXDLMSBaseAL *server = NULL;

static DLMS_INTERFACE_TYPE interfaceType = DLMS_INTERFACE_TYPE_WRAPPER;
static DLMS_SERVICE_TYPE protocolType = DLMS_SERVICE_TYPE_UDP;
static int max_pdu_size = 1024;
static char *port = LN_SERVER_PORT_STR;
static bool is_print_packets = false;
static int conformance = DLMS_CONFORMANCE_GENERAL_PROTECTION |
DLMS_CONFORMANCE_GENERAL_BLOCK_TRANSFER |
DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_GET_OR_READ |
DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_SET_OR_WRITE | DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_ACTION |
DLMS_CONFORMANCE_GET | DLMS_CONFORMANCE_SET |
DLMS_CONFORMANCE_SELECTIVE_ACCESS | DLMS_CONFORMANCE_ACTION;

static unsigned char server_sys_title[] =
{
	0x4D,0x4D,0x4D,0x00,0x00,0x00,0x00,0x01
};

static unsigned char d_server[] =
{
	0xB5,0x82,0xD8,0xC9,0x10,0x01,0x83,0x02,0xBA,0x31,0x31,
	0xBA,0xB9,0xBB,0x68,0x38,0x10,0x8B,0xB9,0x40,0x8C,0x30,
	0xB2,0xE4,0x92,0x85,0x98,0x52,0x56,0xA5,0x90,0x38
};

static unsigned char q_server[] =
{
	0xE4,0xD0,0x7C,0xEB,0x0A,0x5A,0x6D,0xA9,0xD2,0x22,0x8B,
	0x05,0x4A,0x1F,0x5E,0x29,0x5E,0x17,0x47,0xA9,0x63,0x97,
	0x4A,0xF7,0x50,0x91,0xA0,0xB0,0xBC,0x2F,0xB9,0x2D,0xA7,
	0xD2,0xAB,0xD9,0xFD,0xD4,0x15,0x79,0xF3,0x6A,0x1C,0x81,
	0x71,0xA0,0xCB,0x63,0x82,0x21,0xDF,0x19,0x49,0xFD,0x95,
	0xC8,0xFA,0xE1,0x48,0x89,0x69,0x20,0x45,0x0D
};

static int getopt(int argc, char* argv[])
{
	for (int i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], "-i") == 0)
		{
			if (i + 1 < argc)
			{
				if (strcmp(argv[i + 1], "hdlc") == 0)
				{
					printf("### configuration ###   interfaceType = DLMS_INTERFACE_TYPE_HDLC\n");
					interfaceType = DLMS_INTERFACE_TYPE_HDLC;
					++i;
				}

				else if (strcmp(argv[i + 1], "wrapper") == 0)
				{
					printf("### configuration ###   interfaceType = DLMS_INTERFACE_TYPE_WRAPPER\n");
					interfaceType = DLMS_INTERFACE_TYPE_WRAPPER;
					++i;
				}
			}
		}

		else if (strcmp(argv[i], "-protocol") == 0)
		{
			if (i + 1 < argc)
			{
				if (strcmp(argv[i + 1], "tcp") == 0)
				{
					printf("### configuration ###   protocolType = DLMS_SERVICE_TYPE_TCP\n");
					protocolType = DLMS_SERVICE_TYPE_TCP;
					++i;
				}

				else if (strcmp(argv[i + 1], "udp") == 0)
				{
					printf("### configuration ###   protocolType = DLMS_SERVICE_TYPE_UDP\n");
					protocolType = DLMS_SERVICE_TYPE_UDP;
					++i;
				}

				//else if (strcmp(argv[i + 1], "serial") == 0)
				//{
				//	printf("### configuration ###   protocolType = DLMS_SERVICE_TYPE_SERIAL\n");
				//	protocolType = DLMS_SERVICE_TYPE_SERIAL;
				//	++i;
				//}
			}

		}

		else if (strcmp(argv[i], "-p") == 0)
		{
			if (i + 1 < argc)
			{
				port = argv[i + 1];
				printf("### configuration ###   port = %s\n", port);
			}

		}

		else if (strcmp(argv[i], "-m") == 0)
		{
			if (i + 1 < argc)
			{
				max_pdu_size = atoi(argv[i + 1]);
				printf("### configuration ###   max_pdu_size = %d\n", max_pdu_size);
				++i;
			}
		}

		else if (strcmp(argv[i], "-c") == 0)
		{
			if (i + 1 < argc)
			{
				conformance = hex_string_to_int(argv[i + 1]);
				printf("### configuration ###   conformance = 0x%x\n", conformance);
				++i;
			}
		}
		
		else if (strcmp(argv[i], "-print") == 0)
		{
			printf("### configuration ###   print received and sent packets\n");
			is_print_packets = true;
		}
	}
}

static int get_hex_value(unsigned char c)
{
	if (c >= '0' && c <= '9')
	{
		return c - '0';
	}

	if (c >= 'a' && c <= 'f')
	{
		return c - 'a' + 10;
	}

	if (c >= 'A' && c <= 'F')
	{
		return c - 'A' + 10;
	}
}

static int hex_string_to_int(char *s)
{
	int ret = 0;
	char *ptr = s;

	while (!((*ptr >= '1' && *ptr <= '9')
		|| (*ptr >= 'a' && *ptr <= 'f')
		|| (*ptr >= 'A' && *ptr <= 'F')))
	{
		++ptr;
	}

	while (*ptr != '\0')
	{
		ret = ret * 16 + get_hex_value(*ptr);
		++ptr;
	}

	return ret;
}

int setObj(int argc, char* argv[])
{
	if(server == NULL)
	{
		printf("server isn't running\n");
		return -1;
	}

	CGXDLMSVariant val;

	for (int i = 0; i < argc; ++i)
	{
		val.Clear();

		if (strcmp(argv[i], "-v") == 0)
		{
			if (i + 1 < argc)
			{
				std::string volStr = VOLTAGE_OBJECT;
				CGXDLMSObject *voltage_object = (server->GetItems()).FindByLN(DLMS_OBJECT_TYPE_ALL, volStr);

				if(voltage_object != NULL)
				{
					val.fltVal = atof(argv[i + 1]);
					((CGXDLMSData*)voltage_object)->SetValue(val);
					printf("### set ###   voltage = %.6f\n", val.fltVal);
					++i;
				}
			}
		}

		if (strcmp(argv[i], "-i") == 0)
		{
			if (i + 1 < argc)
			{
				std::string currStr = CURRENT_OBJECT;
				CGXDLMSObject *current_object = (server->GetItems()).FindByLN(DLMS_OBJECT_TYPE_ALL, currStr);

				if(current_object != NULL)
				{
					val.fltVal = atof(argv[i + 1]);
					((CGXDLMSData*)current_object)->SetValue(val);
					printf("### set ###   current = %.6f\n", val.fltVal);
					++i;
				}
			}
		}

		if (strcmp(argv[i], "-p") == 0)
		{
			if (i + 1 < argc)
			{
				std::string powerStr = POWER_OBJECT;
				CGXDLMSObject *power_object = (server->GetItems()).FindByLN(DLMS_OBJECT_TYPE_ALL, powerStr);

				if(power_object != NULL)
				{
					float power = atof(argv[i + 1]);
					val.fltVal = atof(argv[i + 1]);
					((CGXDLMSData*)power_object)->SetValue(val);
					printf("### set ###   power = %.6f\n", val.fltVal);
					++i;
				}
			}
		}
		
		else if (strcmp(argv[i], "-print") == 0)
		{
			server->m_print = true;
		}
		
		else if (strcmp(argv[i], "-stoprint") == 0)
		{
			server->m_print = false;
		}
	}

	return 0;
}

int kill_server(int argc, char* argv[])
{
	is_print_packets = false;
	if(server->StopServer() != 0 ||
	server->KillThread() != 0)
	{
		printf("thread didn't killed correctly\n");
		return -1;
	}

	delete server;
	server = NULL;

	return 0;
}

int main_server(int argc, char* argv[])
{
	getopt(argc, argv);

#ifdef __MBED__
	uint32_t _net_iface;
	EthernetInterface *netInterface;

	if(initPlatform() != 0) {
	   printf("ERROR - initPlatform() failed!\n");
	   return 1;
	}

	init_connection();

    if((protocolType == DLMS_SERVICE_TYPE_UDP) || (protocolType == DLMS_SERVICE_TYPE_TCP))
    {
			netInterface = new EthernetInterface();

			if(PAL_SUCCESS != pal_registerNetworkInterface(netInterface, &_net_iface)) {
				printf("Interface registration failed.");
				assert(0);
			}
    }
#endif

	server = CGXDLMSServerFactory::getCGXDLMSServer(true, interfaceType, protocolType);
	server->SetMaxReceivePDUSize(max_pdu_size);
	server->SetConformance((DLMS_CONFORMANCE)conformance);
	server->m_print = is_print_packets;

	///////////////////// ECDSA /////////////////////////////
	server->SetPrivateKey(d_server);
	server->SetServerPublicKey(q_server);

	CGXByteBuffer server_system_title;
	server_system_title.Set(server_sys_title, 8);
	(server->GetCiphering())->SetSystemTitle(server_system_title);
	/////////////////////////////////////////////////////////

	server->CreateObjects();

	server->StartServer(port);

    return 0;
}
