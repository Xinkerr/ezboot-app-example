#include "debug.h"
#include <rtthread.h>
#include <rthw.h>
#include <ota_mgr.h>
#include <ezboot_config.h>
#include <stdlib.h>
#include <fal.h>


int main( void )
{
    SystemCoreClockUpdate();
    rt_kprintf( "SystemClk:%d\r\n", SystemCoreClock );
    rt_kprintf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );
    rt_kprintf("Version: %u.%u\r\n", CONFIG_OTA_VERSION >> 16, CONFIG_OTA_VERSION & 0x0000ffff);

    fal_init();

	ota_mgr_state_t state = ota_mgr_state_get();
	rt_kprintf("ota state:%d\r\n", state);

    while( 1 )
    {
        rt_thread_mdelay( 500 );
    }
}
