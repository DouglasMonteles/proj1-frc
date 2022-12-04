#include "dll.h"
#include "I_dll.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

static int mode;
static char cmd[250];
static int app_running = 1;


#define MENU 0
#define SEND_FILE 1
#define RECEIVE_FILE 2


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
        mode = SEND_FILE;
    else if (cmd[0] == '2')
        mode = RECEIVE_FILE;
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
    mode = MENU;
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
    mode = MENU;
}

void run_app()
{
    while (app_running) {
        display_app_header();

        switch (mode) {
        case MENU:
            display_menu();
            process_menu();
            break;

        case SEND_FILE:
            display_send_file();
            process_send_file();
            break;

        case RECEIVE_FILE:
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
        mode = MENU;
        return 1;
    }

    int file_size = 0;

    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);

    rewind(fp);

    char chunk[CQ_DATA_MAX_LEN];

    int bytes_read;
    int bytes_sent = 0;
    long long int packages_sent = 0;

    while (bytes_read = fread(chunk + CQ_HEADER_LEN, sizeof(char), CQ_MESSAGE_LEN, fp)) {
        *((int *)chunk) = bytes_read;

        send_data_to_dll(chunk, CQ_DATA_MAX_LEN);
        bytes_sent += bytes_read;
        packages_sent++;
        if (packages_sent % 100 == 0)
            printf("\t\t\033[0;34mUpload:\033[0m %dB/ %dB\n", bytes_sent, file_size);
    }
    memset(chunk, 0x0, CQ_DATA_MAX_LEN);
    send_data_to_dll(chunk, CQ_DATA_MAX_LEN);

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

    char chunk_data[CQ_DATA_MAX_LEN];

    while(1) {
        get_data_from_dll(chunk_data, &chunk_len);

        int useful_msg_len = *((int *)chunk_data);
        if (useful_msg_len == 0)
            break;

        bytes_received += useful_msg_len;

        for (int i = CQ_HEADER_LEN; i < CQ_HEADER_LEN + useful_msg_len; i++) {
            fputc(chunk_data[i], fp);
        }
        if (bytes_received % 100 == 0)
            printf("\t\t\033[0;34mDownload:\033[0m %dB\n", bytes_received);
    }

    fclose(fp);
}

void show_help()
{
    printf("\033[0;31mUsage\033[0m:\n\tStart Data Link Layer: \033[0;33mprog dll HOST_PORT RECEIVER_ADDRESS RECEIVER_PORT [options]\033[0m\n");
    printf("\n\t\tOptions:\n\t\t\t\033[0;35m[-p, --pdu-size] PDU_SIZE\033[0m\t\t\033[0;34m# Set the PDU size to PDU_SIZE (PS: this option is just a hint and might not take effect)\033[0m\n");
    printf("\t\t\t\033[0;35m[-v, --verbose]\t\t\t\t\033[0;34m# Start DLL in verbose mode\033[0m\n");
    printf("\n\tStart the Application: \033[0;33mprog app\033[0m\n\n");
}

int main(int argc, char ** argv)
{
    if (argc == 1) {
        show_help();
        return 0;
    }
    else if (argc > 1) {
        if (strcmp(argv[1], "app") == 0) {
            initialize_dll_interface(NANO_TIMEOUT);
            run_app();
            shut_down_dll_interface();
        }
        else if (strcmp(argv[1], "dll") == 0 && argc >= 5) {

            int argi = 5;
            int pdu_size = 100;
            int verbose = 0;
            while (argi < argc) {
                if (strcmp(argv[argi], "-p") == 0 || strcmp(argv[argi], "--pdu-size") == 0) {
                    if (argc < 7) {
                        show_help();
                        return 0;
                    }
                    pdu_size = atoi(argv[argi + 1]);
                    argi += 2;
                }
                else if (strcmp(argv[argi], "-v") == 0 || strcmp(argv[argi], "--verbose") == 0) {
                    verbose = 1;
                    argi++;
                }
            }
            initialize_dll(argv[2], argv[3], argv[4], pdu_size, verbose);
        }
        else {
            show_help();
        }
    }
    return 0;
}