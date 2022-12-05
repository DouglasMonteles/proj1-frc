#include "definitions.h"
#include "dll_display.h"
#include "file_download_upload.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

static int mode;
static char cmd[250];
static int app_running = 1;

void display_app_header()
{
    system("tput reset");
    printf("------------------------- \033[0;32mgamaTorrent\033[0m -----------------------------------\n");
    printf("\t\tCompartilhe seus arquivos com qualquer um sem que a policia federal bata na sua porta!\n\n");
}

void display_menu()
{
    printf("\t\t\t- Para enviar arquivos, digite 1\n");
    printf("\t\t\t- Para receber arquivos, digite 2\n");
    printf("\t\t\t- Para sair, digite 3\n");
}

void process_menu()
{
    scanf(" %s", cmd);
    if (cmd[0] == '1')
        mode = OPC_SEND_FILE;
    else if (cmd[0] == '2')
        mode = OPC_RECEIVE_FILE;
    else if (cmd[0] == '3')
        app_running = 0;
}

void display_send_file()
{
    printf("\t\tPasta atual:\n");
    system("ls");
    printf("\n\n\t\tDigite o caminho do arquivo a ser enviado: ");
}

void process_send_file()
{
    scanf(" %s", cmd);
    if (process_file(cmd) == 0)
        printf("\033[0;32mArquivo enviado!\033[0m\n");
    else
        printf("\033[0;31mOcorreu um erro durante o envio do arquivo\033[0m\n");

    printf("\033[0;33mDigite algo para continuar\033[0m.\n");
    scanf(" %s", cmd);
    mode = OPC_MENU;
}

void display_receive_file()
{
    printf("\t\tVocê será notificado assim que alguém estiver lhe enviando um arquivo.\n");
}

void process_receive_file()
{
    mount_file();
    printf("\033[0;32mArquivo recebido!\033[0m\n");
    printf("\033[0;33mDigite algo para continuar\033[0m.\n");
    scanf(" %s", cmd);
    mode = OPC_MENU;
}

void run_app()
{
    while (app_running) {
        display_app_header();

        switch (mode) {
        case OPC_MENU:
            display_menu();
            process_menu();
            break;

        case OPC_SEND_FILE:
            display_send_file();
            process_send_file();
            break;

        case OPC_RECEIVE_FILE:
            display_receive_file();
            process_receive_file();
            break;

        default:
            break;
        }
    }
}

void show_chunk(char * chunk, int chunk_len)
{
    for (int i = 0; i < chunk_len; i++)
        printf("%c", chunk[i]);
}

int process_file(char *file_path)
{
    FILE *fp;
    fp = fopen(file_path, "r");

    if (fp == NULL) {
        printf("Erro: %s\n", strerror(errno));
        mode = OPC_MENU;
        return 1;
    }

    int file_size = 0;

    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);

    rewind(fp);

    char chunk[MSG_MAX_SIZE];

    int bytes_read;
    int bytes_sent = 0;
    long long int packages_sent = 0;

    while (bytes_read = fread(chunk + MSG_HEADER_SIZE, sizeof(char), MSG_SIZE, fp)) {
        *((int *)chunk) = bytes_read;

        send_data_to_dll(chunk, MSG_MAX_SIZE);
        bytes_sent += bytes_read;
        packages_sent++;
        if (packages_sent % 100 == 0)
            printf("\t\t\033[0;34mUpload:\033[0m %dB/ %dB\n", bytes_sent, file_size);
    }
    memset(chunk, 0x0, MSG_MAX_SIZE);
    send_data_to_dll(chunk, MSG_MAX_SIZE);

    fclose(fp);

    return 0;
}

void mount_file()
{
    char filename[255];

    int chunk_len;
    int bytes_received = 0;

    printf("\t\tDigite o nome do arquivo a ser salvo.\n");
    scanf(" %s", filename);

    FILE *fp;
    fp = fopen(filename, "w");

    char chunk_data[MSG_MAX_SIZE];

    while(1) {
        get_data_from_dll(chunk_data, &chunk_len);

        int useful_msg_len = *((int *)chunk_data);
        if (useful_msg_len == 0)
            break;

        bytes_received += useful_msg_len;

        for (int i = MSG_HEADER_SIZE; i < MSG_HEADER_SIZE + useful_msg_len; i++) {
            fputc(chunk_data[i], fp);
        }
        if (bytes_received % 100 == 0)
            printf("\t\t\033[0;34mDownload:\033[0m %dB\n", bytes_received);
    }

    fclose(fp);
}