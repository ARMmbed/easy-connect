

#ifndef SERVER_GBT_H_
#define SERVER_GBT_H_

#define GBT_CLIENT_SOURCE 0
#define GBT_SERVER_SOURCE 1

#define GBT_RESPONSE_COMPLETE 1
#define GBT_RESPONSE_NOT_COMPLETE 2

#define GBT_SERVER_DROP_RECEIVED_BLOCKS -2
#define GBT_SERVER_DROP_SENT_BLOCKS -3
#define GBT_SERVER_DROP_RECEIVED_AND_SENT_BLOCKS -4

#define GBT_FRAME_SIZE 6

enum error_counters {
	SAME_BLOCK_TWICE = 0,
	BN_TOO_HIGH = 1,
	NON_LOGICAL = 2,
	ACK_TOO_LOW = 3,
	FAILED_DECODING = 4,
	TIMEOUT = 5,
	NUM_ERRORS
};

typedef struct gbt_block {
	int block_number;
	int data_size;
	int is_data_block;
	void *data;

	struct gbt_block *next;
} gbt_block_t;

typedef struct Request_general_block_transfer_parameters {
	char	 block_transfer_streaming;
	char	 block_transfer_window;

	/* Context for parsing across buffer boundaries */
} Request_general_block_transfer_parameters_t;

typedef struct packet
{
	int size;
	void *data;
	struct packet *next;
}packet_t;

typedef struct gbt_server_info
{
	union server_info_transfer
	{
		struct Request_general_block_transfer_parameters *params;
		void *response_ptr;
		int failure_type;
	} choice1;

	short response_size;
	short source;
	int max_pdu_size;
} gbt_server_info_t;

typedef struct server_gbt_session
{
	int entry;
	int receive_all_data;
	int blocks_in_current_window;
	int last_acknowledged;
	int lost_counter;
	int received_last_block;
	int highest_expected;
	char counters[NUM_ERRORS];
	int total_counter;

	bool sent_all_data;
	int last_data_block_sent;

	int is_streaming_enable_client;
	int server_max_window;
	int server_curr_window;
	int client_window;
	int message_size;
	int num_data_blocks;
	void **data_blocks;
	void *message;
	int blocks_to_send;
	int all_sent_acknowledged;
	int highest_BN;
	int last_ACK_sent;
	int message_composed;
	int blocks_sent_size;
	int data_size;
	int server_send_data;
	int server_first_data_block;

	int total_received_size;
	gbt_block_t *last_in_order;
	gbt_block_t *list;

	int block_received;
	int enable_new_data_send;
	int highest_receive;
	int packet_sent;
	int received_last_window_block;

} server_gbt_session_t;

packet_t *get_next_packet();
int server_handle_gbt(void *message, int size, gbt_server_info_t *info);
void server_destroy_session();
bool is_gbt_active();
void set_gbt_unactive();
bool sent_all_data();
int get_last_in_order();

#endif /* SERVER_GBT_H_ */
