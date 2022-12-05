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
static int queue_buffer_pos = 0;
static char queue_buffer[MSG_MAX_SIZE];

static int pdu_size;

static char * incoming_frame_buffer;
static long long incoming_frame_id = 0;

static char * outcoming_frame_buffer;
static long long outcoming_frame_id = 0;

static const char * dll_error_msg_format = "(DLL) \033[0;31mERROR:\033[0m ";
static const char * dll_warning_msg_format = "(DLL) \033[0;33mWARNING:\033[0m ";
static const char * dll_success_msg_format = "(DLL) \033[0;32mSUCCESS:\033[0m ";
static const char * dll_info_msg_format = "(DLL) \033[0;34mINFO:\033[0m ";

static int verbose = 0;
static int dll_is_running = 1;

void init_dll_process(char * host_port, char * receiver_address, char * receiver_port, int flag_pdu_size)
{
    init_socket(host_port, receiver_address, receiver_port, MICRO_TIMEOUT);
    initialize_dll_interface(NANO_TIMEOUT);

    pdu_size = MSG_MAX_SIZE + PDU_HEADER_SIZE;
    for (int i = 2; i < MSG_MAX_SIZE; i++) {
        if (MSG_MAX_SIZE % i != 0)
            continue;

        if (abs((i + PDU_HEADER_SIZE) - flag_pdu_size) < abs((pdu_size + PDU_HEADER_SIZE) - flag_pdu_size))
            pdu_size = i + PDU_HEADER_SIZE;
    }

    if (MSG_MAX_SIZE % (pdu_size - PDU_HEADER_SIZE) != 0) {
        printf("%sCritical error: queue message size (%d) can't be divided by PDU payload size (%d).\n", dll_error_msg_format, MSG_MAX_SIZE, pdu_size - PDU_HEADER_SIZE);
        printf("%sInsert a different PDU size.\n", dll_warning_msg_format);
        exit(1);
    }

    if (pdu_size != flag_pdu_size)
        printf("%sPDU size was adjusted to %d to avoid inconsistency problems.\n", dll_warning_msg_format, pdu_size);

    if ((incoming_frame_buffer = malloc((size_t) pdu_size)) == NULL) {
        printf("%sUnable to allocate memory to incoming frame buffer.\n", dll_error_msg_format);
        exit(1);
    }

    if ((outcoming_frame_buffer = malloc((size_t) pdu_size)) == NULL) {
        printf("%sUnable to allocate memory to outcoming frame buffer.\n", dll_error_msg_format);
        exit(1);
    }

    operation_mode = ID_RECEIVER;
    printf("%sData Link Layer initialized as %s successfully with PID %d!\n", dll_success_msg_format, operation_mode == ID_SENDER ? "ID_SENDER" : "ID_RECEIVER", getpid());

    if (verbose)
        printf("%sInitialized in VERBOSE mode!\n", dll_info_msg_format);

    execute_dll_process();
    destroy_dll_process();
}

void destroy_dll_process()
{
    free(incoming_frame_buffer);
    free(outcoming_frame_buffer);

    shut_down_dll_interface();
    destroy_socket();
}

void execute_dll_process()
{
    while (dll_is_running) {
        if (operation_mode == ID_SENDER) {
            if (get_msg_data_from_queue() == MSG_TIMEOUT) {
                // Ninguem enviou nada via fila, checar socket
                operation_mode = ID_RECEIVER;
                continue;
            }
            send_msg_data();
        }

        if (operation_mode == ID_RECEIVER) {
            while (queue_buffer_pos < QUEUE_BUFFER_SIZE - 1) {
                int frame_status;
                while (frame_status = receive_frame()) {
                    if (frame_status == SOCKET_TIMEOUT) {
                        // Ninguem enviou nada no socket, checar fila
                        operation_mode = ID_SENDER;
                        break;
                    }
                    printf("%sFailed to receive frame %lld. Trying again...\n", dll_error_msg_format, incoming_frame_id);
                }

                if (operation_mode == ID_SENDER)
                    break;

                get_frame_from_sender();
                unpack_message_from_frame_buffer();
            }
            queue_buffer_pos = 0;

            if (operation_mode == ID_RECEIVER)
                send_msg_data_to_queue();
        }
    }
}

void set_verbose_dll(int value)
{
    verbose = value;
}

void define_op_mode(int value)
{
    if (value != ID_RECEIVER && value != ID_SENDER) {
        printf("%sAttempt to set operation mode with invalid value: %d.\n", dll_error_msg_format, value);
        return;
    }

    operation_mode = value;

    if (verbose)
        printf("%sNow operating as %s.\n", dll_info_msg_format, operation_mode == ID_RECEIVER ? "ID_RECEIVER" : "ID_SENDER");
}

int get_msg_data_from_queue()
{
    int nothing;
    return get_timed_data_from_app(queue_buffer, &nothing);
}

void send_msg_data_to_queue()
{
    send_data_to_app(queue_buffer, QUEUE_BUFFER_SIZE);
}

int verify_incoming_frame()
{
    long long received_id = *((long long *)incoming_frame_buffer);
    if (received_id != incoming_frame_id) {
        printf("%sPackage ID differs from expected. Expected: %lld, actual: %lld\n", dll_error_msg_format, incoming_frame_id, received_id);
        return 1;
    }

    // TODO implementar CRC ou paridade
    // ------------

    incoming_frame_id++;
    return 0;
}

int send_status_error_confirmation_frame()
{
    memset(outcoming_frame_buffer, 0xFF, pdu_size);
    return send_frame();
}

int send_success_confirmation_frame()
{
    memset(outcoming_frame_buffer, 0x0, pdu_size);
    return send_frame();
}

void send_msg_data()
{
    if (verbose)
        printf("%sSending data from queue.\n", dll_info_msg_format);

    while (queue_buffer_pos < QUEUE_BUFFER_SIZE - 1) {
        pack_message_from_queue_buffer();
        send_frame_msg_to_receiver();
    }
    queue_buffer_pos = 0;

    if (verbose)
        printf("%sData from queue sent successfully.\n\n", dll_info_msg_format);
}

void send_frame_msg_to_receiver()
{
    int attempts = 0;
    while (1) {
        if (verbose)
            printf("%sSending frame %lld.\n", dll_info_msg_format, outcoming_frame_id);

        while (send_frame()) {
            printf("%sFailed to send frame %lld. Trying again...\n", dll_error_msg_format, outcoming_frame_id);
        }

        while (get_info_confirmation_frame() && attempts <= 3) {
            printf("%sFailed to receive confirmation frame. Trying again...\n", dll_error_msg_format);
            attempts++;
        }
        if (attempts > 3) {
            printf("%sResending frame due confirmation failure.\n", dll_warning_msg_format);
            attempts = 0;
            continue;
        }
        if (check_info_confirmation_frame())
            continue;

        break;
    }
}

int send_frame()
{
    return send_msg_through_socket(outcoming_frame_buffer, pdu_size);
}

int get_info_confirmation_frame()
{
    receive_frame();
}

int check_info_confirmation_frame()
{
    int is_error = 0;
    for (int i = 0; i < pdu_size; i++) {
        if (incoming_frame_buffer[i] == 0xFF)
            is_error = 1;
    }
    if (is_error)
        printf("%sReceived an ERROR confirmation from confirmation frame.\n", dll_warning_msg_format);

    return is_error;
}

int receive_frame()
{
    int result = receive_msg_through_socket(incoming_frame_buffer, pdu_size);
    return result;
}

void get_frame_from_sender()
{
    if (verbose)
        printf("%sReceiving frame %lld.\n", dll_info_msg_format, incoming_frame_id);

    if (verify_incoming_frame())
        while (send_status_error_confirmation_frame())
            printf("%sFailed to send ERROR confirmation frame. Trying again...\n", dll_error_msg_format);

    else
        while (send_success_confirmation_frame())
            printf("%sFailed to send OK confirmation frame. Trying again...\n", dll_error_msg_format);
}

void pack_message_from_queue_buffer()
{
    *((long long *) outcoming_frame_buffer) = outcoming_frame_id++;
    for (int i = 0; i < pdu_size - PDU_HEADER_SIZE; i++)
        outcoming_frame_buffer[i + PDU_HEADER_SIZE] = queue_buffer[queue_buffer_pos++];
}

void unpack_message_from_frame_buffer()
{
    for (int i = 0; i < pdu_size - PDU_HEADER_SIZE; i++)
        queue_buffer[queue_buffer_pos++] = incoming_frame_buffer[i + PDU_HEADER_SIZE];
}