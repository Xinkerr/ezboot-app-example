#include <rtthread.h>
#include <rtdevice.h>
#include "board.h"

static rt_err_t  drv_uart_configure(struct rt_serial_device *serial, struct serial_configure *cfg);
static rt_err_t  drv_uart_control(struct rt_serial_device *serial, int cmd, void *arg);
static int       drv_uart_putc(struct rt_serial_device *serial, char c);
static int       drv_uart_getc(struct rt_serial_device *serial);
static rt_size_t drv_uart_dma_transmit(struct rt_serial_device *serial, rt_uint8_t *buf, rt_size_t size, int direction);
// 注册一个虚拟串口, 用来连接设备 
const struct rt_uart_ops _uart_opss =
{
    drv_uart_configure,
    drv_uart_control,
    drv_uart_putc,
    drv_uart_getc,
    drv_uart_dma_transmit
};
rt_err_t drv_uart_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    return (RT_EOK);
}
rt_err_t drv_uart_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    return (RT_EOK);
}
int drv_uart_putc(struct rt_serial_device *serial, char c)
{
    return (RT_EOK);
}
int drv_uart_getc(struct rt_serial_device *serial)
{
    return (RT_EOK);
}
rt_size_t drv_uart_dma_transmit(struct rt_serial_device *serial, rt_uint8_t *buf, rt_size_t size, int direction)
{
    return (RT_EOK);
}

static struct rt_serial_device  vserial;

static int virtul_uart_init(void)
{
    struct rt_serial_device *serial;
    serial                  = &vserial;
    serial->ops             = &_uart_opss;
    rt_hw_serial_register(serial,
                            "vuart0",
                            RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                            RT_NULL);
	return 0;
}
INIT_BOARD_EXPORT(virtul_uart_init);
