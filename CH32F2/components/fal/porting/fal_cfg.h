/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-05-17     armink       the first version
 */

#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_

#include <rtconfig.h>
#include <board.h>

#define FAL_PART_HAS_TABLE_CFG
#define FAL_USING_SFUD_PORT
#define RT_VER_NUM

/* ===================== Flash device Configuration ========================= */
extern const struct fal_flash_dev ch32f2_onchip_flash;
extern struct fal_flash_dev nor_flash0;

/* flash device table */
#define FAL_FLASH_DEV_TABLE                                          \
{                                                                    \
    &ch32f2_onchip_flash,                                           \
    &nor_flash0,                                                     \
}
/* ====================== Partition Configuration ========================== */
#ifdef FAL_PART_HAS_TABLE_CFG
/* partition table */
#include <ezboot_config.h>
#if OTA_MGR_EXTERN_FLASH
#define FAL_OTA_MGR_ADDR		OTA_MGR_DATA_ADDRESS
#define FAL_OTA_MGR_DEV_NAME	"norflash0"
#else
#define FAL_OTA_MGR_ADDR		OTA_MGR_DATA_ADDRESS-0x08000000
#define FAL_OTA_MGR_DEV_NAME    "ch32_onchip"
#endif

#if OTA_IMAGE_EXTERN_FLASH
#define FAL_OTA_IMAGE_ADDR		OTA_IMAGE_ADDRESS
#define FAL_OTA_IMAGE_DEV_NAME	"norflash0"
#else
#define FAL_OTA_IMAGE_ADDR		OTA_IMAGE_ADDRESS-0x08000000
#define FAL_OTA_IMAGE_DEV_NAME	"ch32_onchip"
#endif

#define FAL_APP_ADDR			APP_ADDRESS-0x08000000

#define FAL_PART_TABLE                                                               \
{                                                                                    \
    {FAL_PART_MAGIC_WORD,       "boot",     "ch32_onchip",  0,          12*1024, 0}, \
    {FAL_PART_MAGIC_WORD,       "ota_mgr",  FAL_OTA_MGR_DEV_NAME,  FAL_OTA_MGR_ADDR,     OTA_MGR_REGION_SIZE, 0}, \
    {FAL_PART_MAGIC_WORD,       "app",      "ch32_onchip",  FAL_APP_ADDR,    APP_REGION_SIZE, 0}, \
    {FAL_PART_MAGIC_WORD,       "nvs",      "ch32_onchip",  FAL_APP_ADDR+APP_REGION_SIZE,    2*4096, 0}, \
    {FAL_PART_MAGIC_WORD,       "FlashDB",  "norflash0",    0,          16*1024, 0}, \
    {FAL_PART_MAGIC_WORD,       "ota_image", FAL_OTA_IMAGE_DEV_NAME,   FAL_OTA_IMAGE_ADDR,  OTA_IMAGE_REGION_SIZE, 0}, \
}
#endif /* FAL_PART_HAS_TABLE_CFG */

#endif /* _FAL_CFG_H_ */
