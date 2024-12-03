/***************************************************************************
*
* Copyright (c) 2021, Xinkerr
*
* This file is part of tiny-event-driven.
*
* this is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Disclaimer:
* AUTHOR MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
* REGARDING THE SOFTWARE (INCLUDING ANY ACOOMPANYING WRITTEN MATERIALS),
* ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED USE, INCLUDING,
* WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, THE IMPLIED
* WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE OR USE, AND THE IMPLIED
* WARRANTY OF NONINFRINGEMENT.
* AUTHOR SHALL HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT,
* NEGLIGENCE OR OTHERWISE) FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT
* LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION,
* LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS) ARISING FROM USE OR
* INABILITY TO USE THE SOFTWARE, INCLUDING, WITHOUT LIMITATION, ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA,
* SAVINGS OR PROFITS,
* EVEN IF Disclaimer HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* YOU ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THE SOFTWARE TO ACHIEVE YOUR
* INTENDED RESULTS, AND FOR THE INSTALLATION OF, USE OF, AND RESULTS OBTAINED
* FROM, THE SOFTWARE.
*
* LICENSE: LGPL V3.0
* see: http://www.gnu.org/licenses/lgpl-3.0.html
*
* Create Date:   2021/5/12
* Modified Date: 2021/5/12
* Version:2.0
* Github: https://github.com/Xinkerr
* Mail: garyzh@yeah.net
*
****************************************************************************/
#ifndef __YMODEM_H__
#define __YMODEM_H__

/******************************************CONFIG***************************************************/
#define YMODEM_BUF_SIZE                 150     //ymoedm存放一帧数据的buffer
#define YMODEM_INITIATE_TIMEOUT         30000   //发起请求的超时时间 单位同runtime
#define YMODEM_REC_TIMEOUT              1200     //一帧数据的接收超时时间 单位同runtime
#define YMODEM_1K_ENABLE                0       //是否使能1K模式
/***************************************************************************************************/

#include <stdint.h>
typedef void (*putc_func_t)(uint8_t put_data);
typedef int (*read_func_t)(uint8_t* pdata, int len);
typedef void (*ymodem_file_handler_t)(char* file, int size);
typedef void (*ymodem_data_handler_t)(uint8_t num, uint8_t* pdata, int len);
typedef void (*ymodem_end_handler_t)(void);
typedef void (*ymodem_error_handler_t)(int err);
typedef uint32_t (*ym_runtime_get_t)(void);
  
/**
 * @brief  Ymodem初始化
 *
 * @param[in] putc: 注册数据发送函数
 * @param[in] read: 注册读取数据函数
 * @param[in] rec_file_func: 注册文件信息帧接收回调函数
 * @param[in] rec_data_func: 注册文件数据帧接收回调函数
 * @param[in] end_func: 注册文件接收结束帧回调函数
 * @param[in] err_func: 注册ymodem接收出错回调函数
 * @param[in] runtime_get_func: 注册系统时间获取函数 
 *
 * @return void
 */
void ymodem_init(putc_func_t putc, read_func_t read,
                 ymodem_file_handler_t rec_file_func,
                 ymodem_data_handler_t rec_data_func,
                 ymodem_end_handler_t end_func,
                 ymodem_error_handler_t err_func,
                 ym_runtime_get_t runtime_get_func);

/**
 * @brief  Ymodem接收端协议处理
 *
 * @return int
 */
int ymodem_recv_process(void);

/**
 * @brief  Ymodem开始接收端开始执行
 *
 * @return int
 */
int ymodem_start(void);

#endif
