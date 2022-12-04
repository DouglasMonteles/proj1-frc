#ifndef _DATA_LINK_INTERFACE_H_
#define _DATA_LINK_INTERFACE_H_

#include <sys/types.h>

#define CQ_HEADER_LEN 4
#define CQ_MESSAGE_LEN 100
#define CQ_DATA_MAX_LEN (CQ_HEADER_LEN + CQ_MESSAGE_LEN)
#define CQ_MAX_DATA_AMOUNT 10

#define CQ_TIMEOUT 5

// Inicializa a fila de comunicação interprocessos
void initialize_dll_interface(long nano_timeout);

// Finaliza a fila de comunicacao interprocessos
void shut_down_dll_interface();

// // Requisita à camada de enlace de dados uma oportunidade de enviar dados
// // A execucao permanecera bloqueada ate que a camada de enlace de dados
// // responda positivamente à requisicao.
// // Depois de responder positivamente, a camada de enlace escutará dados vindos
// // da interface até que a sessão de envio seja explicitamente encerrada
// // via finish_sending_session
// void request_sending_session();

// // Sinaliza à camada de enlace de dados que a sessão de envio de dados acabou
// // liberando a camada.
// void finish_sending_session();

// // Envia à camada de aplicação um token sinalizando que a camada de
// // enlace de dados está disponivel para ser utilizada para envio de
// // dados
// void seed_dll_token(pid_t token);

// // Destroi os tokens fornecidos se houverem
// void destroy_dll_token(pid_t token);

// Envia dados para a camada de enlace de dados (N-1)
// A função bloqueará a execução até que os dados sejam enviados
//
// data - array que contem os dados a serem enviados
// data_len - o tamanho em bytes do array a ser enviado (deve ser menor que CQ_DATA_MAX_LEN)
void send_data_to_dll(char * data, int data_len);

// Recebe dados da camada de enlace de dados (N-1)
// data - o array que será preenchido com os dados
// data_len - a quantidade de bytes que o array recebeu
// data_len é sempre menor ou igual que CQ_DATA_MAX_LEN
// Retorna 0 em sucesso, retorna TIMEOUT em timeout e retorna errno em erros
int get_timed_data_from_dll(char * data, int * data_len);

// O mesmo que get_timed_data_from_dll porem sem timeout
void get_data_from_dll(char * data, int * data_len);

// Envia dados para a camada de aplicacao (N)
// A função bloqueará a execução até que os dados sejam enviados
//
// data - array que contem os dados a serem enviados
// data_len - o tamanho em bytes do array a ser enviado (deve ser menor que CQ_DATA_MAX_LEN)
// data_len é sempre menor ou igual que CQ_DATA_MAX_LEN
void send_data_to_app(char * data, int data_len);

// Recebe dados da camada da aplicao (N)
// A função bloqueará a execução até que os dados sejam recebidos
//
// data - o array que será preenchido com os dados
// data_len - a quantidade de bytes que o array recebeu
// data_len é sempre menor ou igual que CQ_DATA_MAX_LEN
// Retorna 0 em sucesso, retorna CQ_TIMEOUT em timeout e retorna errno em erros
int get_timed_data_from_app(char * data, int * data_len);

// O mesmo que get_timed_data_from_app porem sem timeout
void get_data_from_app(char * data, int * data_len);

#endif