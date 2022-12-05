#ifndef _DLL_H_
#define _DLL_H_

void init_dll_process(char *port, char *receiver_host, char *receiver_port, int flag_pdu_size);
void destroy_dll_process();
void execute_dll_process();
void define_op_mode(int value);
int get_msg_data_from_queue();
void send_msg_data_to_queue();
int verify_incoming_frame();
int send_status_error_confirmation_frame();
int send_success_confirmation_frame();
void send_msg_data();
void get_msg_data();
void send_frame_msg_to_receiver();
int send_frame();
int get_info_confirmation_frame();
int check_info_confirmation_frame();
int receive_frame();
void get_frame_from_sender();
void pack_message_from_queue_buffer();
void unpack_message_from_frame_buffer();

#endif
