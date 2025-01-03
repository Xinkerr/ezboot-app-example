
#include "main.h"
#include <ezboot_config.h>
#include <stdint.h>
#include <string.h>
#include <rtthread.h>
#include <rthw.h>
#include <board.h>
#include <ringbuffer.h>
#include <ymodem.h>
#include <ota_mgr.h>

static uint32_t write_offset = OTA_IMAGE_ADDRESS;

static void ymd_putc(uint8_t u8data)
{
    console_user_send((char)u8data);
}

static int ymd_read(uint8_t* pdata, int len)
{
    int get_len = console_user_rx_get(pdata, len);
    return get_len;
}

static void ymd_file_handler(char* file, int size)
{
    rt_kprintf("file:%s\nsize:%d\n", file, size);
    ota_mgr_image_erase(OTA_IMAGE_ADDRESS, OTA_IMAGE_REGION_SIZE);
}

static void ymd_data_handler(uint8_t num, uint8_t* pdata, int len)
{
    rt_kprintf("num:%d\n", num);
    ota_mgr_image_write(write_offset, pdata, len);
    write_offset += len;
}
static void ymd_end_handler(void)
{
    console_user_recv_delegate_set(false);
    ota_mgr_state_set(OTA_REQUEST);
    rt_kprintf("finish\n");
    rt_hw_cpu_reset();
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
    ota_mgr_image_hw_init();
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
        write_offset = OTA_IMAGE_ADDRESS;
        ymodem_start();
        return RT_EOK;
    }
    else
    {
        rt_kprintf("Invalid Parameters\n");
        return RT_EINVAL;
    }
}

MSH_CMD_EXPORT_ALIAS(yota_start, rb, Y-modem OTA start);
