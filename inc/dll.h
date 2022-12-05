#ifndef _DATA_LINK_LAYER_H_
#define _DATA_LINK_LAYER_H_

void init_dll_process(char *port, char *receiver_host, char *receiver_port, int flag_pdu_size);
void destroy_dll_process();
void execute_dll_process();
void define_op_mode(int value);
static int get_msg_data_from_queue();
static void send_msg_data_to_queue();
static int verify_incoming_frame();
static int send_status_error_confirmation_frame();
static int send_success_confirmation_frame();
static void send_msg_data();
static void get_msg_data();
static void send_frame_msg_to_receiver();
static int send_frame();
static int get_info_confirmation_frame();
static int check_info_confirmation_frame();
static int receive_frame();
static void get_frame_from_sender();
static void pack_message_from_queue_buffer();
static void unpack_message_from_frame_buffer();

#endif