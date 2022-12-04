#include <stdio.h>
#include <string.h>

#include "main_definitions.h"
#include "I_dll.h"
#include "dll.h"
#include "ws.h"

int layer_state;
int operation_mode;

char queue_buffer[MSG_MAX_PIECES];
char incoming_frame_buffer[PDU_SIZE];
char outcoming_frame_buffer[PDU_SIZE];

long long incoming_frame_id = 0;
long long outcoming_frame_id = 0;

void show_buffer(char *buffer, int buffer_len) {
  for (int i = 0; i < buffer_len; i++)
    printf("%c", buffer[i]);
}

void init_dll(int t_operation_mode, char *host_address, char *host_port, char *receiver_address, char *receiver_port) {
  init_I_dll();
  init_ws(host_address, host_port, receiver_address, receiver_port);
  operation_mode = t_operation_mode;
  printf("DLL inicializado com sucesso! -> %s\n", operation_mode == SENDER ? "SENDER" : "RECEIVER");
}

void destroy_dll() {
  destroy_I_dll();
  destroy_ws();
}

void run_dll() {
  while (operation_mode == SENDER) {
    get_data_from_queue();
    pack_message_from_queue_buffer();
    delivery_frame();
    //printf("\nSending frame ID: %lld\n", outcoming_frame_id);
  }

  while (operation_mode == RECEIVER) {
    get_data_from_sender();
    unpack_message_from_frame_buffer();
    send_data_to_queue();
    // printf("Receiving frame ID: \n%lld\n", incoming_frame_id);
  }
}

void get_data_from_queue() {
  int nothing;
  obtain_msg_from_instance(queue_buffer, &nothing);
  // show_buffer(queue_buffer, MSG_MAX_PIECES);
}

void send_data_to_queue() {
  //show_buffer(queue_buffer, MSG_MAX_PIECES);
  send_msg_to_instance(queue_buffer, MSG_MAX_PIECES);
}

int check_incoming_frame() {
  long long received_id = *((long long *)incoming_frame_buffer);
  if (received_id != incoming_frame_id) {
    printf("Id os pacotes sao diferentes do esperado! Esperado: %lld, Recebido: %lld\n", incoming_frame_id, received_id);
    return 1;
  }

  incoming_frame_id++;
  return 0;
}

int send_error_confirmation_frame() {
  memset(outcoming_frame_buffer, 0xFF, PDU_SIZE);
  return send_frame();
}

int send_ok_confirmation_frame() {
  memset(outcoming_frame_buffer, 0x0, PDU_SIZE);
  return send_frame();
}

void delivery_frame() {
  do {
    while (send_frame()) {
      printf("Tentativa de envio de frame falhou. Tentando novamente...\n");
    }
    
    while (get_confirmation_frame()) {
      printf("Tentativa de recebimento falhou. Tentando novamente......\n");
    }
  } while (check_confirmation_frame());
}

int send_frame() {
  return send_data_through_socket(outcoming_frame_buffer, PDU_SIZE);
}

int get_confirmation_frame() {
  return receive_data_through_socket(incoming_frame_buffer, PDU_SIZE);
}

int check_confirmation_frame() {
  int is_error = 0;
  for (int i = 0; i < PDU_SIZE; i++) {
    if (incoming_frame_buffer[i] == 0xFF)
      is_error = 1;
  }

  if (is_error) {
    printf("Ocorreu um erro no frame\n");
  }
  return is_error;
}

int receive_frame() {
  return receive_data_through_socket(incoming_frame_buffer, PDU_SIZE);
}

void get_data_from_sender() {
  while (receive_frame())
    ;
  // Caso o frame recebido esteja corrompido, tentar enviar mensage de erro para o par
  if (check_incoming_frame())
    while (send_error_confirmation_frame())
      ;

  // Caso o frame recebido esteja ok, tentar enviar mensagem de ok para o par
  else
    while (send_ok_confirmation_frame())
      ;
}

void pack_message_from_queue_buffer() {
  *((long long *)outcoming_frame_buffer) = outcoming_frame_id++;
  for (int i = PDU_HEADER; i < PDU_SIZE; i++)
    outcoming_frame_buffer[i] = queue_buffer[i - PDU_HEADER];
}

void unpack_message_from_frame_buffer() {
  for (int i = PDU_HEADER; i < PDU_SIZE; i++)
    queue_buffer[i - PDU_HEADER] = incoming_frame_buffer[i];
}