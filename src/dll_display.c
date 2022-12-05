#include "definitions.h"
#include "dll_display.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <mqueue.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

static mqd_t receiver_queue;
static mqd_t sender_queue;

static struct mq_attr receiver_queue_props;
static struct mq_attr sender_queue_props;

static char* receiver_queue_name = "/receiver_queue";
static char* sender_queue_name = "/sender_queue";

static struct timespec timeout;

void build_queue(struct mq_attr queue_props, mqd_t queue, char *queue_name) {
	queue_props.mq_maxmsg = MSG_MAX_AMOUNT;
	queue_props.mq_msgsize = MSG_MAX_SIZE;

	queue = mq_open(
		queue_name,
		O_CREAT | O_RDWR,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
		&queue_props
	);

	if (queue < 0) {
		printf("[Erro]: Problemas na criacao da fila.\n");
		printf("Erro: %s\n", strerror(errno));
		exit(1);
	}
}

void init_ddl_display_process(long v_timeout) {
	// build_queue(receiver_queue_props, receiver_queue, receiver_queue_name);
	// build_queue(sender_queue_props, sender_queue, sender_queue_name);
	receiver_queue_props.mq_maxmsg = MSG_MAX_AMOUNT;
	receiver_queue_props.mq_msgsize = MSG_MAX_SIZE;

	receiver_queue = mq_open(
		receiver_queue_name,
		O_CREAT | O_RDWR,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
		&receiver_queue_props
	);

	if (receiver_queue < 0) {
		printf("[Erro]: Problemas na criacao da fila.\n");
		printf("Erro: %s\n", strerror(errno));
		exit(1);
	}

	sender_queue_props.mq_maxmsg = MSG_MAX_AMOUNT;
	sender_queue_props.mq_msgsize = MSG_MAX_SIZE;

	sender_queue = mq_open(
		sender_queue_name,
		O_CREAT | O_RDWR,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
		&sender_queue_props
	);

	if (sender_queue < 0) {
		printf("[Erro]: Problemas na criacao da fila.\n");
		printf("Erro: %s\n", strerror(errno));
		exit(1);
	}

	timeout.tv_sec = 0;
	timeout.tv_nsec = v_timeout;
}

void destroy_dll_display() {
	mq_close(sender_queue);
	mq_close(receiver_queue);

	mq_unlink(sender_queue_name);
	mq_unlink(receiver_queue_name);
}

void send_data_msg_to_dll(char* msg, int data_len) {
	if (mq_send(sender_queue, msg, data_len, 0) < 0) {
		printf("[Erro]: Problemas ao enviar dados para o DLL.\n");
		printf("Erro: %s\n", strerror(errno));
	}
}

int get_timed_data_msg_from_dll(char* msg, int* size) {
	int msg_n_priority;
	int bytes_received = mq_timedreceive(receiver_queue, msg, MSG_MAX_SIZE, &msg_n_priority, &timeout);
	
	if (bytes_received < 0) {
		if (errno == ETIMEDOUT)
			return MSG_TIMEOUT;

		printf("[Erro]: Problemas ao receber dados do DLL.\n");
		printf("Erro: %s\n", strerror(errno));

		return errno;
	}

	return 0;
}

void get_data_msg_from_dll(char* msg, int* size) {
	int msg_n_priority;
	int bytes_received = mq_receive(receiver_queue, msg, MSG_MAX_SIZE, &msg_n_priority);
}

void send_data_msg_to_instance(char* msg, int data_len) {
	if (mq_send(receiver_queue, msg, data_len, 0) < 0) {
		printf("[Erro]: Problemas ao enviar dados para a instancia\n");
		printf("Erro: %s\n", strerror(errno));
	}
}

int get_timed_data_msg_from_instance(char* msg, int* size) {
	int msg_n_priority;
	int bytes_received = mq_timedreceive(sender_queue, msg, MSG_MAX_SIZE, &msg_n_priority, &timeout);
	if (bytes_received < 0) {
		if (errno == ETIMEDOUT)
			return MSG_TIMEOUT;

		printf("[Erro]: Problemas ao receber dados da instancia\n");
		printf("Error: %s\n", strerror(errno));

		return errno;
	}

	return 0;
}

void get_data_msg_from_instance(char* msg, int* size) {
	int msg_n_priority;
	int bytes_received = mq_receive(sender_queue, msg, MSG_MAX_SIZE, &msg_n_priority);
}
