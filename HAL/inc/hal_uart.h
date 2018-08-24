/******************************************************************************
*
*           Copyright (c) 2018 ��������΢���ӿƼ����޹�˾.
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
#ifndef __HAL_UART_H__
#define __HAL_UART_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* Include Files
******************************************************************************/
#include "stm32f1xx.h"
#include "freertos.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
#include "string.h"
	
/******************************************************************************
* Macros
******************************************************************************/
#define HAL_UART_NUM                5

/******************************************************************************
* Types
******************************************************************************/
typedef enum
{
	HAL_UART_RECV_CPLT_TRIG_EVT_TIM,			/*< �������ڽ�����ɵ��¼�����Ϊ��ʱ����ʱ */
	HAL_UART_RECV_CPLT_TRIG_EVT_CHAR,			/*< �������ڽ�����ɵ��¼�����Ϊ���յ������ַ� */
	
	HAL_UART_RECV_CPLT_TRIG_EVT_ERR,			/*< �������ڽ�����ɵ��¼����ʹ��� */
} HalUartRecvCpltTrigEvtType_t;

typedef struct
{
	UART_HandleTypeDef* pUartHandle;	
    uint32_t        	mBaudrate;				/*< ������ */
    uint8_t*			pRcvBuffer;				/*< �������ݻ��� */
    uint8_t*			pSndBuffer;				/*< �������ݻ��� */
    uint16_t        	mRcvBufferSize;			/*< �������ݻ����С */
    uint16_t        	mSndBufferSize;			/*< �������ݻ����С */
	HalUartRecvCpltTrigEvtType_t mRecvTrigEvt;	/*< ������������¼����� */
	uint8_t				mRecvCpltTrigChar;		/*< ����������ɵ������ַ� */
    TimerHandle_t* 		pRecvCpltTrigTimer;		/*< �����������ʹ�õĶ�ʱ�� */
	uint32_t			mRecvCpltTrigTime;		/*< ����������ɶ�ʱ����ʱʱ�� */
    SemaphoreHandle_t*  pRcvCpltSemaphore;		/*< ������ɶ�ֵ�ź��������������еȴ��ź���ֵ�ı��ж���û�н������ */
	
	
    //���³�ʼ��ʱ��������
	uint8_t				mRecvCpltFlag;			/*< ������ɱ�ʶ���ɲ�ͣ��ѯ�˱�־λ�ж���û�н������ */
    uint8_t*			pRcvBufHead;
    uint8_t*			pRcvBufTail;
    uint8_t*			pSndBufHead;
    uint8_t*			pSndBufTail;
} HalUart_t;

/******************************************************************************
* External Functions
******************************************************************************/
void GCHAL_UART_Init(HalUart_t *uart);
void GCHAL_UART_Write(HalUart_t *uart, uint8_t *dat, uint16_t len);
void GCHAL_UART_WriteString(HalUart_t *uart, char *str);
uint16_t GCHAL_UART_Read(HalUart_t *uart, uint8_t *dat, uint16_t len);
uint16_t GCHAL_UART_Peek(HalUart_t *uart, uint8_t *dat, uint16_t len);
void GCHAL_UART_RxCpltTimerExpire(TimerHandle_t timer);

#ifdef __cplusplus
}
#endif

#endif  /* __HAL_UART_H__ */
