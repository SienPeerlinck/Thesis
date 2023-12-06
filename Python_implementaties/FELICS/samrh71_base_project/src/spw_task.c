#include "sam.h"
#include "spw_task.h"
#include "spacewire.h"

#include "opt/printf-stdarg.h"

#define TX_TIMEOUT 0x2ffff


#define SPECTRUM_SIMULATION
#ifdef SPECTRUM_SIMULATION
  #include "simulation.h"
  #include "felics.h"
  #define NOCACHE __attribute__((section(".flexram_nocache"), aligned(32)))
  NOCACHE static int sim_data[384*64];
  //static int sim_data[384*64];
#endif

typedef struct
{
  unsigned random_seed : 8;
  unsigned frame_size : 24;
} fpga_cmd_t;

typedef struct {
    char letter_T;
    char letter_C;
    char letter_M;
    unsigned char n_loop;
    unsigned int frame_size;
    unsigned int cntr;
    unsigned int sim_amplitude;
    unsigned int sim_noise;
    unsigned int felics_dmin;
    unsigned int crc; 
} telecommand_t;
#define TC_SIZE sizeof(telecommand_t)


static send_desc_t* make_fpga_cmd(send_desc_t *send_desc, unsigned int frame_size) {
    fpga_cmd_t *fpga_cmd;

    fpga_cmd = (fpga_cmd_t*)(send_desc->str.daddr);
    fpga_cmd->frame_size = frame_size;
    fpga_cmd->random_seed = 0;
    send_desc->str.dsize = sizeof(fpga_cmd_t);
    send_desc->str.dcrc = 1;
    send_desc->str.hsize = 0;
    send_desc->str.rb1 = 1;
    send_desc->str.rsize = 1;
    return send_desc;
}

send_desc_t* make_telemetry(char *fpga_data, unsigned int n, send_desc_t *send_desc) {
    char err;
    char *header;
    
    header = (char*)(send_desc->str.haddr);
    header[0] = 'T';
    header[1] = 'M';
    header[2] = 'Y';
    
    send_desc->str.daddr = fpga_data;
    send_desc->str.dsize = n;
    send_desc->str.dcrc = 1;
    send_desc->str.hcrc = 1;
    send_desc->str.hsize = 3;
    send_desc->str.rb1 = 2;
    send_desc->str.rsize = 1;
    send_desc->str.timeout = TX_TIMEOUT;
    
    return send_desc;
}

static send_desc_t* send_fpga_cmd(unsigned int frame_size) {
    send_desc_t *send_desc;

    if (send_desc = spw_get_send_desc()) {
        send_desc = make_fpga_cmd(send_desc, frame_size);
        // spw send
        NVIC_DisableIRQ(SPW_IRQn);
        SPW_REGS->SPW_PKTTX1_NXTSENDADDR = send_desc;
        //SPW_REGS->SPW_PKTTX1_NXTSENDCFG = SPW_PKTTX1_NXTSENDCFG_LEN(1);
        SPW_REGS->SPW_PKTTX1_NXTSENDCFG = SPW_PKTTX1_NXTSENDCFG_LEN(1) | SPW_PKTTX1_NXTSENDCFG_START_STARTNOW;
        NVIC_EnableIRQ(SPW_IRQn);
        return send_desc;
    }
    else
        return 0;
}

static send_desc_t* send_telemetry(char *buf, unsigned int n) {
    send_desc_t *send_desc;

    if (send_desc = spw_get_send_desc()) {
        send_desc = make_telemetry(buf, n, send_desc);
        // spw send
        NVIC_DisableIRQ(SPW_IRQn);
        SPW_REGS->SPW_PKTTX1_NXTSENDADDR = send_desc;
        //SPW_REGS->SPW_PKTTX1_NXTSENDCFG = SPW_PKTTX1_NXTSENDCFG_LEN(1);
        SPW_REGS->SPW_PKTTX1_NXTSENDCFG = SPW_PKTTX1_NXTSENDCFG_LEN(1) | SPW_PKTTX1_NXTSENDCFG_START_STARTNOW;
        NVIC_EnableIRQ(SPW_IRQn);
        return send_desc;
    } 
    else
        return 0;
}

static char is_tc(rcv_desc_t *rcv_desc) {
    unsigned int n;
    char crc;
    char *buf;

    n = rcv_desc->str.dsize;
    crc = rcv_desc->str.crc;
    //printf("is_tc %08x n=%d crc=%d\n", rcv_desc, n, crc);
    if (crc == 0) {
        buf = rcv_desc->str.daddr;
        return (n == TC_SIZE && ! strncmp(buf, "TCM", 3));
    }
    return 0;
}

static char is_fpga_data(rcv_desc_t *rcv_desc) {
    unsigned int n;
    char crc;
    char *buf;

    n = rcv_desc->str.dsize;
    crc = rcv_desc->str.crc;
    //printf("is_fpga_data %08x n=%d crc=%d\n", rcv_desc, n, crc);
    if (crc == 0) {
        buf = rcv_desc->str.daddr;
        return (! strncmp(buf, "SCI", 3));
    }
    return 0;
}

void spw_task(void *pvParameters) {
    rcv_desc_t *rcv_desc;
    rcv_desc_t *last_fpga_rcv_desc = 0;
    unsigned int n;
    char *buf;
    telecommand_t *tc;
    int n_loop;
    unsigned int frame_size;
    send_desc_t *send_desc;
    spw_event_t event;
    unsigned int timeout_cntr = 0;
    unsigned int felics_dmin, sim_noise, sim_amplitude;
    vTaskDelay(100);

    start_spw();

    printf("spw_task started\n");

    while (1) {
        event = wait_spw_event();
        if (event.type == RCV_DESC_EV) {
            rcv_desc = event.p;
            if (is_tc(rcv_desc)) {
                //-- TC received ---------------------------------------------
                buf = rcv_desc->str.daddr;
                tc = buf;
                n_loop = tc->n_loop;
                frame_size = tc->frame_size;
                felics_dmin = tc->felics_dmin;
                sim_noise = tc->sim_noise;
                sim_amplitude = tc->sim_amplitude;
                printf("-> TC n_loop=%d cntr=%d\n", n_loop, tc->cntr);
                printf("      frame_size=%d\n", frame_size);
                
                //print_spw_status();
                release_rcv_desc(rcv_desc);
                
                //-- Send FPGA command ---------------------------------------
                send_desc = send_fpga_cmd(frame_size);
                //printf("%3d Cmd(%08x) ->\n", n_loop, send_desc);

            } else if (is_fpga_data(rcv_desc)) {
                //-- FPGA data received --------------------------------------
                //printf("-> Data (%08x)\n");
                last_fpga_rcv_desc = rcv_desc;
                
                //-- Send FPGA command -----------------------------------
                if (--n_loop > 0) {
                    send_desc = send_fpga_cmd(frame_size);
                    //printf("%3d Cmd(%08x) ->\n", n_loop, send_desc);
                }
                //-- Send TM -------------------------------------------------
                n = rcv_desc->str.dsize;
                buf = rcv_desc->str.daddr;

                #ifdef SPECTRUM_SIMULATION
                  // Spectrum simulation ======================================                
                  sim_spectrum_params_t spectrum_params;
                  spectrum_params.width = 384;
                  spectrum_params.height = 64;
                  spectrum_params.amplitude = sim_amplitude;
                  spectrum_params.noise_bits = sim_noise;
                  n = make_spectrum(sim_data, spectrum_params);
  
                  // Felics compression
                  char felics_err;
                  FelicsHandler felics_h = felics_init(buf, BIG_BUF_SIZE);
                  felics_h.delta_min = felics_dmin;
                  felics_encode_header(&felics_h,
                                       spectrum_params.width,
                                       spectrum_params.height);
                  felics_err = felics_encode_row0(&felics_h,
                                     sim_data,
                                     spectrum_params.width);
                  for (int i=1; i < spectrum_params.height; i++) {
                      if (felics_err) break;
                      felics_err = felics_encode_row(&felics_h,
                                        &sim_data[i     * spectrum_params.width],
                                        &sim_data[(i-1) * spectrum_params.width],
                                        spectrum_params.width);
                  }
                  if (! felics_err) {
                      n = felics_close(&felics_h);
                      // printf("Felics compression %d pxs -> %d bytes\n", n, bs.byte_cntr);
    
                      //-- Send sim_data ----------------------
                      //n *= sizeof(int);
                      //buf = sim_data;
                      //last_fpga_rcv_desc->str.daddr = buf;

                      //-- or Send compressed data ------------
                  } else {
                      n = 0;
                      printf("!!! Felics Error : %d\n", felics_err);
                  }
                  //===========================================================
                #endif

                if (n > 0)
                    send_desc = send_telemetry(buf, n);
                else
                    release_rcv_desc(rcv_desc);



                //printf("TM(%08x) ->\n", send_desc);
            } else {
                printf("Invalid Rx Frame: rcv_desc %08x\n", rcv_desc);
                release_rcv_desc(rcv_desc);
            }

        } else if (event.type == SEND_DESC_EV) {
            send_desc = event.p;
            if (last_fpga_rcv_desc && send_desc->str.daddr == last_fpga_rcv_desc->str.daddr) {
                release_rcv_desc(last_fpga_rcv_desc);
            }
            spw_release_send_desc(send_desc);
            //printf("TxDone (%08x)\n", send_desc);
            
        } else if (event.type == TX_TIMEOUT_EV) {
            printf("Tx Timeout %d\n", ++timeout_cntr);
            send_desc = event.p;
            if (last_fpga_rcv_desc && send_desc->str.daddr == last_fpga_rcv_desc->str.daddr) {
                release_rcv_desc(last_fpga_rcv_desc);
            }
            spw_release_send_desc(send_desc);
            n_loop = 0;
            
        } else {
            printf("Invalid Event Type :%d\n", event.type);
        }
    }
}
