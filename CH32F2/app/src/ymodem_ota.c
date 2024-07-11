#include <ymodem.h>
#include <ymodem_ota.h>
#include <fal.h>
#include <ota_mgr.h>

static size_t write_offset;
static const struct fal_partition *part_dev;
static int abort_step = 0;

static enum rym_code _rym_echo_data(
        struct rym_ctx *ctx,
        rt_uint8_t *buf,
        rt_size_t len)
{
    int err = fal_partition_write(part_dev, write_offset, buf, len);
    if(err < 0)
    {
        abort_step = 3;
        return RYM_CODE_CAN;
    }
    write_offset += len;

    return RYM_CODE_ACK;
}

static enum rym_code begin_callback(struct rym_ctx *ctx, rt_uint8_t *buf, rt_size_t len)
{
    write_offset = 0;
    part_dev = NULL;

    part_dev = fal_partition_find("ota_image");
    if(part_dev == NULL)
    {
        abort_step = 1;
        return RYM_CODE_CAN;
    }

    int err = fal_partition_erase_all(part_dev);
    if(err < 0)
    {
        abort_step = 2;
        return RYM_CODE_CAN;
    }

    return RYM_CODE_ACK;
}   

static enum rym_code end_callback(struct rym_ctx *ctx, rt_uint8_t *buf, rt_size_t len)
{
    ota_mgr_state_set(OTA_REQUEST);
    return RYM_CODE_ACK;
}  

rt_err_t rym_cat_to_dev(rt_device_t dev)
{
    struct rym_ctx rctx;
    rt_err_t res;

    res = rym_recv_on_device(&rctx, dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                             begin_callback, _rym_echo_data, end_callback, 30);
    
    return res;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
void ymdota(void)
{
    rt_device_t dev;
    
    rt_console_set_device("vuart0");
    finsh_set_device("vuart0");

    dev = rt_device_find("uart1");
    if (!dev)
    {
        rt_kprintf("could not find idev\n");
    }

    rym_cat_to_dev(dev);

    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
    finsh_set_device(RT_CONSOLE_DEVICE_NAME);

        NVIC_SystemReset();

    if(abort_step)
        rt_kprintf("abort %d\r\n", abort_step);
    else
        NVIC_SystemReset();
}

MSH_CMD_EXPORT(ymdota, Y-modem download OTA file);
#endif
