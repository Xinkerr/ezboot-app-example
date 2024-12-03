#ifndef __BOARD_H__
#define __BOARD_H__
#include <stdbool.h>

typedef void (*console_user_recv_hook_t)(char rec_data);

void console_user_recv_hook_set(console_user_recv_hook_t hook);

void console_user_recv_delegate_set(bool state);

void console_user_send(char u8data);

#endif
