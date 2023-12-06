#ifndef INC_SPACEWIRE_H
#define INC_SPACEWIRE_H

#include "FreeRTOS.h"

#define BIG_BUF_SIZE 276480 + 1024 // 384 * 288 * 20 / 8;
#define SMALL_BUF_SIZE 1024
#define TX_HEADER_SIZE 1024

#define SPW_RX_Q_FULL_ERROR 1
#define SPW_TX_Q_FULL_ERROR 2
#define TX_PREV_ERROR 4

typedef struct
{
    unsigned start : 19;
    unsigned : 5;
    unsigned rsize : 4;
    unsigned : 1;
    unsigned entry : 2;
    unsigned skip : 1;

    unsigned rb4 : 8;
    unsigned rb3 : 8;
    unsigned rb2 : 8;
    unsigned rb1 : 8;

    unsigned rb8 : 8;
    unsigned rb7 : 8;
    unsigned rb6 : 8;
    unsigned rb5 : 8;

    unsigned hsize : 8;
    unsigned hcrc : 1;
    unsigned : 7;
    unsigned esc_char : 8;
    unsigned esc_mask : 4;
    unsigned : 4;

    unsigned haddr : 32;

    unsigned dsize : 24;
    unsigned dcrc : 1;
    unsigned : 7;

    unsigned daddr : 32;

    unsigned timeout : 19;
    unsigned : 13;
} send_desc_str;

typedef union
{
    send_desc_str str;
    unsigned int words[8];
} send_desc_t;

typedef struct
{
    unsigned eop : 1;
    unsigned eep : 1;
    unsigned cont : 1;
    unsigned split : 1;
    unsigned : 12;
    unsigned crc : 8;
    unsigned : 8;

    unsigned daddr : 32;

    unsigned dsize : 24;

    unsigned etime : 19;
    unsigned : 13;
} rcv_desc_str;

typedef union
{
    rcv_desc_str str;
    unsigned int words[4];
} rcv_desc_t;

#define RCV_DESC_EV 1
#define SEND_DESC_EV 2
#define TX_TIMEOUT_EV 3

typedef struct {
    int type;
    void *p;
} spw_event_t;

void spw_init(void);

unsigned int spw_link1_status(void);
unsigned int spw_link2_status(void);
unsigned int spw_router_status(void);
unsigned int spw_tx1_status(void);
unsigned int spw_rx1_status(void);
unsigned int spw_rx1_prev_buf_status(void);

void start_spw(void);
send_desc_t* spw_get_send_desc();
void spw_release_send_desc(send_desc_t *send_desc);
void release_rcv_desc(rcv_desc_t *rcv_desc);
spw_event_t wait_spw_event();

void print_spw_status(void);

#endif
