#ifndef __BOARD_H__
#define __BOARD_H__
#include <stdbool.h>
#include <stdint.h>

typedef void (*console_user_recv_hook_t)(char rec_data);

uint16_t console_user_rx_get(const void* pdata, uint16_t len);

void console_user_recv_delegate_set(bool state);

void console_user_send(char u8data);

#endif
