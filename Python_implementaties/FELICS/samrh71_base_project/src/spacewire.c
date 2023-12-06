#include "sam.h"
#include "spacewire.h"

// FreeRTOS includes
#include "FreeRTOS.h"
#include "queue.h"

#include "opt/printf-stdarg.h"

#define NOCACHE __attribute__((section(".flexram_nocache"), aligned(32)))

// Buffers
#define N_RXBUF 2
#define N_TXBUF 2
#define N_SEND_DESC 2

static char send_desc_busy[N_SEND_DESC];

NOCACHE static char rx_bufs[N_RXBUF][BIG_BUF_SIZE];
NOCACHE static char tx_bufs[N_TXBUF][SMALL_BUF_SIZE];
NOCACHE static char tx_headers[N_TXBUF][TX_HEADER_SIZE];
NOCACHE static rcv_desc_t rcv_desc_list[N_RXBUF];
NOCACHE static send_desc_t send_desc_list[N_SEND_DESC];

static unsigned int spw_error = 0;

static QueueHandle_t spw_event_queue;

static rcv_desc_t *last_act_rcv_desc = 0;
static send_desc_t *last_act_send_desc = 0;
static unsigned int last_tx1_status;
static unsigned int last_rx1_prev_buf_status;

void spw_init(void)
{
    // !!! from Datasheet ----------------------------------------------------
    // 36.5.2 Power Management
    // The SpW is clocked through the Power Management Controller (PMC),thus the programmer must first configure the
    // PMC to enable the SpW clock and the timetick clock if required.
    // On transmission, the SpaceWire bitrate is defined from the GCLK clock frequency (see the Speed Link section for
    // more details). The GLCK frequency must be less than or equal to twice the MCK clock frequency; that is, GCLK ≤
    // 2*MCK
    // On reception, the SpaceWire bitrate is linked to the MCK clock frequency. The bit rate must be less than or equal to
    // four times MCK; that is, RX_SpaceWire_bitrate ≤ 4*MCK.
    //------------------------------------------------------------------------

    // Enable Peripheral Clock for SpaceWire (id=65), GCLK source = PLLA_CLK / (GCLKDIV + 1)
    //                                                            = 100 MHz  / 1 = 100 MHz
    PMC_REGS->PMC_PCR = PMC_PCR_EN(1) | PMC_PCR_GCLKCSS_PLLA_CLK | PMC_PCR_GCLKDIV(0) | PMC_PCR_CMD(1) | PMC_PCR_PID(65);
    // Enable GCLK for SpaceWire (id=65)
    PMC_REGS->PMC_PCR = PMC_PCR_EN(1) | PMC_PCR_GCLKCSS_PLLA_CLK | PMC_PCR_GCLKDIV(0) | PMC_PCR_CMD(1) | PMC_PCR_PID(65) | PMC_PCR_GCLKEN(1);
    // PMC_REGS->PMC_PCR = PMC_PCR_EN(1) | PMC_PCR_GCLKEN(1) | PMC_PCR_GCLKCSS_MCK_CLK | PMC_PCR_CMD(1) | PMC_PCR_PID(65);

    // The initial transmitter signalling bitrate is SpWInitbitrate = SpwClk / (0.5 x (TxInitDiv + 1))
    // The operating transmitter signalling bitrate is SpWOperbitrate = SpwClk / (0.5 x (TxOperDiv + 1))
    // SpWCLK = 100MHz,  TxInitDiv = 19 => SpWInitbitrate = 10 Mbits/s
    // SpWCLK = 100MHz,  TxOperDiv = 0  => SpWOperbitrate = 200 Mbits/s
    SPW_REGS->SPW_LINK1_CLKDIV = SPW_LINK1_CLKDIV_TXINITDIV(19) | SPW_LINK1_CLKDIV_TXOPERDIV(0);
    SPW_REGS->SPW_LINK2_CLKDIV = SPW_LINK2_CLKDIV_TXINITDIV(19) | SPW_LINK2_CLKDIV_TXOPERDIV(0);

    SPW_REGS->SPW_LINK1_CFG = SPW_LINK1_CFG_COMMAND_LINK_START;

    spw_event_queue = xQueueCreate(N_RXBUF + N_SEND_DESC, sizeof(spw_event_t));
}

inline unsigned int spw_link1_status(void)
{
    return SPW_REGS->SPW_LINK1_STATUS;
}

inline unsigned int spw_link2_status(void)
{
    return SPW_REGS->SPW_LINK2_STATUS;
}

inline unsigned int spw_router_status(void)
{
    return SPW_REGS->SPW_ROUTER_STS;
}

inline static void next_rcv(rcv_desc_t *rcv_desc)
{
    if (rcv_desc == &rcv_desc_list[0])
        SPW_REGS->SPW_PKTRX1_NXTBUFDATAADDR = rx_bufs[0];
    else if (rcv_desc == &rcv_desc_list[1])
        SPW_REGS->SPW_PKTRX1_NXTBUFDATAADDR = rx_bufs[1];
    else
        return -1;

    SPW_REGS->SPW_PKTRX1_NXTBUFDATALEN = BIG_BUF_SIZE;

    SPW_REGS->SPW_PKTRX1_NXTBUFPKTADDR = rcv_desc;
    SPW_REGS->SPW_PKTRX1_NXTBUFCFG = SPW_PKTRX1_NXTBUFCFG_MAXCNT(1);
    SPW_REGS->SPW_PKTRX1_NXTBUFCFG = SPW_PKTRX1_NXTBUFCFG_MAXCNT(1) | SPW_PKTRX1_NXTBUFCFG_START_STARTLATER;
    return 0;
}

inline static void reset_transmitter(void)
{
    SPW_REGS->SPW_PKTTX1_SWRESET = 0x4D616A6F;
    SPW_REGS->SPW_PKTTX1_SWRESET = 0x72546F6D;
}

static void init_transmitter(void)
{
    reset_transmitter();
    memset(send_desc_list, 0, N_SEND_DESC * sizeof(send_desc_t));
}

inline static void reset_receiver(void)
{
    SPW_REGS->SPW_PKTRX1_SWRESET = 0x4D616A6F;
    SPW_REGS->SPW_PKTRX1_SWRESET = 0x72546F6D;
    // printf("SPW_PKTRX1 Soft Reset !");
}

static void init_receiver(void)
{
    reset_receiver();
}


static void init_interrupts(void)
{
    NVIC_SetPriority(SPW_IRQn, 5);
    NVIC_EnableIRQ(SPW_IRQn);

    SPW_REGS->SPW_PKTRX1_PI_C = 0x1f; // Clear all pending interrupts for RX1
    SPW_REGS->SPW_PKTTX1_PI_C = 0x0f; // Clear all pending interrupts for TX1

    SPW_REGS->SPW_PKTRX1_IM = SPW_PKTRX1_IM_ACT(1) | SPW_PKTRX1_IM_DEACT(1);
    SPW_REGS->SPW_PKTTX1_IM = SPW_PKTTX1_IM_ACT(1) | SPW_PKTTX1_IM_DEACT(1);
}

void start_spw(void)
{
    init_receiver();
    init_transmitter();
    init_interrupts();
    next_rcv(&rcv_desc_list[0]);
    next_rcv(&rcv_desc_list[1]);
}

void release_rcv_desc(rcv_desc_t *rcv_desc) {
    last_rx1_prev_buf_status = SPW_REGS->SPW_PKTRX1_PREVBUFSTS;
    next_rcv(rcv_desc);
}

send_desc_t* spw_get_send_desc() {
    if (! send_desc_busy[0]) {
        send_desc_list[0].str.daddr = &tx_bufs[0];
        send_desc_list[0].str.haddr = &tx_headers[0];
        send_desc_busy[0] = 1;
        return &send_desc_list[0];
    }
    else if (! send_desc_busy[1]) {
        send_desc_list[1].str.daddr = &tx_bufs[1];
        send_desc_list[1].str.haddr = &tx_headers[1];
        send_desc_busy[1] = 1;
        return &send_desc_list[1];
    }
    else
        return 0;
}

void spw_release_send_desc(send_desc_t *send_desc) {
    if (send_desc == &send_desc_list[0])
        send_desc_busy[0] = 0;
    else if (send_desc == &send_desc_list[1])
        send_desc_busy[1] = 0;
    else
        printf("Invalid (release) send desc %08x\n", send_desc);
}

spw_event_t wait_spw_event() {
    spw_event_t e;
    xQueueReceive(spw_event_queue, &e, -1);
    return e;
}

static void rx_act_handler(void)
{
    last_act_rcv_desc = SPW_REGS->SPW_PKTRX1_CURBUFPKTADDR;

    // Clear pending interrupt
    SPW_REGS->SPW_PKTRX1_PI_C = SPW_PKTRX1_PI_C_ACT(1);
}

static void rx_deact_handler(void)
{
    spw_event_t e;
    e.type = RCV_DESC_EV;
    e.p = last_act_rcv_desc;
    if (xQueueSendToBackFromISR(spw_event_queue, &e, NULL) == errQUEUE_FULL)
        spw_error |= SPW_RX_Q_FULL_ERROR;

    // Clear pending interrupt
    SPW_REGS->SPW_PKTRX1_PI_C = SPW_PKTRX1_PI_C_DEACT(1);
}

static void tx_act_handler(void)
{   
    last_act_send_desc = SPW_REGS->SPW_PKTTX1_CURSENDADDR;

    // Clear pending interrupt
    SPW_REGS->SPW_PKTTX1_PI_C = SPW_PKTTX1_PI_C_ACT(1);
}

static void tx_deact_handler(void)
{
    spw_event_t e;
    unsigned int tx_status;
    tx_status = SPW_REGS->SPW_PKTTX1_STATUS;
    last_tx1_status = tx_status;
    if ((tx_status & SPW_PKTTX1_STATUS_PREV_Msk) == SPW_PKTTX1_STATUS_PREV_LASTSENDLISTOK) {
        e.type = SEND_DESC_EV;
        e.p = last_act_send_desc;
        if (xQueueSendToBackFromISR(spw_event_queue, &e, NULL) == errQUEUE_FULL)
            spw_error |= SPW_TX_Q_FULL_ERROR;
    }
    else if ((tx_status & SPW_PKTTX1_STATUS_PREV_Msk) == SPW_PKTTX1_STATUS_PREV_ABORTEDTIMEOUT) {
        e.type = TX_TIMEOUT_EV;
        e.p = last_act_send_desc;
        if (xQueueSendToBackFromISR(spw_event_queue, &e, NULL) == errQUEUE_FULL)
            spw_error |= SPW_TX_Q_FULL_ERROR;
    } else {
        spw_error |= TX_PREV_ERROR;
    }

    // Unlock the send list
    SPW_REGS->SPW_PKTTX1_STATUS = 0;

    // Clear pending interrupt
    SPW_REGS->SPW_PKTTX1_PI_C = SPW_PKTTX1_PI_C_DEACT(1);
}

void SPW_Handler(void)
{
    unsigned int pending_int_reg;

    pending_int_reg = SPW_REGS->SPW_PKTRX1_PI_RM;
    if (pending_int_reg & SPW_PKTRX1_PI_RM_DEACT_Msk)
        rx_deact_handler();
    if (pending_int_reg & SPW_PKTRX1_PI_RM_ACT_Msk)
        rx_act_handler();

    pending_int_reg = SPW_REGS->SPW_PKTTX1_PI_RM;
    if (pending_int_reg & SPW_PKTTX1_PI_RM_ACT_Msk)
        tx_act_handler();
    if (pending_int_reg & SPW_PKTTX1_PI_RM_DEACT_Msk)
        tx_deact_handler();
}

static void print_link_status(unsigned int s)
{
    printf("  Seen: (");
    if (s & 0x02000000) printf("Run ");
    if (s & 0x01000000) printf("Con ");
    if (s & 0x00800000) printf("Std ");
    if (s & 0x00400000) printf("Rdy ");
    if (s & 0x00200000) printf("ErW ");
    if (s & 0x00100000) printf("ErR ");
    printf(")\n  ");
    if (s & 0x00080000) printf("GotNCh ");
    if (s & 0x00040000) printf("GotFCT ");
    if (s & 0x00020000) printf("GotNul ");
    if (s & 0x00010000) printf("TxEm ");
    printf("Div=%d  State=", (s & 0xf0) >> 4);
    switch(s & 0x7) {
        case 0: printf("ErrRst"); break;
        case 1: printf("ErrWait"); break;
        case 2: printf("Rdy"); break;
        case 3: printf("Started"); break;
        case 4: printf("Conn"); break;
        case 5: printf("Run"); break;
        default: printf("???"); break;
    }
    printf("\n");
}

static void print_rx1_status(unsigned int s)
{
    printf("spw_rx1_status          %08x\n", s);
    printf("  Deact=%d Pending=%d Act=%d", (s >> 21) & 1, (s >> 20) & 1, (s >> 19) & 1);
    printf("    Arm=%d  Locked=%d Pkt=%d\n", (s >> 18) & 1, (s >> 17) & 1, (s >> 16) & 1);
    printf("  Count=%d\n", s & 0xffff);
}

static void print_rx1_prev_buf_status(unsigned int s)
{
    printf("spw_rx1_prev_buf_status %08x\n", s);
    printf("  Lckd=%d DMAerr=%d Fulld=%d Fulli=%d EEP=%d\n", (s >> 31) & 1, (s >> 19) & 1, 
            (s >> 18) & 1, (s >> 17) & 1, (s >> 16) & 1);
    printf("  Count=%d\n", s & 0xffff);
}

static void print_tx1_status(unsigned int s)
{
    printf("spw_tx1_status          %08x\n", s);
    printf("  Prev: ");
    switch((s >> 16) & 0x7) {
        case 0: printf("NoInfo"); break;
        case 1: printf("LastOK"); break;
        case 2: printf("AbMemErr"); break;
        case 3: printf("AbNew"); break;
        case 4: printf("AbUsr"); break;
        case 5: printf("AbTOut"); break;
        default: printf("???"); break;
    }
    printf("\n  Deact=%d Pending=%d Act=%d Arm=%d\n", (s >> 3) & 1, (s >> 2) & 1, (s >> 1) & 1, s & 1);
}


void print_spw_status(void)
{   
    /*
    printf("rcv_desc_list[0] : %08x\n", &rcv_desc_list[0]);
    printf("rcv_desc_list[1] : %08x\n", &rcv_desc_list[1]);
    printf("spw_router_status       %08x\n", spw_router_status());
    */
    
    printf("spw_link1_status        %08x\n", spw_link1_status());
    print_link_status(spw_link1_status());
    printf("spw_link2_status        %08x\n", spw_link2_status());
    print_link_status(spw_link2_status());
    
    /*
    print_rx1_status(SPW_REGS->SPW_PKTRX1_STATUS);
    print_rx1_prev_buf_status(last_rx1_prev_buf_status);
    print_tx1_status(last_tx1_status);
    
    printf("Error reg      : %x\n", spw_error);
    printf("send_desc_busy : %d %d\n", send_desc_busy[0], send_desc_busy[1]);
    */
}

