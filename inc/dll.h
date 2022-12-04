void init_dll(char *host_port, char *receiver_address, char *receiver_port, int pdu_size_flag, int verbose_flag);
void destroy_dll();
int get_data_from_queue();
void run_dll();
void send_data_to_queue();
int check_incoming_frame();
int send_error_confirmation_frame();
int send_ok_confirmation_frame();
void send_frame_to_receiver();
int send_frame();
int get_confirmation_frame();
int check_confirmation_frame();
int receive_frame();
void pack_message_from_queue_buffer();
void unpack_message_from_frame_buffer();
void set_verbose(int v);
void set_verbose_dll(int v);
void set_op_mode(int m);
void send_msg();
void get_msg();
void get_frame_from_sender();