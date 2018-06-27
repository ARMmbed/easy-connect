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
#include "hls_keys.h"


static int get_hex_value(unsigned char c);
static int hex_string_to_int(char *s);

static CGXDLMSBaseAL *server = NULL;

static TEST_CASE s_test_case = NO_TEST;
static DLMS_INTERFACE_TYPE interfaceType = DLMS_INTERFACE_TYPE_WRAPPER;
static DLMS_SERVICE_TYPE protocolType = DLMS_SERVICE_TYPE_UDP;
static int max_pdu_size = 1024;
static int key_number = 0;
key_pair_t keys;
static char *port = LN_SERVER_PORT_STR;
static bool is_print_packets = false;
static int conformance = DLMS_CONFORMANCE_GENERAL_PROTECTION |
// remove until milestone 5 - GBT support
DLMS_CONFORMANCE_GENERAL_BLOCK_TRANSFER |
DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_GET_OR_READ |
DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_SET_OR_WRITE | DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_ACTION |
DLMS_CONFORMANCE_GET | DLMS_CONFORMANCE_SET |
DLMS_CONFORMANCE_SELECTIVE_ACCESS | DLMS_CONFORMANCE_ACTION;

static unsigned char server_sys_title[] =
{
	0x4D,0x4D,0x4D,0x00,0x00,0x00,0x00,0x01
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

		else if (strcmp(argv[i], "-k") == 0)
		{
			if (i + 1 < argc)
			{
				key_number = atoi(argv[i + 1]);
				key_number = (key_number > NUM_OF_KEYS || key_number < 1) ? 1 : key_number;
				printf("### configuration ###   use key number %d\n", key_number);
				++i;
			}
		}

		else if (strcmp(argv[i], "-test") == 0)
		{
			if (i + 1 < argc)
			{
				int test_num = atoi(argv[i + 1]);
				test_num = (test_num > 5 || test_num < 1) ? 0 : test_num;
				s_test_case = (TEST_CASE)test_num;
				printf("### configuration ###   test case number %d\n", test_num);
				++i;
			}
		}
	}
}

static int handle_test_params()
{
	if(key_number == 0)
		key_number = 1;
	switch (s_test_case) {
		case GOOD_PATH_OPEN_FLOW_WITH_HLS: 
		{
			// set the key pair to the one in the green book example
			// key_number = 1; - default
			break;
		}
		case BAD_PATH_NO_KEY_IN_SERVER:
		{
			key_number = 0;
			break;
		}
		case BAD_PATH_FAILED_CERTIFICATE_AUTHORITY:
		{
			break;
		}
		case BAD_PATH_KEY_MISMATCH:
		{
			break;
		}
		case BAD_PATH_IDENTICAL_CHALLENGES:
		{
			break;
		}
		default:
			break;
	}

	printf("key_number=%d\n", key_number);
	if(key_number > 0 && key_number <= NUM_OF_KEYS) {
		keys.m_num_pair = key_number - 1;
		keys.m_private = private_keys[keys.m_num_pair];
		keys.m_public = public_keys[keys.m_num_pair];
		
		if(s_test_case == BAD_PATH_KEY_MISMATCH) {
			int wrong_ind = (keys.m_num_pair + 1) % NUM_OF_KEYS;
			keys.m_public = public_keys[wrong_ind];
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

		if (strcmp(argv[i], "-hum") == 0)
		{
			if (i + 1 < argc)
			{
				std::string humidityStr = HUMIDITY_OBJECT;
				CGXDLMSObject *humidity_object = (server->GetItems()).FindByLN(DLMS_OBJECT_TYPE_ALL, humidityStr);

				if(humidity_object != NULL)
				{
					val.fltVal = atof(argv[i + 1]);
					((CGXDLMSData*)humidity_object)->SetValue(val);
					printf("### set ###   humidity = %.6f\n", val.fltVal);
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
	handle_test_params();

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
	if(s_test_case != BAD_PATH_NO_KEY_IN_SERVER) {
		server->SetPrivateKey(keys.m_private);
		server->SetServerPublicKey(keys.m_public);
	}

	server->SetTestCase(s_test_case);

	CGXByteBuffer server_system_title;
	server_system_title.Set(server_sys_title, 8);
	(server->GetCiphering())->SetSystemTitle(server_system_title);
	/////////////////////////////////////////////////////////

	server->CreateObjects();

	server->StartServer(port);

    return 0;
}
