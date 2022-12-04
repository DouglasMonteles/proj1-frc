void init_ws(char *host, char *port, char *receiver_host, char *receiver_port);
void destroy_ws();
int send_data_through_socket(char * data_buffer, int data_buffer_size);
int receive_data_through_socket(char * data_buffer, int data_buffer_size);