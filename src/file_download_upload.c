#include "definitions.h"
#include "dll_display.h"
#include "file_download_upload.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

#define MAX 300

int selected_option;
char commands[MAX];
int is_app_running = 1;

void options_menu() {
	printf("\nEnvie e receba arquivos sem precisar ir Catar com vários Pendrives\n\n");
	printf("--------------- MENU --------------\n");
	printf("[1] - Enviar seus arquivos\n");
	printf("[2] - Receber arquivos\n");
	printf("[3] - Finalizar a aplicação\n");
	printf("---------------------------------\n\n");

	scanf(" %s", commands);

	switch (commands[0]) 	{
		case '1':
			selected_option = OPC_SEND_FILE;
			break;

		case '2':
			selected_option = OPC_RECEIVE_FILE;
			break;

		case '3':
			is_app_running = 0;
			break;
		
		default:
			break;
	}	
}

void init_send_files() {
	printf("Informe o nome do arquivo a ser enviado. Ex: teste.txt\n");
	scanf(" %s", commands);
	if (upload_file(commands) == 0)
		printf("Seu arquivo <%s> foi enviado!\n", commands);
	else
		printf("Erro ao enviar arquivo <%s>\n", commands);

	// printf("\033[0;33mDigite algo para continuar\033[0m.\n");
	// scanf(" %s", commands);
	selected_option = OPC_MENU;
}

void init_receive_files() {
	download_file();
	printf("Arquivo foi recebido com sucesso!\n");
	// printf("\033[0;33mDigite algo para continuar\033[0m.\n");
	// scanf(" %s", commands);
	selected_option = OPC_MENU;
}

void init_process() {
	while (is_app_running) {
		switch (selected_option) 		{
		case OPC_MENU:
			options_menu();
			break;

		case OPC_SEND_FILE:
			init_send_files();
			break;

		case OPC_RECEIVE_FILE:
			init_receive_files();
			break;

		default:
			break;
		}
	}
}

int upload_file(char *file_name) {
	FILE *file;
	file = fopen(file_name, "r");

	if (file == NULL) {
		printf("Ocorreu um erro: %s\n", strerror(errno));
		selected_option = OPC_MENU;
		return 1;
	}

	int file_size_in_bytes = 0;

	fseek(file, 0L, SEEK_END);
	file_size_in_bytes = ftell(file);

	rewind(file);

	char msg_data_chunks[MSG_MAX_SIZE];

	int bytes_read;
	int bytes_sended = 0;
	long long int packages_sended = 0;

	while (bytes_read = fread(msg_data_chunks + MSG_HEADER_SIZE, sizeof(char), MSG_SIZE, file))	{
		*((int *)msg_data_chunks) = bytes_read;

		send_data_to_dll(msg_data_chunks, MSG_MAX_SIZE);
		bytes_sended += bytes_read;
		packages_sended++;
		if (packages_sended % 100 == 0)
			printf("Upload em andamento: [%d / %d]\n", bytes_sended, file_size_in_bytes);
	}

	memset(msg_data_chunks, 0x0, MSG_MAX_SIZE);
	send_data_to_dll(msg_data_chunks, MSG_MAX_SIZE);

	fclose(file);
	return 0;
}

void download_file() {
	char file_name[MAX];

	int msg_data_chunk_len;
	int msg_bytes_received = 0;

	printf("Informe o nome do arquivo que será recebido:\n");
	scanf(" %s", file_name);

	FILE *file;
	file = fopen(file_name, "w");

	char msg_chunk_data[MSG_MAX_SIZE];

	while (1)	{
		get_data_from_dll(msg_chunk_data, &msg_data_chunk_len);

		int useful_msg_len = *((int *)msg_chunk_data);
		if (useful_msg_len == 0)
			break;

		msg_bytes_received += useful_msg_len;

		for (int i = MSG_HEADER_SIZE; i < MSG_HEADER_SIZE + useful_msg_len; i++) {
			fputc(msg_chunk_data[i], file);
		}
		
		if (msg_bytes_received % 100 == 0)
			printf("Fazendo o download: [%d] bytes\n", msg_bytes_received);
	}

	fclose(file);
}