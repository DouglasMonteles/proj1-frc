#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "main_definitions.h"
#include "I_dll.h"
#include "dll.h"
#include "ws.h"

bool is_ddl_server_running = TRUE;
bool has_msg_to_send = FALSE;
bool has_msg_to_receive = FALSE;
bool is_verbose_enabled = FALSE;

int layer_state;
int operation_mode = 0;

char queue_buffer[MSG_MAX_PIECES];
char* outcoming_frame_buffer;
char* incoming_frame_buffer;

long long incoming_frame_id = 0;
long long outcoming_frame_id = 0;

int buffer_index = 0;
int pdu_size = 0;

// void show_buffer(char *buffer, int buffer_len) {
//   printf("%d[", buffer_len);

//   for (int i = 0; i < buffer_len; i++) {
//     switch (buffer[i]) {
//       case 0xFF:
//         printf("1");
//         break;
//       case 0x0:
//         printf("0");

//       default:
//         printf("%c", buffer[i]);
//         break;
//     }
//   }

//   printf("]\n");
// }

void init_dll(char *host_port, char *receiver_address, char *receiver_port, int aux_pdu_size, int flag_verbose) {
  init_ws(host_port, receiver_address, receiver_port, TIMEOUT_100K);
  init_I_dll(TIMEOUT_1000K);

  pdu_size = MSG_MAX_PIECES + PDU_HEADER;
  
  for (int i = 2; i < MSG_MAX_PIECES; i++) {
    if ((MSG_MAX_PIECES % i) != 0) continue;

    if (abs((i + PDU_HEADER) - aux_pdu_size) < abs((pdu_size + PDU_HEADER) - aux_pdu_size))
      pdu_size = i + PDU_HEADER;
  }

  if (MSG_MAX_PIECES % (pdu_size - PDU_HEADER) != 0) {
    printf("Critical error: queue message size (%d) can't be divided by PDU payload size (%d).\n", MSG_MAX_PIECES, pdu_size - PDU_HEADER);
    printf("Insert a different PDU size.\n");
    exit(1);
  }

  if (pdu_size != aux_pdu_size)
    printf("PDU size was adjusted to %d to avoid inconsistency problems.\n", pdu_size);

  if ((incoming_frame_buffer = malloc((size_t)pdu_size)) == NULL) {
    printf("Unable to allocate memory to incoming frame buffer.\n");
    exit(1);
  }

  if ((outcoming_frame_buffer = malloc((size_t) pdu_size)) == NULL) {
    printf("Unable to allocate memory to outcoming frame buffer.\n");
    exit(1);
  }

  is_verbose_enabled = flag_verbose;
  operation_mode = RECEIVER;
  
  printf("Data Link Layer initialized as %s successfully with PID %d!\n", operation_mode == SENDER ? "SENDER" : "RECEIVER", getpid());

  run_dll();
  destroy_dll();
}

void destroy_dll() {
  destroy_I_dll();
  destroy_ws();

  free(incoming_frame_buffer);
  free(outcoming_frame_buffer);
}

void run_dll() {
  while (is_ddl_server_running) {
    if (operation_mode == SENDER) {
      if (get_data_from_queue() == TIMEOUT) {
        operation_mode = RECEIVER;
        continue;
      }
      send_msg();
    }

    if (operation_mode == RECEIVER) {
      while (buffer_index < (BUFFER_SIZE - 1)) {
        int frame_status;

        while (frame_status = receive_frame()) {
          if (frame_status == SOCKET_TIMEOUT) {
            operation_mode = SENDER;
            break;
          }

          printf("Failed to receive frame %lld. Trying again...\n", incoming_frame_id);
        }

        if (operation_mode == SENDER) break;

        get_frame_from_sender();
        unpack_message_from_frame_buffer();
      }
      
      buffer_index = 0;

      if (operation_mode == RECEIVER) 
        send_data_to_queue();
    }
  }
}

void set_verbose_dll(int v) {
  is_verbose_enabled = v;
}

void set_operation_mode(int value) {
  if (value != RECEIVER && value != SENDER) {
    printf("Attempt to set operation mode with invalid value: %d.\n", value);
    return;
  }

  operation_mode = value;

  if (is_verbose_enabled)
    printf("Now operating as %s.\n", operation_mode == RECEIVER ? "RECEIVER" : "SENDER");
}

int get_data_from_queue() {
  int nothing;
  return obtain_timed_msg_from_instance(queue_buffer, &nothing);
}

void get_frame_from_queue() {
  int nothing;
  obtain_msg_from_instance(queue_buffer, &nothing);
  // show_buffer(queue_buffer, MSG_MAX_PIECES);
}

void send_data_to_queue() {
  //show_buffer(queue_buffer, MSG_MAX_PIECES);
  send_msg_to_instance(queue_buffer, BUFFER_SIZE);
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
  memset(outcoming_frame_buffer, 0xFF, pdu_size);
  return send_frame();
}

int send_ok_confirmation_frame() {
  memset(outcoming_frame_buffer, 0x0, pdu_size);
  return send_frame();
}

void send_msg() {
  if (is_verbose_enabled)
    printf("Sending data from queue.\n");

  while (buffer_index < BUFFER_SIZE - 1) {
    pack_message_from_queue_buffer();
    send_frame_to_receiver();
  }

  buffer_index = 0;

  if (is_verbose_enabled)
    printf("Data from queue sent successfully.\n\n");
}

void get_msg() {
  if (is_verbose_enabled)
    printf("Receiving message data.\n");

  if (is_verbose_enabled && operation_mode == RECEIVER)
    printf("Message data received successfully.\n\n");
}

void send_frame_to_receiver() {
  int attempts = 0;
    while (1) {
      if (is_verbose_enabled)
        printf("Sending frame %lld.\n", outcoming_frame_id);

      while (send_frame()) {
        printf("Failed to send frame %lld. Trying again...\n", outcoming_frame_id);
      }

      while (get_confirmation_frame() && attempts <= 3) {
        printf("Failed to receive confirmation frame. Trying again...\n");
        attempts++;
      }

      if (attempts > 3) {
        printf("Resending frame due confirmation failure.\n");
        attempts = 0;
        continue;
      }

      if (check_confirmation_frame())
        continue;

      break;
    }
}

int send_frame() {
  return send_data_through_socket(outcoming_frame_buffer, pdu_size);
}

int get_confirmation_frame() {
  return receive_frame();
}

int check_confirmation_frame() {
  int is_error = 0;
  for (int i = 0; i < pdu_size; i++) {
    if (incoming_frame_buffer[i] == 0xFF)
      is_error = 1;
  }

  if (is_error) {
    printf("Ocorreu um erro no frame\n");
  }
  return is_error;
}

int receive_frame() {
  int result = receive_data_through_socket(incoming_frame_buffer, pdu_size);
  return result;
}

void get_frame_from_sender() {
  // Caso o frame recebido esteja corrompido, tentar enviar mensage de erro para o par
  if (is_verbose_enabled)
    printf("Receiving frame %lld.\n", incoming_frame_id);

  if (check_incoming_frame())
    while (send_error_confirmation_frame())
      printf("Failed to send ERROR confirmation frame. Trying again...\n");

  // Caso o frame recebido esteja ok, tentar enviar mensagem de ok para o par
  else
    while (send_ok_confirmation_frame())
      printf("Failed to send OK confirmation frame. Trying again...\n");
}

void pack_message_from_queue_buffer() {
  *((long long *)outcoming_frame_buffer) = outcoming_frame_id++;
  for (int i = 0; i < pdu_size - PDU_HEADER; i++)
    outcoming_frame_buffer[i + PDU_HEADER] = queue_buffer[buffer_index++];
}

void unpack_message_from_frame_buffer() {
  for (int i = 0; i < pdu_size - PDU_HEADER; i++)
    queue_buffer[buffer_index++] = incoming_frame_buffer[i + PDU_HEADER];
}