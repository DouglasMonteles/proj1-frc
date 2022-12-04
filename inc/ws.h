#ifndef _SOCKET_CONNECTION_H_
#define _SOCKET_CONNECTION_H_

#define SC_TIMEOUT 5

void initialize_socket(char * host_port, char * receiver_address, char * receiver_port, long micro_timeout);
void shut_down_socket();

int send_data_through_socket(char * data_buffer, int data_buffer_size);

// Tenta receber dados via socket, retorna SC_TIMEOUT em timeout e 0 em sucesso
int receive_data_through_socket(char * data_buffer, int data_buffer_size);

#endif