#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main_definitions.h"
#include "dll.h"
#include "I_dll.h"
#include "ws.h"

bool is_app_running = 1;
int mode;

void call_witch_process(char goal, char* file_name);

bool read_yes_or_no_answer() {
  char resp;
  printf("Deseja informar o nome de outro arquivo para enviar? [s/n]\n");
  scanf(" %c", &resp);

  return resp == 's' ? YES : NO;
}

void menu_options(char *file_name, char goal) {
  int opc;

  printf("------ MENU DE OPCOES ----\n");
  printf("[1] - Enviar arquivos\n");
  printf("[2] - Receber arquivos\n");
  printf("[3] - Finalizar programa\n");
  printf("------ -------------- ----\n");

  scanf("%d", &opc);

  switch (opc) {
    case 1:
      mode = 1; // SEND FILE
    break;
    
    case 2:
      mode = 2; //RECEIVER FILE
    break;
    default:
      is_app_running = FALSE;
      break;
  }

  while (is_app_running) {
    char file_name[100];
    
    switch (mode) {
      case 0: // MENU
      
      break;

      case 1: // SEND
        scanf(" %s", file_name);
        call_witch_process('s', file_name);
      break;

      case 2: // RECEIVE
        scanf(" %s", file_name);
        call_witch_process('r', file_name);
      break;
    }
  }
}

void l(char *msg) {
  printf("log: %s\n", msg);
}

void info_file_size(FILE* file) {
  int size = 0;

  fseek(file, 0l, SEEK_END);
  size = ftell(file);
  rewind(file);

  printf("Tamanho do arquivo: %d bytes\n", size);
}

void show_pieces_of_bytes(char* arr, int length) {
  for (int i = 0; i < length; i++)
    printf("%c\n", arr[i]);
}

void send_file_bytes(FILE* file) {
  char msg_pieces[MSG_MAX_PIECES]; // Tamanho acordado da mensagem

  l("Estabelecendo conexão para enviar o arquivo...");
  
  int bytes = 0;
  int bytes_sended = 0;
  long long packs_sended = 0;

  while (bytes = fread((msg_pieces + MSG_HEADER), sizeof(char), MSG_BODY, file)) {
    *((int *) msg_pieces) = bytes;
    
    // Show the pieces of bytes
    // show_pieces_of_bytes(msg_pieces, MSG_MAX_PIECES);

    send_msg_to_dll(msg_pieces, MSG_MAX_PIECES);

    bytes_sended += bytes;
    packs_sended++;
    if (packs_sended % 100 == 0) {
      printf("Download -> %d de ", bytes_sended);
      info_file_size(file);
      printf("\n");
    }
  }

  memset(msg_pieces, 0x0, MSG_MAX_PIECES);
  send_msg_to_dll(msg_pieces, MSG_MAX_PIECES);

  l("Arquivo enviado \\(^o^)/");
}

void send_file(char* file_name) {
  int size;
  // Read the file
  FILE *file;
  file = fopen(file_name, "r");

  if (file == NULL) {
    printf("Erro ao ler arquivo!\n");
    exit(1);
  }

  // Print file size in bytes
  info_file_size(file);

  // Sending the file
  send_file_bytes(file);

  fclose(file);
}

void receive_file(char* file_name) {
  FILE *file;
  file = fopen(file_name, "w");

  int len = 0;
  char data_msg_in_bytes[MSG_MAX_PIECES];

  while (TRUE) {
    get_data_from_dll(data_msg_in_bytes, &len);
    int msg_size = *((int*) data_msg_in_bytes);

    if (msg_size <= 0) break;

    // copy data in the file
    for (int i = MSG_HEADER; i < (MSG_HEADER + msg_size); i++)
      fputc(data_msg_in_bytes[i], file);
  } 

  fclose(file);
}

void call_witch_process(char goal, char* file_name) {
  if (goal == 's') { // SENDER
    // activate_sender_file_process
    l("activate_sender_file_process");
    send_file(file_name);
  } else { // RECEIVER
    // activate_receiver_file_process
    l("activate_receiver_file_process");
    receive_file(file_name);
  }
}

int main(int argc, char **argv) {
  switch (argc) {
    // Prepare to send files
    case 1:
    case 2:
      init_I_dll(TIMEOUT_1000K);
      char goal = argv[1][0]; // SENDER = 's' or RECEIVER = 'r'
      char file_name[100];

      do {
        menu_options(file_name, goal);
        
      } while (read_yes_or_no_answer());
    break;

    // Prepare to observer the send and receive files
    default:
      init_dll(argv[2], argv[3], argv[4], 112, 1);
      set_verbose_dll(1);
  }

  return 0;
}