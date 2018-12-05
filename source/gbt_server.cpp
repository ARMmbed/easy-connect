
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "gbt_server.h"

#define MAX_EXPECTED_BLOCKS_TO_SEND 80
#define CLIENT_DEFAULT_WINDOW 1
#define MAX_BN_RANGE 10

#define WINDOW_MASK 0X3F
#define LB_IS_1 0X80
#define STR_IS_0 0xBF
#define STR_IS_1 0x40
#define GBT_BN_ALREADY_RECEIVED 1
#define GBT_BN_VALID 0
#define GBT_BN_NOT_VALID -1

#define GBT_ABORT -3
#define GBT_DROP_PACKET -2
#define GBT_FAILURE -1
#define GBT_RESPONSE_COMPLETE 1
#define GBT_RESPONSE_NOT_COMPLETE 2
#define GBT_PACKET_SENT 3
#define SUCCESS 0
#define GBT_FRAME_MAX_SIZE 10

static int server_get_window();
static int server_encode_and_send(int block_control, int BN, int ACK, void *buf, int size);

typedef struct OCTET_STRING {
	unsigned char *buf;	/* Buffer with consecutive OCTET_STRING bits */
	int size;	/* Size of the buffer */
} OCTET_STRING_t;

typedef struct General_Block_Transfer {
	unsigned short	 block_number;
	unsigned short	 block_number_ack;
	unsigned char	 block_control;
	OCTET_STRING_t	 block_data;
} General_Block_Transfer_t;

static packet_t *server_send = NULL;
static server_gbt_session_t *server_session = NULL;

int get_last_in_order()
{
	if (server_session->last_in_order == NULL)
		return 0;

	return server_session->last_in_order->block_number;
}

bool sent_all_data()
{
	return server_session->sent_all_data;
}

bool is_gbt_active()
{
	return (server_session != NULL);
}

void set_gbt_unactive()
{
	server_session = NULL;
}

packet_t *get_next_packet()
{
	packet_t *ret = server_send;

	if (server_send != NULL)
	{
		server_send = server_send->next;
	}

	return ret;
}

static void insert_server_packet(packet_t *packet)
{
	packet_t *ptr = server_send;

	if (ptr == NULL)
	{
		server_send = packet;
	}

	else
	{
		while(ptr->next != NULL)
		{
			ptr = ptr->next;
		}

		ptr->next = packet;
	}
}

// return value: if return NULL then send non-data packet,
// else return ptr to the data to send
static void *server_configure_block_properties(int *BN,
		int *BNA)
{
	int block_number;
	int block_number_ack;
	void *ret = NULL;
	int block_index;

	if (server_session->last_in_order == NULL)
		block_number_ack = 0;
	else
		block_number_ack = server_session->last_in_order->block_number;

	block_number = server_session->last_acknowledged + 1;

	if ((server_session->server_send_data == 1) &&
			(server_session->message != NULL)) {
		block_index = block_number - server_session->server_first_data_block + 1;
		ret = server_session->data_blocks[block_index];
	}

	*BN = block_number;
	*BNA = block_number_ack;

	return ret;
}

static void server_send_packets()
{
	int BNA;
	int BN;
	int window;
	int block_control = 0;
	void *send = NULL;
	int i, message_size;
	int block_index;
	int client_window = server_session->client_window;

	window = server_get_window();
	server_session->server_curr_window = window;

	send = server_configure_block_properties(&BN,
			&BNA);

	block_control |= (STR_IS_1 | (window & 0x3f));

	// send no-data packet
	if (send == NULL) {
		block_control |= LB_IS_1; //LB = 1
		block_control &= STR_IS_0; //STR = 0

		server_encode_and_send(block_control,
				BN,
				BNA,
				NULL,
				0);
	} else {
		message_size = server_session->data_size;

		for (i = 1 ; i <= client_window ;  ++i) {
			//case last block to send
			int size_sent = (char *)send -
					(char *)server_session->message;
			int next_size_sent = size_sent + server_session->data_size;

			if (next_size_sent >= server_session->message_size) {
				block_control |= LB_IS_1; //LB = 1
				block_control &= STR_IS_0; //STR = 0
				message_size = server_session->message_size
						- size_sent;

				server_encode_and_send(block_control,
						BN,
						BNA,
						send,
						message_size);

				server_session->sent_all_data = true;
				break;
			} else {
				if (i == client_window) {
					block_control &= STR_IS_0; //STR = 0
					//todo: LB = 1 when
					//you've already send
					//all blocks and this
					//is lost-block-recovery
				}

				server_encode_and_send(block_control,
						BN,
						BNA,
						send,
						message_size);
			}

			++BN;
			block_index = BN - server_session->server_first_data_block + 1;
			send = server_session->data_blocks[block_index];
		}
	}

	if (server_session->highest_BN < BN)
	{
		server_session->highest_BN = BN;
	}

	server_session->packet_sent = 1;
	server_session->blocks_in_current_window = 0;
	server_session->received_last_window_block = 0;
}

static int server_is_BN_valid(int BN)
{
	int valid = GBT_BN_VALID;
	gbt_block_t *BN_ptr = server_session->list;
	int order_BN = 0;

	while (BN_ptr != NULL) {
		if (BN_ptr->block_number == BN) {
				valid = GBT_BN_ALREADY_RECEIVED;
				break;
		}

		BN_ptr = BN_ptr->next;
	}

	return valid;
}

void server_destroy_session()
{
	gbt_block_t *ptr, *tmp;

	free(server_session->data_blocks);
	free(server_session->message);

	ptr = server_session->list;

	while (ptr != NULL)
	{
		tmp = ptr->next;
		free(ptr->data);
		free(ptr);
		ptr = tmp;
	}

	free(server_session);
	server_session = NULL;
}

void *server_compose_message()
{
	char *ret;
	int copy_to = 0;
	gbt_block_t *ptr = server_session->list;

	ret = (char*)malloc(server_session->total_received_size);

	assert(ret != NULL);

	while (ptr != NULL)
	{
		memcpy(ret + copy_to, ptr->data, ptr->data_size);
		copy_to += ptr->data_size;
		ptr = ptr->next;
	}

	return ret;
}

static int server_get_window()
{
	int needed_window = 0;
	int window = server_session->server_max_window;

	if((server_session->last_in_order != NULL && server_session->last_in_order->next == NULL) ||
			(server_session->list == NULL))
	{
		return window;
	}

	needed_window = window;

	if(server_session->list != NULL)
	{
		if(server_session->last_in_order == NULL)
		{
			needed_window = server_session->list->block_number - 1;
		}

		else if (server_session->last_in_order->next != NULL)
		{
			needed_window = server_session->last_in_order->next->block_number - server_session->last_in_order->block_number - 1;
		}

		else
		{
			return window;
		}
	}

	return (needed_window < window ? needed_window : window);
}

static int server_get_size_of_size(int size)
{
	if(size <= 127)
		return 1;
	else if (size <= 255)
		return 2;
	else if (size <= 65535)
		return 3;
	else
		return 4;
}

static int fill_size_asn1(char *exp, int data_size)
{
	if (data_size <= 127)
	{
		exp[6] = data_size;
		return 1;
	}

	else if (data_size <= 255)
	{
		exp[6] = (char)0x81;
		exp[7] = data_size;
		return 2;
	}

	else if (data_size <= 65535)
	{
		exp[6] = (char)0x82;
		exp[7] = data_size >> 8;
		exp[8] = data_size;
		return 3;
	}

	else
	{
		exp[6] = (char)0x83;
		exp[7] = data_size >> 16;
		exp[8] = data_size >> 8;
		exp[9] = data_size;
		return 4;
	}
}

static int server_encode_and_send(int block_control, int BN, int ACK, void *buf, int size)
{
	char *send_buffer = NULL;

	int packet_size = GBT_FRAME_SIZE + server_get_size_of_size(size) + size;

	send_buffer = (char*)calloc(1, packet_size);

	send_buffer[0] = 0xe0;
	send_buffer[1] = block_control;
	send_buffer[2] = BN >> 8;
	send_buffer[3] = BN;
	send_buffer[4] = ACK >> 8;
	send_buffer[5] = ACK;
	int data_offset = fill_size_asn1(send_buffer, size);
	memcpy(send_buffer + 6 + data_offset, buf, size);

	packet_t *ptr;

	ptr = (packet_t*)calloc(1, sizeof(packet_t));
	ptr->data = send_buffer;
	ptr->size = packet_size;
	ptr->next = NULL;

	insert_server_packet(ptr);

	return SUCCESS;
}

void *server_manage_gbt(int is_timeout)
{
	void *ret = NULL;

	if (is_timeout)
	{
		if(!server_session->sent_all_data)
			server_send_packets();
	}

	else
	{
		if (server_session->receive_all_data && !server_session->message_composed)
		{
			ret = server_compose_message();
			server_session->message_composed = 1;
		// todo: start timer
		}

		else
		{
			server_send_packets();
		}
	}

	return ret;
}


static int server_check_message_correctness(int block_control, int BN, int BNA)
{
	int LB = (block_control >> 7) & 0x1;
	int ret = server_is_BN_valid(BN);

	if ((((block_control >> 6) & 0x3) == 0x2) && (BN == 0) && (BNA == 0))
	{
		printf("GBT ABORT: server receive abort request\n");
		return GBT_ABORT;
	}

	return ret;
}

static gbt_block_t *server_create_new_list_block(General_Block_Transfer_t *block)
{
	gbt_block_t *ret = (gbt_block_t*)calloc(1, sizeof(gbt_block_t));

	assert(ret != NULL);

	ret->block_number = block->block_number;
	ret->next = NULL;
	ret->data_size = block->block_data.size;

	if(ret->data_size == 0)
	{
		ret->is_data_block = 0;
		ret->data = NULL;
	}

	else
	{
		ret->data = malloc(ret->data_size);

		assert(ret->data != NULL);

		memcpy(ret->data, block->block_data.buf, ret->data_size);
	}

	return ret;
}

static void server_insert_to_list(gbt_block_t *insert)
{
	gbt_block_t *insert_ptr;
	gbt_block_t *last_ptr = server_session->last_in_order;

	if (server_session->list == NULL)
	{
		server_session->list = insert;

		if(insert->block_number == 1)
			server_session->last_in_order = insert;

		return;
	}

	if(server_session->list->block_number > insert->block_number)
	{
		insert->next = server_session->list;
		server_session->list = insert;

		if(insert->block_number == 1)
		{
			server_session->last_in_order = insert;
			last_ptr = insert;
		}
	}

	else
	{
		if (server_session->last_in_order != NULL)
			insert_ptr = server_session->last_in_order;
		else
			insert_ptr = server_session->list;

		while(insert_ptr->next != NULL)
		{
			if(insert->block_number < insert_ptr->next->block_number)
			{
				break;
			}

			insert_ptr = insert_ptr->next;
		}

		insert->next = insert_ptr->next;
		insert_ptr->next = insert;
	}

	while (last_ptr != NULL && last_ptr->next != NULL && (last_ptr->block_number == last_ptr->next->block_number - 1))
	{
		last_ptr = last_ptr->next;
	}

	server_session->last_in_order = last_ptr;
}

static int server_save_block(General_Block_Transfer_t *block)
{
	gbt_block_t *gbt_block = server_create_new_list_block(block);

	server_insert_to_list(gbt_block);

	server_session->total_received_size += gbt_block->data_size;

	return 0;
}

static int get_asn1_size(void *buf, int *size_of_size)
{
	unsigned char *ptr = (unsigned char*)buf;
	int ret = 0;
	int size_ret;
	int i;

	unsigned int size0 = ptr[6];
	bool size_is_more_than_1 = (size0 & 0x80);

	if (!size_is_more_than_1)
	{
		size_ret = 1;
		ret = size0 & 0x7f;
	}

	else
	{
		size0 &= 0x7f;
		size_ret = size0 + 1;


		for (i = 0 ; i < size0 ; i++)
		{
			int tmp = ptr[7 + i];
			ret |= (tmp & 0xff) << (8*(size0 - i - 1));
		}
	}

	*size_of_size = size_ret;

	return ret;
}

// description: decode the GBT packet to struct. in case the decoding fails it returns NULL
static General_Block_Transfer_t *server_packet_to_struct(void *buf, int size, int *status)
{
	General_Block_Transfer_t *ret;
	unsigned int data_size;
	ret = (General_Block_Transfer_t*)calloc(1, sizeof(General_Block_Transfer_t));
	assert(ret);
	unsigned char *packet = (unsigned char*)buf;

	ret->block_control = packet[1];
	int high_bn = packet[2];
	int low_bn = packet[3];
	ret->block_number = ((high_bn & 0xff) << 8) | (low_bn & 0xff);
	int high_bna = packet[4];
	int low_bna = packet[5];
	ret->block_number_ack = ((high_bna & 0xff) << 8) | (low_bna & 0xff);
	int size_of_size;
	int message_size = get_asn1_size(buf, &size_of_size);
	ret->block_data.size = message_size;

	ret->block_data.buf = (unsigned char*)malloc(message_size);

	memcpy(ret->block_data.buf, packet + 6 + size_of_size, message_size);

	return ret;
}

static void server_init_data_blocks(void *buff, int size)
{
	int num_data_blocks, i;
	int data_size = server_session->data_size;
	char *ptr;

	server_session->message = buff;
	server_session->message_size = size;


	if (size % data_size == 0)
		num_data_blocks = size / data_size;
	else
		num_data_blocks = size / data_size + 1;

	server_session->num_data_blocks = num_data_blocks;
	// allocation is 'num_data_blocks + 1' for every BN
	// will fit the index in the array
	server_session->data_blocks = (void**)calloc(num_data_blocks + 1, sizeof(void *));
	assert(server_session->data_blocks != NULL);

	for (i = 1, ptr = (char*)buff ; i <= num_data_blocks ; ++i) {
		server_session->data_blocks[i] = ptr;
		ptr += data_size;
	}
}


static void server_init_gbt_session(void *buff, int size, gbt_server_info_t *info)
{
	server_session = (server_gbt_session_t*)calloc(1, sizeof(server_gbt_session_t));
	assert(server_session);

	int data_size;

	server_session->message = NULL;
	data_size = info->max_pdu_size - GBT_FRAME_MAX_SIZE;
	server_session->data_size = data_size;
	server_session->num_data_blocks = 0;

	// todo: implement the option when stream is disable -> the client can send only one block at a time
	server_session->server_max_window = info->choice1.params->block_transfer_window;
	server_session->server_curr_window = server_session->server_max_window;
	server_session->client_window = CLIENT_DEFAULT_WINDOW;
	server_session->list = NULL;
	memset(server_session->counters, 0, 6);
	server_session->total_counter = 0;
	server_session->last_in_order = NULL;
	server_session->received_last_block = 0;
	server_session->last_acknowledged = 0;
	server_session->sent_all_data = false;
	server_session->last_data_block_sent = 0;
	server_session->receive_all_data = 0;
	server_session->total_received_size = 0;
	server_session->blocks_in_current_window = 0;
	server_session->highest_expected = 2;
	// have to be initialized with -1
	server_session->last_ACK_sent = -1;

	// todo: decide what is the data size to be sent each time
	server_session->message_size = 0;

	server_session->message_composed = 0;
	server_session->block_received = 0;
	server_session->highest_receive = 0;
	server_session->server_send_data = 0;
	server_session->server_first_data_block = 0;
	server_session->packet_sent = 0;
	server_session->received_last_window_block = 0;
}

void server_print_list_order()
{
	gbt_block_t *ptr = server_session->list;

	printf("server: ");

	while (ptr != NULL)
	{
		printf("%d ->", ptr->block_number);
		ptr = ptr->next;
	}

	printf("\n");
}

int server_handle_gbt(void *message, int size, gbt_server_info_t *info)
{
	General_Block_Transfer_t *gbt_block;
	int status = SUCCESS;
	int all_in_order;
	int ret = GBT_RESPONSE_NOT_COMPLETE;
	void *message_ptr = NULL;

	if (info == NULL)
		return -1;

	if (server_session == NULL)
	{
		server_init_gbt_session(message, size, info);
	}

	if(info->source == GBT_SERVER_SOURCE)
	{
		server_init_data_blocks(message, size);
		server_session->server_send_data = 1;
		server_session->server_first_data_block = server_session->last_acknowledged + 1;
		server_manage_gbt(0);
	}

	else
	{
		gbt_block = server_packet_to_struct(message, size, &status);

		free(message);

		if(gbt_block == NULL)
		{
			return status;
		}

		status = server_check_message_correctness(gbt_block->block_control, gbt_block->block_number, gbt_block->block_number_ack);

		if (status == GBT_ABORT)
		{
			server_destroy_session();
			free(gbt_block->block_data.buf);
			free(gbt_block);
			return -1;
		}

		else if (status == GBT_DROP_PACKET)
		{
			free(gbt_block->block_data.buf);
			free(gbt_block);
			return status;
		}

		server_session->client_window = gbt_block->block_control & WINDOW_MASK;
		server_session->last_acknowledged = gbt_block->block_number_ack;
		int block_control = gbt_block->block_control;

		if(!((server_session->last_data_block_sent <= server_session->last_acknowledged && server_session->sent_all_data) || server_session->message == NULL))
		{
			server_session->enable_new_data_send = 1;
		}

		if (gbt_block->block_number > server_session->highest_receive)
		{
			server_session->highest_receive = gbt_block->block_number;
		}

		server_session->block_received = 1;
		server_session->packet_sent = 0;

		if (status != GBT_BN_ALREADY_RECEIVED)
			server_save_block(gbt_block);

		// in case of no-data-block the size will be 0
		if (gbt_block->block_data.size == 0)
		{
			// todo: kill timer (if exist)
			server_manage_gbt(0);
		}

		else
		{
			all_in_order =
					server_session->last_in_order->next == NULL;
			server_session->received_last_block |=
				(block_control >> 7) & 0x1;
			server_session->received_last_window_block |=
				(((block_control >> 6) & 0x1) == 0);
			server_session->receive_all_data =
					server_session->received_last_block
				&& all_in_order;

			if (server_session->received_last_window_block ||
				server_session->receive_all_data)
			{
				// todo: kill timer
				message_ptr = server_manage_gbt(0);

				if(message_ptr != NULL)
				{
					info->choice1.response_ptr = message_ptr;
					info->response_size = server_session->total_received_size;
					ret = GBT_RESPONSE_COMPLETE;
				}
			}
		}

		free(gbt_block->block_data.buf);
		free(gbt_block);
	}

	if (server_session->packet_sent == 1)
		ret = GBT_PACKET_SENT;

	server_print_list_order();

	return ret;
}

