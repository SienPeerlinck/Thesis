#include "flexcom1.h"
#include "crc.h"
#include "tc_tm.h"
#include "utils.h"
#include "obt.h"
#include "opt/printf-stdarg.h"
#include "task.h"

#define TCT_SET_OBT 0
#define TCT_GET_OBT 1
#define TCT_CALIB_OBT 2
#define TCT_GET_TASK_STATUS 3

#define SLIP_END 0xC0
#define SLIP_ESC 0xDB
#define SLIP_ESC_END 0xDC
#define SLIP_ESC_ESC 0xDD

#define TC_NO_ERROR 0
#define TC_ERR_TIMEOUT 1
#define TC_ERR_BUFFER_OVERFLOW 2
#define TC_ERR_SLIP_ESC 3

#define TC_BUFFER_LENGTH 256
#define TM_BUFFER_LENGTH 1024

static char tc_buffer[TC_BUFFER_LENGTH];
static char tm_buffer[TM_BUFFER_LENGTH];
static char tc_error;
static Obt_t last_tc_obt;

void init_telecommand(void) {
    init_usart1();
}

static int get_tc(TickType_t timeout) {
    int b;
    unsigned int ix = 0;
    char slip_esc_flag = 0;
    tc_error = TC_NO_ERROR;
    char last_tc_obt_valid = 0;

    while ((b = usart1_getchar(timeout)) != -1) {
        if (! last_tc_obt_valid) {
            get_obt(&last_tc_obt);
            last_tc_obt_valid = 1;
        }
        if (b == SLIP_END) return ix;
        if (ix == TC_BUFFER_LENGTH) {
            tc_error = TC_ERR_BUFFER_OVERFLOW;
            return -1;
        }
        if (b == SLIP_ESC) slip_esc_flag = 1;
        else { 
            if (! slip_esc_flag)
                tc_buffer[ix] = (char)b;
            else if (b == SLIP_ESC_END)
                tc_buffer[ix] = SLIP_END;
            else if (b == SLIP_ESC_ESC)
                tc_buffer[ix] = SLIP_ESC;
            else {
                tc_error = TC_ERR_SLIP_ESC;
                return -1;
            }
            slip_esc_flag = 0;
            ix++;
        }
    }
    tc_error = TC_ERR_TIMEOUT;
    return -1;
}

static void send_tm(unsigned int n) {
    unsigned int crc;
    char *p_crc = tm_buffer + n;
    
    crc = crc32(tm_buffer, n, 0);
    // printf("send_tm() crc : %08x\n", crc);
    *p_crc++ = crc & 0xff;
    *p_crc++ = (crc >> 8) & 0xff;
    *p_crc++ = (crc >> 16) & 0xff;
    *p_crc++ = (crc >> 24) & 0xff;

    for (char* p = tm_buffer; p < p_crc; p++) {
        if (*p == SLIP_END) {
            usart1_putchar(SLIP_ESC, -1);
            usart1_putchar(SLIP_ESC_END, -1);
        }
        else if (*p == SLIP_ESC) {
            usart1_putchar(SLIP_ESC, -1);
            usart1_putchar(SLIP_ESC_ESC, -1);
        }
        else usart1_putchar(*p, -1);
    }
    usart1_putchar(SLIP_END, -1);
}

static void tc_get_obt(void)
{
    Obt_t obt;
    unsigned int* tm_wbuf = (unsigned int*) tm_buffer;

    get_obt(&obt);

    tm_wbuf[0] = 0x01796d74;
    tm_wbuf[1] = obt.timr;
    tm_wbuf[2] = obt.calr;
    tm_wbuf[3] = obt.msr;

    send_tm(16);
}

static void tc_get_task_status(void) {
    TaskStatus_t *task_array;
    volatile UBaseType_t task_array_size;  // FixMe: volatile ???
    unsigned int* tm_wbuf = (unsigned int*) tm_buffer;
    char **task_name;

    task_array_size = uxTaskGetNumberOfTasks();
    if (task_array_size * sizeof(TaskStatus_t) < TM_BUFFER_LENGTH - 8) {
        tm_wbuf[0] = 0x03796d74;
        task_array = &tm_wbuf[2];
        task_array_size = uxTaskGetSystemState(task_array,
                                                task_array_size,
                                                &tm_wbuf[1]);
        for (int i=0; i < task_array_size; i++) {
            task_name = &task_array[i].pcTaskName;
            strncpy(task_name, *task_name, 4);
        }
        send_tm(8 + task_array_size * sizeof(TaskStatus_t));
    }
}

void tc_task(void *pvParameters)
{
    int tc_length;
    unsigned int crc;
    Obt_t obt;
    char tc_ok;
    while (1)
    {
        tc_length = get_tc(10);
        if (tc_length == -1) {
            if (tc_error != TC_ERR_TIMEOUT)
                printf("TC error : %d\n", tc_error);
        } else {
            unsigned int crc = crc32(tc_buffer, tc_length, 0);

            if (crc != 0) {
                printf("TC received with bad CRC (%08x)\nDump\n", crc);
                hex_dump(tc_buffer, tc_length);
                continue;
            }

            // Execute TCs
            //----------------------------------------------------------------------
            tc_ok = 0;
            switch (tc_buffer[3])
            {
                case TCT_SET_OBT:
                    if (tc_length == 16) {
                        obt.timr = ((unsigned int*)tc_buffer)[1];
                        obt.calr = ((unsigned int*)tc_buffer)[2];
                        set_obt(&obt);
                        tc_ok = 1;
                    }
                    break;
                case TCT_GET_OBT:
                    if (tc_length == 8) {
                        tc_get_obt();
                        tc_ok = 1;
                    }
                    break;
                case TCT_CALIB_OBT:
                    if (tc_length == 20) {
                        obt.timr = ((unsigned int*)tc_buffer)[1];
                        obt.calr = ((unsigned int*)tc_buffer)[2];
                        obt.msr = ((unsigned int*)tc_buffer)[3];
                        calib_obt(&last_tc_obt, &obt);
                        tc_ok = 1;
                    }
                    break;
                case TCT_GET_TASK_STATUS:
                    if (tc_length == 8) {
                        tc_get_task_status();
                        tc_ok = 1;
                    }
                    break;

                default:
                    break;
            }
            if (! tc_ok) {
                printf("Invalid TC\nDump:\n ");
                hex_dump(tc_buffer, tc_length);
                break;
            }
        }
    }
}


