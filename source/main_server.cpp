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
//#include "mbed-os/features/FEATURE_LWIP/lwip-interface/EthernetInterface.h"
#endif

#if defined(CLI_MODE) // || defined(__MBED__)
#include "init_plat.h"
#endif // defined(CLI_MODE) || defined(__MBED__)

#include <cstring>
#include <string.h>
#include <signal.h>

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


#define DEFAUL_SERVER_WINDOW 3

static int get_hex_value(unsigned char c);
static int hex_string_to_int(char *s);
static char *hex_string_to_hex_arr(char *s, int *size);

static CGXDLMSBaseAL *server = NULL;

static TEST_CASE s_test_case = NO_TEST;
static DLMS_INTERFACE_TYPE interfaceType = DLMS_INTERFACE_TYPE_WRAPPER;
static DLMS_SERVICE_TYPE protocolType = DLMS_SERVICE_TYPE_UDP;
static int max_pdu_size = 1024;
static int key_number = 0;
static char server_ip[32];
static int error_send_packet = -1;
key_pair_t keys;
static char *port = NULL;

static bool is_print_packets = false;
static int conformance = DLMS_CONFORMANCE_GENERAL_PROTECTION |
// remove until milestone 5 - GBT support
DLMS_CONFORMANCE_GENERAL_BLOCK_TRANSFER |
DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_GET_OR_READ |
DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_SET_OR_WRITE | DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_ACTION |
DLMS_CONFORMANCE_GET | DLMS_CONFORMANCE_SET |
DLMS_CONFORMANCE_SELECTIVE_ACCESS | DLMS_CONFORMANCE_ACTION;
static char **s_set_argv = NULL;
static int s_set_argc = 0;
static char *s_drop_receive = NULL;
static char *s_drop_send = NULL;
static int s_drop_receive_size = 0;
static int s_drop_send_size = 0;
static int s_window = DEFAUL_SERVER_WINDOW;
static bool s_drop_only_received_gbt = false;
static bool s_drop_only_sent_gbt = false;

#define SERVER_SYS_TITLE_SIZE 8

static unsigned char server_sys_title[SERVER_SYS_TITLE_SIZE] =
{
		0x53, 0x72, 0x76, 0x41, 0x72,0x30, 0x30, 0x31 //hexadecimal representation of utf-8 encoding of "SrvA001"
};

static void print_buf(char *s, int size)
{
	int i;
	int enable_print = 1;

	if(enable_print)
	{
		for (i = 0 ; i < size ; ++i)
		{
			printf("%02x ", 0xff & s[i]);

			if((i+1) % 20 == 0)
				printf("\n");
		}

		printf("\n");
	}
}

static void getopt(int argc, char* argv[])
{
	//no specific IP use localhost
	server_ip[0] = '\0';

	for (int i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], "-window") == 0)
		{
			if (i + 1 < argc)
			{
				s_window = atoi(argv[i + 1]);
				printf("### configuration ###   server_window = %d\n", s_window);
				++i;
			}
		}

		else if ((strcmp(argv[i], "-dropr") == 0) ||
				(strcmp(argv[i], "-droprgbt") == 0))
		{
			if (i + 1 < argc)
			{
				if (strcmp(argv[i], "-droprgbt") == 0)
					s_drop_only_received_gbt = true;

				s_drop_receive = (char*)hex_string_to_hex_arr(argv[i + 1], &s_drop_receive_size);
				if(s_drop_receive != NULL)
				{
					printf("### configuration ###   received packet drop is active\n");
					print_buf(s_drop_receive, s_drop_receive_size);
				}
				++i;
			}
		}

		else if ((strcmp(argv[i], "-drops") == 0) ||
				(strcmp(argv[i], "-dropsgbt") == 0))
		{
			if (i + 1 < argc)
			{
				if (strcmp(argv[i], "-dropsgbt") == 0)
					s_drop_only_sent_gbt = true;

				s_drop_send = (char*)hex_string_to_hex_arr(argv[i + 1], &s_drop_send_size);
				if(s_drop_send != NULL)
				{
					printf("### configuration ###   sent packet drop is active\n");
					print_buf(s_drop_send, s_drop_send_size);
				}
				++i;
			}
		}

		else if (strcmp(argv[i], "-i") == 0)
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

		else if (strcmp(argv[i], "-max") == 0)
		{
			if (i + 1 < argc)
			{
				max_pdu_size = atoi(argv[i + 1]);
				printf("### configuration ###   max_pdu_size = %d\n", max_pdu_size);
				++i;
			}
		}

		else if (strcmp(argv[i], "-ip") == 0)
		{
			if (i + 1 < argc)
			{
				strcpy(server_ip,argv[i + 1]);
				printf("### configuration ###   server_ip = %s\n", server_ip);
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

		else if (strcmp(argv[i], "-errsp") == 0)
		{
			if (i + 1 < argc)
			{
				error_send_packet = atoi(argv[i + 1]);
				printf("### configuration ###  error_send_packet = %d\n", error_send_packet);
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

		else if (strcmp(argv[i], "-set") == 0)
		{
			if (i + 1 < argc)
			{
				s_set_argv = argv + i + 1;
				s_set_argc = argc - i - 1;
				++i;
			}
		}
	}
}

static void handle_test_params()
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

	return 0;
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

static char *hex_string_to_hex_arr(char *s, int *size)
{
	// this function receive string of hexadecimal number starts with '0x'
	// it returns an array which represents the bit stream of the hexadecimal number
	// for example: if the input is '0x152' the output will be pointer to next bit stream: --> [01010010][00000001]

	char *ptr = s;
	char *end;
	char *ret = NULL;
	int string_len;
	int i;

	// the input string must start with '0x' or '0X'
	ptr = strstr(s,"0x");
	if(ptr == NULL)
	{
		ptr = strstr(s,"0X");
		if(ptr == NULL)
		{
			printf("%s: error - parameter should be hexadecimal number starting with 0x\n",__func__);
			return NULL;
		}
	}

	//skip the 0x sign
	ptr += 2;

	// caculate the len of the output
	string_len = strlen(ptr);
	for(i = 0 ; i < string_len; i++)
	{
		if (!isxdigit(ptr[i]))
		{
			printf("%s: error - parameter should be hexadecimal number starting with 0x\n",__func__);
			return NULL;
		}
	}

	end = ptr + string_len - 1;
	int alloc_len = (string_len % 2 == 0) ? string_len / 2 : (string_len / 2) + 1;
	*size = alloc_len;
	ret = (char*)calloc(1, alloc_len);

	assert(ret != NULL);

	i = 0;

	// next loop build the output array
	while(end >= ptr)
	{
		// every char in the input string will represent a bits in the output array (because every hexadecimal number is 4 bits)
		char hex = get_hex_value(*end);

		// if the index is even, the 4 bits goes to the lower part of the char in the output array
		if(i % 2 == 0)
		{
			ret[i / 2] = (ret[i / 2] & 0xf0) | (0x0f & hex);
		}

		// if the index is odd, the 4 bits goes to the upper part of the char in the output array
		else
		{
			ret[i / 2] = (ret[i / 2] & 0x0f) | (0xf0 & (hex << 4));
		}

		++i;
		--end;
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

		if (strcmp(argv[i], "-cur") == 0)
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

		if (strcmp(argv[i], "-pow") == 0)
		{
			if (i + 1 < argc)
			{
				std::string powerStr = POWER_OBJECT;
				CGXDLMSObject *power_object = (server->GetItems()).FindByLN(DLMS_OBJECT_TYPE_ALL, powerStr);

				if(power_object != NULL)
				{
					int power_val =  atoi(argv[i + 1]);
					val = power_val;
					((CGXDLMSData*)power_object)->SetValue(val);
					printf("### set ###   power = %d\n", power_val);
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
	printf("Server terminated successfully\n");
	return 0;
}


/* stub for secure storage functions */
static const unsigned char *get_private_key(uint32_t *size)
{
	*size = PRIVATE_KEY_SIZE;
	return keys.m_private;
}

static void handle_client_terminate_signal (int sig, siginfo_t *siginfo, void *context)
{
	printf ("%s: Got terminate signal from PID: %ld, signo: %d\n",
			__func__, (long)siginfo->si_pid, siginfo->si_signo);

	//we got kill command from teh client
	//terminate the server
	kill_server(0,NULL);

}

int main_server(int argc, char* argv[])
{
	char default_port[10];
	struct sigaction act;

	getopt(argc, argv);
	handle_test_params();


#ifdef __MBED__
	uint32_t _net_iface;

	if(initPlatform() != 0) {
	   printf("ERROR - initPlatform() failed!\n");
	   return 1;
	}

	init_connection();

	if((protocolType == DLMS_SERVICE_TYPE_UDP) || (protocolType == DLMS_SERVICE_TYPE_TCP))
	{
		if(PAL_SUCCESS != pal_registerNetworkInterface(get_network_interface(), &_net_iface)) {
			printf("Interface registration failed.");
			assert(0);
		}
	}
#endif

	server = CGXDLMSServerFactory::getCGXDLMSServer(true, interfaceType, protocolType);
	server->SetMaxServerPDUSize(max_pdu_size);
	server->SetConformance((DLMS_CONFORMANCE)conformance);
	server->m_print = is_print_packets;
	server->m_drop_receive = s_drop_receive;
	server->m_drop_send = s_drop_send;
	server->m_drop_receive_size = s_drop_receive_size;
	server->m_drop_send_size = s_drop_send_size;
	server->m_error_send_packet = error_send_packet;
	server->m_window = s_window;
	server->m_drop_only_received_gbt = s_drop_only_received_gbt;
	server->m_drop_only_sent_gbt = s_drop_only_sent_gbt;

	///////////////////// ECDSA /////////////////////////////
	if(s_test_case != BAD_PATH_NO_KEY_IN_SERVER) {
		server->SetPrivateKey(keys.m_private);
		server->SetServerPublicKey(keys.m_public);
		server->SetCB(get_private_key);
		server->InitSecurityUtils();
	}

	server->SetTestCase(s_test_case);

	CGXByteBuffer server_system_title;
	server_system_title.Set(server_sys_title, sizeof(server_sys_title));
	(server->GetCiphering())->SetSystemTitle(server_system_title);
	/////////////////////////////////////////////////////////

	server->CreateObjects();

	if (s_set_argc != 0 && s_set_argv != NULL)
	{
		setObj(s_set_argc, s_set_argv);
	}


	if(port == NULL) {
		strcpy(default_port,LN_SERVER_PORT_STR);
		port = default_port;
	}

	//register to get a notification in case a SIGTERM signal was sent by the client
	//this is done in order to gracefully terminate teh server
	memset (&act, '\0', sizeof(act));

	//Use the sa_sigaction field because the handles has two additional parameters
	act.sa_sigaction = &handle_client_terminate_signal;

	//The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler.
	act.sa_flags = SA_SIGINFO;

	if (sigaction(SIGTERM, &act, NULL) < 0) {
		perror ("sigaction");
		return 1;
	}


	if(strlen(server_ip) != 0) {
		server->StartServer(server_ip,port);
	} else {
		server->StartServer(NULL,port);
	}


    return 0;
}
