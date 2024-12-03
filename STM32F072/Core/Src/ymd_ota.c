
#include "main.h"
#include <stdint.h>
#include <string.h>
#include <rtthread.h>
#include <board.h>
#include <ringbuffer.h>
#include <ymodem.h>

static ringbuffer_t recv_rb;
static uint8_t recv_buf[1024];

static void console_user_receive(char rec_data)
{
    ringbuffer_put(&recv_rb, (uint8_t*)&rec_data, 1);
}

static void ymd_putc(uint8_t u8data)
{
    console_user_send((char)u8data);
}

static int ymd_read(uint8_t* pdata, int len)
{
    int get_len = ringbuffer_get(&recv_rb, pdata, len);
    // for(int i=0; i<get_len; i++)
    //     ymd_putc(pdata[i]);
    return get_len;
}

static void ymd_file_handler(char* file, int size)
{
    rt_kprintf("file:%s\nsize:%d\n", file, size);
//    for(int i = 0; i < 128; i++)
//    {
//        printf("%02x ", file[i]);
//    }
//    printf("\n"); 
}

static void ymd_data_handler(uint8_t num, uint8_t* pdata, int len)
{
    rt_kprintf("num:%d\n", num);
}
static void ymd_end_handler(void)
{
    console_user_recv_delegate_set(false);
    rt_kprintf("finish\n");
}

static void ymd_error_handler(int err)
{
    console_user_recv_delegate_set(false);
    rt_kprintf("abort: %d\n", err);
}

static uint32_t ymd_runtime(void)
{
    return rt_tick_get();
}

void ymd_ota_init(void)
{
    ringbuffer_init(&recv_rb, recv_buf, sizeof(recv_buf));
    console_user_recv_hook_set(console_user_receive);
    ymodem_init(ymd_putc, ymd_read, ymd_file_handler,
                ymd_data_handler, ymd_end_handler,
                ymd_error_handler, ymd_runtime);
}

void ymd_ota_process(void)
{
    ymodem_recv_process();
}

static rt_err_t yota_start(uint8_t argc, char **argv)
{
    if(memcmp(argv[1], "-E", strlen(argv[1])) == 0)
    {
        console_user_recv_delegate_set(true);
        ymodem_start();
        return RT_EOK;
    }
    else
    {
        rt_kprintf("Invalid Parameters\n");
        return RT_EINVAL;
    }
}
// MSH_CMD_EXPORT(yota_start, Y-modem OTA start);
MSH_CMD_EXPORT_ALIAS(yota_start, rb, Y-modem OTA start);
