/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-05-24                  the first version
 */

#include <rthw.h>
#include <rtthread.h>
#include "main.h"
#include "board.h"
#include <stdbool.h>
#include <usart.h>

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
/*
 * Please modify RT_HEAP_SIZE if you enable RT_USING_HEAP
 * the RT_HEAP_SIZE max value = (sram size - ZI size), 1024 means 1024 bytes
 */
#define RT_HEAP_SIZE (15*1024)
static rt_uint8_t rt_heap[RT_HEAP_SIZE];

RT_WEAK void *rt_heap_begin_get(void)
{
    return rt_heap;
}

RT_WEAK void *rt_heap_end_get(void)
{
    return rt_heap + RT_HEAP_SIZE;
}
#endif

void SysTick_Handler(void)
{
    rt_interrupt_enter();
    
    rt_tick_increase();

    rt_interrupt_leave();
}

#ifdef RT_USING_FINSH
#include <finsh.h>
static void reboot(uint8_t argc, char **argv)
{
    rt_hw_cpu_reset();
}
MSH_CMD_EXPORT(reboot, Reboot System);
#endif /* RT_USING_FINSH */

/**
 * This function will initial your board.
 */
void rt_hw_board_init(void)
{
    extern void SystemClock_Config(void);
    
    HAL_Init();
    SystemClock_Config();
    SystemCoreClockUpdate();
    /* 
     * 1: OS Tick Configuration
     * Enable the hardware timer and call the rt_os_tick_callback function
     * periodically with the frequency RT_TICK_PER_SECOND. 
     */
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/RT_TICK_PER_SECOND);

    /* Call components board initial (use INIT_BOARD_EXPORT()) */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
    rt_system_heap_init(rt_heap_begin_get(), rt_heap_end_get());
#endif
}

#ifdef RT_USING_CONSOLE
#include <ringbuffer.h>

static ringbuffer_t rx_rb;
static uint8_t rx_buf[512];

uint8_t uart_rx_buffer[1];
static UART_HandleTypeDef* pUartHandle;

// HAL_UART_RxCpltCallback回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    ringbuffer_put(&rx_rb, uart_rx_buffer, 1);
    // 重新启用接收中断
    HAL_UART_Receive_IT(huart, uart_rx_buffer, 1);
}

static int uart_init(void)
{
    pUartHandle = &huart1;
    MX_USART1_UART_Init();

    ringbuffer_init(&rx_rb, rx_buf, sizeof(rx_buf));
    // 启动中断接收
    HAL_UART_Receive_IT(pUartHandle, uart_rx_buffer, 1);
    return 0;
}
INIT_BOARD_EXPORT(uart_init);

static bool user_delegate= false;
void console_user_recv_delegate_set(bool state)
{
    user_delegate = state;
}
void console_user_send(char u8data)
{
    if(user_delegate)
    {
        __HAL_UNLOCK(pUartHandle);
        HAL_UART_Transmit(pUartHandle, (const uint8_t *)&u8data, 1, 1);
    }
}

__inline uint16_t console_user_rx_get(const void* pdata, uint16_t len)
{
    return ringbuffer_get(&rx_rb, (uint8_t*)pdata, len);
}

void rt_hw_console_output(const char *str)
{
    if(user_delegate)
        return;

    rt_size_t i = 0, size = 0;
    char a = '\r';

    __HAL_UNLOCK(pUartHandle);

    size = rt_strlen(str);

    for (i = 0; i < size; i++)
    {
        if (*(str + i) == '\n')
        {
            HAL_UART_Transmit(pUartHandle, (uint8_t *)&a, 1, 1);
        }
        HAL_UART_Transmit(pUartHandle, (uint8_t *)(str + i), 1, 1);
    }
}
#endif

#ifdef RT_USING_FINSH
char rt_hw_console_getchar(void)
{
    /* Note: the initial value of ch must < 0 */
    int ch = -1;

    if(user_delegate)
        return ch;

    uint8_t rx_data;
    if(ringbuffer_get(&rx_rb, &rx_data, 1))
    {
        ch = rx_data;
    }
    else
    {
        rt_thread_mdelay(10);
    }
    return ch;
}
#endif
