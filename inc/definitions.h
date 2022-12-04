#ifndef _DEFINITIONS_H_

#define CQ_HEADER_LEN 4
#define CQ_MESSAGE_LEN 100
#define CQ_DATA_MAX_LEN (CQ_HEADER_LEN + CQ_MESSAGE_LEN)
#define CQ_MAX_DATA_AMOUNT 10

#define CQ_TIMEOUT 5

#define SENDER 0
#define RECEIVER 1

#define PDU_HEADER_SIZE 8

#define MICRO_TIMEOUT 500000L
#define NANO_TIMEOUT 500000000L

#define MENU 0
#define SEND_FILE 1
#define RECEIVE_FILE 2

#define SC_TIMEOUT 5

#endif