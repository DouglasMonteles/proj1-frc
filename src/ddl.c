#include "definitions.h"
#include "dll.h"
#include "socket.h"
#include "dll_display.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

static int operation_mode;
static const int QUEUE_BUFFER_SIZE = MSG_MAX_SIZE;
static int queue_buffer_index = 0;
static char queue_buffer[MSG_MAX_SIZE];
static int pdu_size;
static char * data_incoming_frame_buffer;
static long long incoming_frame_id = 0;
static char * data_outcoming_frame_buffer;
static long long outcoming_frame_id = 0l;
static int verbose = 0;
static int dll_is_running = 1;

void init_dll_process(char* port, char* receiver_host, char* receiver_port, int flag_pdu_size) {
	init_socket(port, receiver_host, receiver_port, TIMEOUT_2);
	init_ddl_display_process(TIMEOUT_1);

	pdu_size = (MSG_MAX_SIZE + PDU_HEADER_SIZE);
	operation_mode = ID_RECEIVER;

	for (int i = 2; i < MSG_MAX_SIZE; i++) {
		if (MSG_MAX_SIZE % i != 0) continue;

		if (abs((i + PDU_HEADER_SIZE) - flag_pdu_size) < abs((pdu_size + PDU_HEADER_SIZE) - flag_pdu_size))
			pdu_size = i + PDU_HEADER_SIZE;
	}

	if (MSG_MAX_SIZE % (pdu_size - PDU_HEADER_SIZE) != 0) {
		printf("[Erro]: O tamanho da fila de msg (%d) nao pode ser divido pelo tamanho da PDU informado (%d).\n", MSG_MAX_SIZE, pdu_size - PDU_HEADER_SIZE);
		printf("[Dica]: Informe outro tamanho de PDU.\n");
		exit(1);
	}

	if (pdu_size != flag_pdu_size)
		printf("[Aviso]: O tamanho da PDU foi ajustado, pois o calculo da PDU retornou [%d] como valor para evitar problemas de inconsistencia.\n", pdu_size);

	if ((data_incoming_frame_buffer = malloc((size_t) pdu_size)) == NULL) {
		printf("[Erro]: Nao foi possivel alocar memoria para o buffer.\n");
		exit(1);
	}

	if ((data_outcoming_frame_buffer = malloc((size_t) pdu_size)) == NULL) {
		printf("[Erro]: Nao foi possivel alocar memoria para o buffer.\n");
		exit(1);
	}

	printf("Camada de Enlace de Dados (DLL) foi inicializada. [%s] - PID %d!\n", operation_mode == ID_SENDER ? "ID_SENDER" : "ID_RECEIVER", getpid());

	execute_dll_process();
	destroy_dll_process();
}

void destroy_dll_process() {
	free(data_incoming_frame_buffer);
	free(data_outcoming_frame_buffer);

	destroy_dll_display();
	destroy_socket();
}

void execute_dll_process() {
	while (dll_is_running) {
		if (operation_mode == ID_SENDER) {
			if (get_msg_data_from_queue() == MSG_TIMEOUT) {
				operation_mode = ID_RECEIVER;
				continue;
			}

			send_msg_data();
		}

		if (operation_mode == ID_RECEIVER) {
			while (queue_buffer_index < QUEUE_BUFFER_SIZE - 1) {
				int frame_status;
				while (frame_status = receive_frame()) {
					if (frame_status == SOCKET_TIMEOUT) {
						operation_mode = ID_SENDER;
						break;
					}

					printf("[Erro]: Frame nao recebido %lld. Tentando obte-lo novamente...\n", incoming_frame_id);
				}

				if (operation_mode == ID_SENDER) break;

				get_frame_from_sender();
				unpack_message_from_frame_buffer();
			}

			queue_buffer_index = 0;

			if (operation_mode == ID_RECEIVER)
				send_msg_data_to_queue();
		}
	}
}

void define_op_mode(int value) {
	if (value != ID_RECEIVER && value != ID_SENDER) {
		printf("Informe um modo de operacao valido! %d.\n", value);
		return;
	}
	operation_mode = value;
}

int get_msg_data_from_queue() {
	int trash;
	return get_timed_data_msg_from_instance(queue_buffer, &trash);
}

void send_msg_data_to_queue() {
	send_data_msg_to_instance(queue_buffer, QUEUE_BUFFER_SIZE);
}

int verify_incoming_frame() {
	long long received_id = *((long long *)data_incoming_frame_buffer);
	if (received_id != incoming_frame_id) {
		printf("[Erro]: O Id dos pacotes estao diferentes do esperado. Expectativa: %lld, Realidade: %lld\n", incoming_frame_id, received_id);
		return 1;
	}

	incoming_frame_id++;
	return 0;
}

int send_status_error_confirmation_frame() {
	memset(data_outcoming_frame_buffer, 0xFF, pdu_size);
	return send_frame();
}

int send_success_confirmation_frame() {
	memset(data_outcoming_frame_buffer, 0x0, pdu_size);
	return send_frame();
}

void send_msg_data() {
	printf("Enviando dados da fila...\n");

	while (queue_buffer_index < QUEUE_BUFFER_SIZE - 1) {
		pack_message_from_queue_buffer();
		send_frame_msg_to_receiver();
	}

	queue_buffer_index = 0;

	printf("Processo de envio de dados finalizado com sucesso!\n\n");
}

void send_frame_msg_to_receiver() {
	int num_attempts = 0;

	while (1) {
		printf("[Info]: Enviando frame %lld.\n", outcoming_frame_id);

		while (send_frame())
			printf("[Erro]: Falha ao enviar o frame %lld. Tentando novamente...\n", outcoming_frame_id);

		while (get_info_confirmation_frame() && num_attempts <= 3) {
			printf("[Erro]: Falha ao receber a confirmação do frame. Tentando novamente...\n");
			num_attempts++;
		}

		if (num_attempts > 3) {
			printf("Re-enviando frame...\n");
			num_attempts = 0;
			continue;
		}

		if (check_info_confirmation_frame()) continue;

		break;
	}
}

int send_frame() {
  return send_msg_through_socket(data_outcoming_frame_buffer, pdu_size);
}

int get_info_confirmation_frame() {
  receive_frame();
}

int check_info_confirmation_frame() {
	int has_error = 0;

	for (int i = 0; i < pdu_size; i++) {
		if (data_incoming_frame_buffer[i] == 0xFF)
			has_error = 1;
	}

	if (has_error)
		printf("A confirmacao do frame informou um erro.\n");

	return has_error;
}

int receive_frame() {
	int response = receive_msg_through_socket(data_incoming_frame_buffer, pdu_size);
	return response;
}

void get_frame_from_sender() {
  printf("Recebendo o frame %lld.\n", incoming_frame_id);

	if (verify_incoming_frame()) {
		while (send_status_error_confirmation_frame())
			printf("Erro no envio da confirmacao do frame. Tentando novamente...\n");

	} else {
		while (send_success_confirmation_frame())
			printf("Erro no envio da confirmacao do frame. Tentando novamente...\n");
	}
}

void pack_message_from_queue_buffer() {
	*((long long *) data_outcoming_frame_buffer) = outcoming_frame_id++;
	for (int i = 0; i < pdu_size - PDU_HEADER_SIZE; i++)
		data_outcoming_frame_buffer[i + PDU_HEADER_SIZE] = queue_buffer[queue_buffer_index++];
}

void unpack_message_from_frame_buffer() {
	for (int i = 0; i < pdu_size - PDU_HEADER_SIZE; i++)
		queue_buffer[queue_buffer_index++] = data_incoming_frame_buffer[i + PDU_HEADER_SIZE];
}