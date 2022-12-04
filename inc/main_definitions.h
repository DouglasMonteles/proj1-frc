#define bool short
#define TRUE 1
#define FALSE 0
#define YES TRUE
#define NO FALSE

#define MSG_HEADER 4
#define MSG_BODY 100
#define MSG_MAX_PIECES (MSG_HEADER + MSG_BODY)
#define MSG_MAX_AMOUNT 10

#define PDU_HEADER 8
#define PDU_BODY MSG_MAX_PIECES
#define PDU_SIZE (PDU_HEADER + PDU_BODY)

#define SENDER 0
#define RECEIVER 1
#define NO_PRIORITY 0

#define SOCKET_TIMEOUT 8
#define TIMEOUT 5
#define TIMEOUT_100K 100000L
#define TIMEOUT_1000K 100000000L

#define BUFFER_SIZE MSG_MAX_PIECES

void l(char *msg);