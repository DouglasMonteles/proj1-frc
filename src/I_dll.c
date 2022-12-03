#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <string.h>
#include <sys/stat.h>

#include "main_definitions.h"

mqd_t send_queue;
mqd_t receive_queue;

struct mq_attr send_queue_props;
struct mq_attr receive_queue_props;

char* send_queue_name = "/send_queue_name";
char* receive_queue_name = "/receive_queue_name";

void init_I_dll() {
  send_queue_props.mq_maxmsg = MSG_MAX_AMOUNT;
  send_queue_props.mq_msgsize = MSG_MAX_PIECES;

  send_queue = mq_open(
    send_queue_name,
    O_CREAT | O_RDWR,
    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
    &send_queue_props
  );

  if (send_queue < 0) {
    printf("Nao foi possivel criar a fila de envios\n");
    exit(1);
  }

  receive_queue_props.mq_maxmsg = MSG_MAX_AMOUNT;
  receive_queue_props.mq_msgsize = MSG_MAX_PIECES;

  receive_queue = mq_open(
    receive_queue_name,
    O_CREAT | O_RDWR,
    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
    &receive_queue_props
  );

  if (receive_queue < 0) {
    printf("Não foi possivel criar a fila de recebimento\n");
    exit(1);
  }
}

void destroy_I_dll() {
  mq_close(receive_queue);
  mq_close(send_queue);
  
  mq_unlink(receive_queue_name);
  mq_unlink(send_queue_name);
}

void send_msg_to_dll(char *msg, int size) {
  if (mq_send(receive_queue,msg, size, 0) < 0) {
    printf("Nao foi possivel enviar a mensagem para o dll :(\n");
  }
}

void obtain_msg_from_dll(char *msg, int * size) {
  int priority;
  int bytes_received = mq_receive(send_queue,msg, MSG_MAX_PIECES, &priority);

  if (bytes_received < 0) {
    printf("Nao foi possivel receber a mensagem do dll\n");
    exit(1);
  }

  *size = bytes_received;
}

void send_msg_to_instance(char *msg, int size) {
  if (mq_send(send_queue, msg, size, 0) < 0) {
    printf("Nao foi possivel enviar uma mensagem para a instancia\n");
  }
}

void obtain_msg_from_instance(char *msg, int *size) {
  int priority;
  int bytes_received = mq_receive(receive_queue, msg, MSG_MAX_PIECES, &priority);
  if (bytes_received < 0) {
    printf("Nao foi possivel receber a mensagem da instancia\n");
    exit(1);
  }

  *size = bytes_received;
}