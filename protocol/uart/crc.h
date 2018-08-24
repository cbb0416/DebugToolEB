/******************************************************************************
*        
*     		Copyright (c) 2018 ��������΢���ӿƼ����޹�˾.   
*        
*******************************************************************************
*  file name:           
*  author:              ChengBingBing
*  version:             1.00
*  file description:    
*******************************************************************************
*  revision history:    date               version                  author
*
*  change summary:
*
******************************************************************************/
#ifndef __CRC_H__
#define __CRC_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* Include Files
******************************************************************************/
#include "stm32f1xx.h"

/**
  * @brief  CRCУ���CRCУ��������
  * @note	1.����ΪCRCУ��ʹ��ʱ��lenΪ��Ч���ݳ���+������Ч���ݺ��2�ֽ�CRCУ����
  *			2.����ΪCRCУ��������ʱ��lenΪ��Ч���ݳ��ȣ�����ֵΪ���ɵ�CRCУ����
  * @param  dat: ������У��������ݻ��У�������
  * @param  len: ������У��������ݻ��У������ݳ���
  * @retval 1. ����ΪCRCУ��ʱ������ֵΪ0��ʾCRCУ��ɹ����������ʧ��
  *         2. ����ΪCRCУ��������ʱ������ֵΪCRCУ����
  */
uint16_t CRC_Crc16Modbus( uint8_t* dat, uint16_t len );

#ifdef __cplusplus
}
#endif

#endif