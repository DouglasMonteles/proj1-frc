void init_socket(char *port, char* receiver_host, char* receiver_port, long m_timeout);
void destroy_socket();
int send_msg_through_socket(char *msg_buffer, int size);
int receive_msg_through_socket(char *msg_buffer, int size);
