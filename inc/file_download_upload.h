#ifndef _APPLICATION_H_
#define _APPLICATION_H_

void run_app();

static void display_app_header();

static void display_menu();
static void process_menu();

static void display_send_file();
static void process_send_file();

static void display_receive_file();
static void process_receive_file();

/*
Recebe o caminho do arquivo, quebra este arquivo em múltiplas chunks
e os envia para a camada N-1
*/
static int process_file(char *file_path);

/*
Recebe o nome do arquivo e monta os pacotes, presentes na fila, de tamanho
chunk_size em um arquivo com esse nome
*/
static void mount_file();

#endif