#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#include "main_definitions.h"

int socket_data;

struct sockaddr_in receiver_data;
struct sockaddr_in host_data;

void init_ws(char *host, char *port, char *receiver_host, char *receiver_port) {
  receiver_data.sin_family = AF_INET;
  receiver_data.sin_addr.s_addr = inet_addr(receiver_host);
  receiver_data.sin_port = htons(atoi(receiver_port));

  host_data.sin_family = AF_INET;
  host_data.sin_addr.s_addr = inet_addr(host);
  host_data.sin_port = htons(atoi(port));

  socket_data = socket(AF_INET, SOCK_DGRAM, 0);

  if (socket_data < 0) {
    l("Não foi possivel abrir uma conexao web socket");
    exit(1);
  }

  if (bind(socket_data, (struct sockaddr *) &host_data, sizeof(host_data)) < 0) {
    printf("Nao foi possivel vincular o host a porta: %d\nExiting...\n", ntohs(host_data.sin_port));
    exit(1);
  }

  l("Socket inicializado com sucesso!\n");

  printf("{UDP, Host IP: %s, Host port: %u, Receiver IP: %s, Receiver port: %u\n",
    inet_ntoa(host_data.sin_addr),
    ntohs(host_data.sin_port),
    inet_ntoa(receiver_data.sin_addr),
    ntohs(receiver_data.sin_port)
  );
}

void destroy_ws() {
  close(socket_data);
}

int send_data_through_socket(char * data_buffer, int data_buffer_size) {
  int bytes_sent_amount = sendto(socket_data, data_buffer, data_buffer_size, 0,(struct sockaddr *) &receiver_data, sizeof(receiver_data));
  return bytes_sent_amount != data_buffer_size;
}

int receive_data_through_socket(char * data_buffer, int data_buffer_size) {
  int receiver_data_size = sizeof(receiver_data);
  int bytes_read_amount = recvfrom(socket_data, data_buffer, data_buffer_size, 0, (struct sockaddr *) &receiver_data, &receiver_data_size);
  return bytes_read_amount != data_buffer_size;
}