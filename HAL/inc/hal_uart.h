/******************************************************************************
*
*           Copyright (c) 2018 苏州速显微电子科技有限公司.
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
	HAL_UART_RECV_CPLT_TRIG_EVT_TIM,			/*< 触发串口接收完成的事件类型为定时器超时 */
	HAL_UART_RECV_CPLT_TRIG_EVT_CHAR,			/*< 触发串口接收完成的事件类型为接收到特殊字符 */
	
	HAL_UART_RECV_CPLT_TRIG_EVT_ERR,			/*< 触发串口接收完成的事件类型错误 */
} HalUartRecvCpltTrigEvtType_t;

typedef struct
{
	UART_HandleTypeDef* pUartHandle;	
    uint32_t        	mBaudrate;				/*< 波特率 */
    uint8_t*			pRcvBuffer;				/*< 接收数据缓存 */
    uint8_t*			pSndBuffer;				/*< 发送数据缓存 */
    uint16_t        	mRcvBufferSize;			/*< 接收数据缓存大小 */
    uint16_t        	mSndBufferSize;			/*< 发送数据缓存大小 */
	HalUartRecvCpltTrigEvtType_t mRecvTrigEvt;	/*< 触发接收完成事件类型 */
	uint8_t				mRecvCpltTrigChar;		/*< 触发接收完成的特殊字符 */
    TimerHandle_t* 		pRecvCpltTrigTimer;		/*< 触发接收完成使用的定时器 */
	uint32_t			mRecvCpltTrigTime;		/*< 触发接收完成定时器超时时间 */
    SemaphoreHandle_t*  pRcvCpltSemaphore;		/*< 接收完成二值信号量，可在任务中等待信号量值改变判断有没有接收完成 */
	
	
    //以下初始化时无需设置
	uint8_t				mRecvCpltFlag;			/*< 接收完成标识，可不停查询此标志位判断有没有接收完成 */
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
