#ifndef _DATA_LINK_LAYER_H_
#define _DATA_LINK_LAYER_H_

void initialize_dll(char * host_port, char * receiver_address, char * receiver_port, int t_pdu_size, int t_verbose);
void shut_down_dll();
void run_dll();
void set_verbose_dll(int value);
void set_operation_mode(int value);
static int get_data_from_queue();
static void send_data_to_queue();
static int check_incoming_frame();
static int send_error_confirmation_frame();
static int send_ok_confirmation_frame();
static void send_data();
static void get_data();
static void send_frame_to_receiver();
static int send_frame();
static int get_confirmation_frame();
static int check_confirmation_frame();
static int receive_frame();
static void get_frame_from_sender();
static void pack_message_from_queue_buffer();
static void unpack_message_from_frame_buffer();

#endif