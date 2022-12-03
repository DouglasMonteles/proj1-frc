void init_dll(int t_operation_mode, char *host_address, char *host_port, char *receiver_address, char *receiver_port);
void destroy_dll();
void get_data_from_queue();
void run_dll();
void send_data_to_queue();
int check_incoming_frame();
int send_error_confirmation_frame();
int send_ok_confirmation_frame();
void delivery_frame();
int send_frame();
int get_confirmation_frame();
int check_confirmation_frame();
int receive_frame();
void get_data_from_sender();
void pack_message_from_queue_buffer();
void unpack_message_from_frame_buffer();