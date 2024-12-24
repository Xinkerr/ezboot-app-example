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
* Create Date:   2021/7/5
* Modified Date: 2021/7/5
* Version:2.0
* Github: https://github.com/Xinkerr
* Mail: garyzh@yeah.net    
*
****************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "crc.h"
#include "ymodem.h"

#define ymd_log(format, ...)

//SOH NUM ~NUM PAYLOAD CRC16H CRC16L
#define SOH_PKG_SZ (1+2+128+2)
//STX NUM ~NUM PAYLOAD CRC16H CRC16L
#define STX_PKG_SZ (1+2+1024+2)

enum ymodem_code
{
    YMD_NONE = 0x00,
    YMD_SOH  = 0x01,
    YMD_STX  = 0x02,
    YMD_EOT  = 0x04,
    YMD_ACK  = 0x06,
    YMD_NAK  = 0x15,
    YMD_CODE_C = 0x43,
};

typedef enum
{
    YMD_STOP = 0,
    YMD_INITIATE,
    YMD_BEGIN,
    YMD_RECEIVE,
    YMD_END,
    YMD_ERROR
}ymodem_step_t;

//���͵��ֽ�����
static putc_func_t data_putc = NULL;
//��ȡ����
static read_func_t data_read = NULL;
//�ļ���Ϣ֡���ջص�
static ymodem_file_handler_t rec_file_cb;
//���ݽ���֡�ص�
static ymodem_data_handler_t rec_data_cb;
//����֡�ص�
static ymodem_end_handler_t end_cb;
//����ص�
static ymodem_error_handler_t error_cb; 
//��ȡϵͳʱ�亯��ָ��
static ym_runtime_get_t ym_runtime_get = NULL;


static uint8_t buffer[YMODEM_BUF_SIZE];
static ymodem_step_t recv_step;
//��ǰ���ݽ��յ�ƫ����
static int buf_offset;
//��ǰ���հ��Ĵ�С
static int pkg_size;
//��ǰ��Ч���İ����
static uint8_t pack_num;

//ymodem����Э�鿪ʼ��ʱ��
static uint32_t ymd_start_time;
//ymodem��ǰ��ʼ���հ���ʱ��
static uint32_t ymd_rec_time;

/**
 * @brief  ������ʼ��
 *
 *
 * @return  void
 */
static void param_init(void)
{
    buf_offset = 0;
    pkg_size = 0;
    pack_num = 0;
    recv_step = YMD_STOP;
}

/**
 * @brief  У��֡����
 *
 * @param[in] pack_num�������
 * @param[in] buf�����ݵ�ַ
 *
 * @return  0��OK
 *         -2������Ŵ���
 *         -3: ����ŷ���У�����
 *         -4: CRCУ�����
 */
static int complete_check(uint8_t pack_num, uint8_t* buf)
{
    uint16_t recv_crc;
    int data_size = pkg_size - 5;
//    if(buf[0] != head)
//        return -1;
    if(buf[1] != pack_num)
        return -2;
    if((buf[1] & buf[2]) != 0x00)
        return -3;
    recv_crc = (uint16_t)(*(buf + pkg_size - 2) << 8) | *(buf + pkg_size - 1);
    if(recv_crc != CRC16(buf+3, data_size))
        return -4;
    return 0;
}

/**
 * @brief  ��ȡ�ļ��Ĵ�С
 *
 * @details ͨ�����յ����ַ���ת����ֵ
 *
 * @param[in] src_str�����յ����ļ���Ϣ֡���ݵĵ�ַ
 *
 * @return �ֽڴ�С
 */
static int get_size(char* src_str)
{
    int size;
    char* ptr;
    ptr = src_str + strlen(src_str) + 1;
    size = atoi(ptr);
    return size;
}

static bool timeout_check(uint32_t start_time, uint32_t timeout)
{
    uint32_t tmp;
	
    uint32_t current_tick = ym_runtime_get();
    if(current_tick >= start_time)
    {
        if(current_tick - start_time >= timeout)
        {
            return true;
        }
    }
    else
    {
        tmp = timeout - (0xffffffff-start_time);
        if(current_tick >= tmp)
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief  YmodemЭ��ͷ֡(�ļ���Ϣ֡)�����ֽڼ��
 *
 * @return 0: û�м�⵽
 *         1: ��⵽���ֽ�SOH
 */
static int receive_head(void)
{
    int ret = 0;
    if(data_read == NULL)
    {
        ymd_log("data_read NULL\n"); 
        return 0;
    }
    if(data_read(buffer, 1))
    {         
        if(buffer[0] == YMD_SOH)
        {
            pkg_size = SOH_PKG_SZ;
            buf_offset = 1;
            ret = 1;
        }
    }
    return ret;
}

/**
 * @brief  YmodemЭ��ͷ֡(�ļ���Ϣ֡)���ݴ���
 *
 * @return 0��δ���յ���ȷ������ͷ֡����
 *         1: ���յ�ͷ֡����
 */
static int receive_begin(void)
{   
    int err;
    int len;
    int ret = 0;
    len = data_read(&buffer[buf_offset], SOH_PKG_SZ);
    // if(len == 0)
    //     data_putc('0');
    // else if(len < 10)
    //     data_putc('0'+len);
    // else if(len < 100)
    // {
    //     data_putc('A');
    //     data_putc('0'+ len/10);
    //     data_putc('0'+ len%10);
    // }
    // else if(len < 200)
    // {
    //     data_putc('B');
    //     data_putc('0'+ len/100);
    //     data_putc('0'+ len%100/10);
    //     data_putc('0'+ len%10);
    // }
    // else if(len < 300)
    // {
    //     data_putc('E');
    //     data_putc('2');
    // }
    // else if(len < 400)
    // {
    //     data_putc('E');
    //     data_putc('3');
    // }
    buf_offset += len;
    // data_putc('F');
    // data_putc('0'+ buf_offset/100);
    // data_putc('0'+ buf_offset%100/10);
    // data_putc('0'+ buf_offset%10);
    if(buf_offset >= SOH_PKG_SZ)
    {
        err = complete_check(0, buffer);
        if(err == 0)
        {
            int file_size = get_size((char*)(buffer+3));
            pack_num = 0;
            rec_file_cb((char*)(buffer+3), file_size); 
            ret = 1;
        }
//        else
//        {
//            ymd_log("err:%d\n", err);
//        }
        buf_offset = 0;
    }
    return ret;
}

#if YMODEM_1K_ENABLE
/**
 * @brief  YmodemЭ������֡���մ���
 *
 * @return 0��δ���յ���ȷ����������֡
 *         1: ���յ���ȷ����������֡
 *         2: ���յ�EOT�����ַ�
 */
static int receive_block(void)
{
    int err;
    int len;
    int ret = 0;
    
    if(buf_offset == 0)
    {
        if(data_read(&buffer[0], 1) == 1)
        {
            switch(buffer[0])
            {
                case YMD_SOH:
                    pkg_size = SOH_PKG_SZ;
                    buf_offset = 1;
                    break;
                case YMD_STX:
                    pkg_size = STX_PKG_SZ; 
                    buf_offset = 1;
                    break;
                case YMD_EOT:
                    pkg_size = 0;
                    buf_offset = 0;
                    ret = 2;
                    break;
            }            
        }
    }
    else
    {
        len = data_read(&buffer[buf_offset], pkg_size-1);
        buf_offset += len;
        if(buf_offset >= pkg_size)
        {
            err = complete_check(pack_num+1, buffer);
            if(err == 0)
            {
                pack_num++;
                switch(buffer[0])
                {
                    case YMD_SOH:
                        rec_data_cb(pack_num, buffer+3, 128);
                        ret = 1;
                        break;
                    
                    case YMD_STX:
                        rec_data_cb(pack_num, buffer+3, 1024);
                        ret = 1;
                        break;
                }
            }
            buf_offset = 0;
        }
    }
    return ret;
}
#else
/**
 * @brief  YmodemЭ������֡���մ���
 *
 * @return 0��δ���յ���ȷ����������֡
 *         1: ���յ���ȷ����������֡
 *         2: ���յ�EOT�����ַ�
 */
static int receive_block(void)
{
    int err;
    int len;
    int ret = 0;
    len = data_read(&buffer[buf_offset], SOH_PKG_SZ);
    buf_offset += len;
    
    if(buf_offset > 0)
    {
        switch(buffer[0])
        {
            case YMD_SOH:
                pkg_size = SOH_PKG_SZ;
                if(buf_offset >= SOH_PKG_SZ)
                {
                    err = complete_check(pack_num+1, buffer);
                    if(err == 0)
                    {
                        pack_num++;
                        rec_data_cb(pack_num, buffer+3, 128);
                        ret = 1;
                    }
                    buf_offset = 0;
                }
                break;
            
            case YMD_EOT:
                pkg_size = 0;
                buf_offset = 0;
                ret = 2;
                break;
        }
    }
    return ret;
}
#endif


/**
 * @brief  YmodemЭ�����֡����
 *
 * @return 0��δ���յ���ȷ�����Ľ���֡
 *         1: ���յ�EOT�����ַ�
 *         2: ���յ���ȷ�����Ľ���֡
 */
static int receive_end(void)
{
    int err;
    int len;
    int ret = 0;
    if(buf_offset == 0)
    {
        if(data_read(&buffer[0], 1) == 1)
        {
            //ͨ�����ֽ��ж�֡����
            switch(buffer[0])
            {
                case YMD_SOH:
                    pkg_size = SOH_PKG_SZ;
                    buf_offset = 1;
                    break;

                case YMD_EOT:
                    pkg_size = 0;
                    ret = 1;
                    break;
            }
        }
    }
    else
    {
        len = data_read(&buffer[buf_offset], pkg_size-1);
        buf_offset += len;
        if(buf_offset >= SOH_PKG_SZ)
        {
            err = complete_check(0x00, buffer);
            if(err == 0)
            {
                ret = 2;
            }
            buf_offset = 0;
        }
    }
    return ret;
}

/**
 * @brief  Ymodem������
 *
 * @param[in] err: �������
 *
 * @return void
 */
static void ymodem_error(int err)
{
    param_init();
    //ִ�д���ص�
    error_cb(err);
}

/**
 * @brief  Ymodem��ʼ��
 *
 * @param[in] putc: ע�����ݷ��ͺ���
 * @param[in] read: ע���ȡ���ݺ���
 * @param[in] rec_file_func: ע���ļ���Ϣ֡���ջص�����
 * @param[in] rec_data_func: ע���ļ�����֡���ջص�����
 * @param[in] end_func: ע���ļ����ս���֡�ص�����
 * @param[in] err_func: ע��ymodem���ճ���ص�����
 * @param[in] runtime_get_func: ע��ϵͳʱ���ȡ���� 
 *
 * @return void
 */
void ymodem_init(putc_func_t putc, read_func_t read,
                 ymodem_file_handler_t rec_file_func,
                 ymodem_data_handler_t rec_data_func,
                 ymodem_end_handler_t end_func,
                 ymodem_error_handler_t err_func,
                 ym_runtime_get_t runtime_get_func)
{
    //������ʼ��
    param_init();
    
    data_putc = putc;
    data_read = read;
    rec_file_cb = rec_file_func;
    rec_data_cb = rec_data_func;
    end_cb = end_func;
    error_cb = err_func;
    ym_runtime_get = runtime_get_func;
}

/**
 * @brief  Ymodem���ն�Э�鴦��
 *
 * @return void
 */
int ymodem_recv_process(void)
{
    int result;
    if(ym_runtime_get == NULL)
        return -1;
    if(data_putc == NULL || data_read == NULL)
        return -2;
    
    switch(recv_step)
    {
        case YMD_STOP:
//            param_init();
            break;
        
        case YMD_INITIATE:
            //һ��ʱ����û���յ�SOH����ִ��error�˳��ص�STOP����
            if(timeout_check(ymd_start_time, YMODEM_INITIATE_TIMEOUT))
            {
                ymodem_error(-YMD_INITIATE);
                break;
            }
            result = receive_head();
            if(result)
            {
//                ymd_log("receive SOH\n");
                //���յ�SOHͷ���������ͷ�����ݲ���
                recv_step = YMD_BEGIN;
                //��ȡ��ǰtickֵ
                ymd_rec_time = ym_runtime_get();
            }
            else
            {
                //�����ַ�'C'
                data_putc(YMD_CODE_C);
            }
            break;
            
        case YMD_BEGIN:
            //һ��ʱ����û���յ������װ����ݣ���ִ��error�˳��ص�STOP����
            if(timeout_check(ymd_rec_time, YMODEM_REC_TIMEOUT))
            {
                ymodem_error(-YMD_BEGIN);
                break;
            }
            result = receive_begin();
            if(result)
            {
                //���յ�����ͷ���ļ���Ϣ���ݺ󣬷���ACK��'C'.
                //�������������ݽ��ղ���
                data_putc(YMD_ACK);
                data_putc(YMD_CODE_C);
                recv_step = YMD_RECEIVE;
                //��ȡ��ǰtickֵ
                ymd_rec_time = ym_runtime_get();
            }
            break;
        
        case YMD_RECEIVE:
            //һ��ʱ����û���յ����������ݣ���ִ��error�˳��ص�STOP����
            if(timeout_check(ymd_rec_time, YMODEM_REC_TIMEOUT))
            {
                ymodem_error(-YMD_RECEIVE);
                break;
            }
            result = receive_block();
            if(result == 1)
            {
                //���յ��������ݺ�,����ACK
                data_putc(YMD_ACK);
                //��ȡ��ǰtickֵ
                ymd_rec_time = ym_runtime_get();
            }
            else if(result == 2)
            {
                //���յ��׸�EOT��,����NACK,�������������
                data_putc(YMD_NAK);
                recv_step = YMD_END;
                //��ȡ��ǰtickֵ
                ymd_rec_time = ym_runtime_get();
            }
            break;
        
        case YMD_END:
            //һ��ʱ����û���յ����������ݣ���ִ��error�˳��ص�STOP����
            if(timeout_check(ymd_rec_time, YMODEM_REC_TIMEOUT))
            {
                ymodem_error(-YMD_END);
                break;
            }
            result = receive_end();
            if(result == 1)
            {
                //���յ��ڶ���EOT,����ACK��'C'
                data_putc(YMD_ACK);
                data_putc(YMD_CODE_C);
            }
            else if(result == 2)
            {
                //���յ��������󣬷���ACK
                data_putc(YMD_ACK);
//                recv_step = YMD_STOP;
//                ymd_log("Finish\n");
                end_cb();
                //������ʼ�����ص�STOP����
                param_init();
            }
            break;
            
        case YMD_ERROR:
            param_init();
            break;
    }
	return 0;
}

/**
 * @brief  Ymodem��ʼ���ն˿�ʼִ��
 *
 * @return void
 */
int ymodem_start(void)
{
    //���뷢��'C'��������
    recv_step = YMD_INITIATE;
    if(ym_runtime_get == NULL)
        return -1;
    //��ȡ��ǰtickֵ
    ymd_start_time = ym_runtime_get();
    return 0;
}
