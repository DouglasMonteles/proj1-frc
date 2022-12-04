#include "definitions.h"
#include "socket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h> /* memset() */
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>

static int socket_data;

// VARIAVEIS PARA SOQUETE
static struct sockaddr_in receiver_data;
static struct sockaddr_in host_data;

static struct timeval timeout;

void initialize_socket(char * host_port, char * receiver_address, char * receiver_port, long micro_timeout)
{
    receiver_data.sin_family = AF_INET;
    receiver_data.sin_addr.s_addr = inet_addr(receiver_address);
    receiver_data.sin_port = htons(atoi(receiver_port));

    host_data.sin_family = AF_INET;
    host_data.sin_addr.s_addr = htonl(INADDR_ANY);
    host_data.sin_port = htons(atoi(host_port));

    socket_data = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_data < 0) {
        printf("Unable to open socket\nExiting...\n");
        printf("Error: %s\n", strerror(errno));
        exit(1);
    }

    if (bind(socket_data, (struct sockaddr *) &host_data, sizeof(host_data)) < 0) {
        printf("Unable to bind socket to host port %d\nExiting...\n", ntohs(host_data.sin_port));
        printf("Error: %s\n", strerror(errno));
        exit(1);
    }

    timeout.tv_sec = 0;
    timeout.tv_usec = micro_timeout;

    if (setsockopt(socket_data, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        printf("An error occurred when trying to set socket timeout.\n Error: %s\n", strerror(errno));
    }

    printf("Socket initialized successfully\n");
    printf(
        "{UDP, Host IP: %s, Host port: %u, Receiver IP: %s, Receiver port: %s\n",
        inet_ntoa(host_data.sin_addr),
        ntohs(host_data.sin_port),
        receiver_address,
        receiver_port
    );

}

void shut_down_socket()
{
    close(socket_data);
}

int send_data_through_socket(char * data_buffer, int data_buffer_size)
{
    int bytes_sent_amount = sendto(socket_data, data_buffer, data_buffer_size, 0,(struct sockaddr *) &receiver_data, sizeof(receiver_data));
    return bytes_sent_amount != data_buffer_size;
}

int receive_data_through_socket(char * data_buffer, int data_buffer_size)
{
    int receiver_data_size = sizeof(receiver_data);
    int bytes_read_amount = recvfrom(socket_data, data_buffer, data_buffer_size, 0, (struct sockaddr *) &receiver_data, &receiver_data_size);

    if (bytes_read_amount == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return SC_TIMEOUT;
    }

    return bytes_read_amount != data_buffer_size;
}