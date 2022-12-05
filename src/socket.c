#include "definitions.h"
#include "socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>

struct sockaddr_in received_msg;
struct sockaddr_in host_props;
static struct timeval timeout;

int socket_msg_body;

void set_props(char *h_port, char *host_receiver, char * h_receiver_post) {
	received_msg.sin_family = AF_INET;
	received_msg.sin_addr.s_addr = inet_addr(host_receiver);
	received_msg.sin_port = htons(atoi(h_receiver_post));

	host_props.sin_family = AF_INET;
	host_props.sin_addr.s_addr = htonl(INADDR_ANY);
	host_props.sin_port = htons(atoi(h_port));

	socket_msg_body = socket(AF_INET, SOCK_DGRAM, 0);
}

void init_socket(char *h_port, char *host_receiver, char * h_receiver_post, long m_timeout) {
	set_props(h_port, host_receiver,  h_receiver_post);
	
	if (socket_msg_body < 0) {
		printf("Nao foi possivel abrir uma conexao com o socket :(\n");
		exit(1);
	}

	if (bind(socket_msg_body, (struct sockaddr *) &host_props, sizeof(host_props)) < 0) {
		printf("Nao foi possivel vincular o socket a porta fornecida %d\nTende utilizar outra porta!\n", ntohs(host_props.sin_port));
		exit(1);
	}

	timeout.tv_sec = 0;
	timeout.tv_usec = m_timeout;

	if (setsockopt(socket_msg_body, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
		printf("Ocorreu um erro ao definir um timeout para o socket.\n Erro: %s\n", strerror(errno));
	}

	printf("Conexao utilizando o socket foi estabelecida com sucesso!\n");
	
	printf(
		"{UDP, IP: %s, PORTA: %u, END. DO RECEPTOR: %s, PORTA: %s\n",
		inet_ntoa(host_props.sin_addr),
		ntohs(host_props.sin_port),
		host_receiver,
		h_receiver_post
	);
}

void destroy_socket() {
	close(socket_msg_body);
}

int send_msg_through_socket(char *msg_buffer, int msg_size) {
	int bytes_sended = sendto(socket_msg_body, msg_buffer, msg_size, 0,(struct sockaddr *) &received_msg, sizeof(received_msg));
	return bytes_sended != msg_size;
}

int receive_msg_through_socket(char * msg_buffer, int msg_size) {
	int received_msg_size = sizeof(received_msg);
	int bytes_reader = recvfrom(socket_msg_body, msg_buffer, msg_size, 0, (struct sockaddr *) &received_msg, &received_msg_size);

	if (bytes_reader == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return SOCKET_TIMEOUT;
	}

	return bytes_reader != msg_size;
}