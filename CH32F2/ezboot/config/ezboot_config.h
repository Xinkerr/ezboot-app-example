#ifndef __BOOT_CONFIG_H__
#define __BOOT_CONFIG_H__

//================================================================COMMON================================================================================
//OTA image
#define CONFIG_OTA_IMAGE_AES128_ENCRYPT     1
#define CONFIG_DECRYPT_BLOCK_MAX            1024
//For security reasons, please modify the KEY and IV, and keep them consistent with those on the ezboot.
#define CONFIG_OTA_IMAGE_AES_KEY            {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10}
#define CONFIG_OTA_IMAGE_AES_IV             {0x10, 0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01}

//NORFLASH config
// #define CONFIG_NORFLASH_CMD_SECTOR_ERASE    0x20
// #define CONFIG_NORFLASH_ERASE_SIZE          4096
// #define CONFIG_NORFLASH_PAGE_SIZE           256

//---------------------------Partition-------------------------------
//bootloader
#define BOOT_ADDRESS                        ((uint32_t)(0x08000000))
// OTA manager
  #define OTA_MGR_EXTERN_FLASH                1
  #define OTA_MGR_DATA_ADDRESS                ((uint32_t)(1024*1024))
  #define OTA_MGR_REGION_SIZE                 ((uint32_t)4096)
//#define OTA_MGR_EXTERN_FLASH                0
//#define OTA_MGR_DATA_ADDRESS                ((uint32_t)(0x08004000)-0x1000)
//#define OTA_MGR_REGION_SIZE                 ((uint32_t)0x1000)
// application
 #define APP_ADDRESS                         ((uint32_t)(0x08004000))
 #define APP_REGION_SIZE                     ((uint32_t)104*1024)
//#define APP_ADDRESS                         ((uint32_t)(0x08004000))
//#define APP_REGION_SIZE                     ((uint32_t)56*1024)
//OTA image
 #define OTA_IMAGE_EXTERN_FLASH              1
 #define OTA_IMAGE_ADDRESS                   ((uint32_t)(1028*1024))
 #define OTA_IMAGE_REGION_SIZE               ((uint32_t)104*1024)
//#define OTA_IMAGE_EXTERN_FLASH              0
//#define OTA_IMAGE_ADDRESS                   ((uint32_t)(0x08012000))
//#define OTA_IMAGE_REGION_SIZE               ((uint32_t)56*1024)

//==================================================================APP=================================================================================
#define CONFIG_DATA_VERSION		            0x00010000
#define CONFIG_OTA_VERSION		            0x00010000

#endif
