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
#include "hal_uart.h"
#include "freertos.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
#include "stm32f1xx_hal_usart.h"

/******************************************************************************
* Variables (Extern, Global and Static)
******************************************************************************/
static HalUart_t *pUart[HAL_UART_NUM] = {NULL, NULL, NULL, NULL, NULL};
static uint8_t RcvData;

/******************************************************************************
* Local Functions
******************************************************************************/
void GCHAL_UART_Init(HalUart_t *uart)
{
    if ((uart->pUartHandle == NULL)
        || (uart->mRecvTrigEvt >= HAL_UART_RECV_CPLT_TRIG_EVT_ERR)
        || ((uart->pRecvCpltTrigTimer == NULL) && (uart->mRecvTrigEvt == HAL_UART_RECV_CPLT_TRIG_EVT_TIM))
        || ((uart->pRcvBuffer == NULL) && (uart->pSndBuffer == NULL))
        || ((uart->pRcvBuffer == NULL) && (uart->mRcvBufferSize == 0))
        || ((uart->pSndBuffer == NULL) && (uart->mSndBufferSize == 0))
        || ((uart->mRecvTrigEvt == HAL_UART_RECV_CPLT_TRIG_EVT_TIM) && (uart->mRecvCpltTrigTime == 0)))
    {
        return;
    }

    if (uart->pRcvCpltSemaphore != NULL)
    {
        *(uart->pRcvCpltSemaphore) = xSemaphoreCreateBinary ();
        if (*(uart->pRcvCpltSemaphore) == NULL)
        {
            return;
        }
    }

    if ((uart->mRecvTrigEvt == HAL_UART_RECV_CPLT_TRIG_EVT_TIM) && (uart->pRecvCpltTrigTimer != NULL))
    {
        *(uart->pRecvCpltTrigTimer) = xTimerCreate("HAL UART RECV TIMER", uart->mRecvCpltTrigTime, pdFALSE, 0, GCHAL_UART_RxCpltTimerExpire);
        if (*(uart->pRecvCpltTrigTimer) == NULL)
        {
            if (*(uart->pRcvCpltSemaphore) != NULL)
            {
                vSemaphoreDelete(*(uart->pRcvCpltSemaphore));
            }

            return;
        }
    }

    if (uart->pUartHandle->Init.BaudRate != uart->mBaudrate)
    {
        uart->pUartHandle->Init.BaudRate = uart->mBaudrate;
        HAL_UART_Init(uart->pUartHandle);
    }

    uart->mRecvCpltFlag = 0;
    uart->pRcvBufHead = uart->pRcvBuffer;
    uart->pRcvBufTail = uart->pRcvBuffer;
    uart->pSndBufHead = uart->pSndBuffer;
    uart->pSndBufTail = uart->pSndBuffer;

    if (uart->pUartHandle->Instance == USART1)
    {
        pUart[0] = uart;
    }
    else if (uart->pUartHandle->Instance == USART2)
    {
        pUart[1] = uart;
    }
    else if (uart->pUartHandle->Instance == USART3)
    {
        pUart[2] = uart;
    }
    else if (uart->pUartHandle->Instance == UART4)
    {
        pUart[3] = uart;
    }
    else if (uart->pUartHandle->Instance == UART5)
    {
        pUart[4] = uart;
    }

    if (uart->pRcvBuffer != NULL)
    {
        HAL_UART_Receive_IT(uart->pUartHandle, &RcvData, 1);
    }
}

void GCHAL_UART_Write(HalUart_t *uart, uint8_t *dat, uint16_t len)
{
    uint16_t i;
    uint16_t send_len;

    if ((uart->pUartHandle == NULL)
        || ((uart->pSndBuffer == NULL) && (uart->mSndBufferSize == 0))
        || (len == 0))
    {
        return;
    }

    for (i = 0; i < len; i++)
    {
        if ((uart->pSndBufHead == uart->pSndBufTail - 1)
            || (uart->pSndBufHead == uart->pSndBufTail + uart->mSndBufferSize - 1))
        {
            break;
        }

        *uart->pSndBufHead = *dat++;
        uart->pSndBufHead ++;
        if (uart->pSndBufHead >= uart->pSndBuffer + uart->mSndBufferSize)
        {
            uart->pSndBufHead = uart->pSndBuffer;
        }
    }

    if ((HAL_UART_GetState(uart->pUartHandle) != HAL_UART_STATE_BUSY_TX)
        && (HAL_UART_GetState(uart->pUartHandle) != HAL_UART_STATE_BUSY_TX_RX))
    {
        if (uart->pSndBufHead > uart->pSndBufTail)
        {
            send_len = uart->pSndBufHead - uart->pSndBufTail;
        }
        else if (uart->pSndBufHead < uart->pSndBufTail)
        {
            send_len =  uart->pSndBuffer + uart->mSndBufferSize - uart->pSndBufTail;
        }
        else
        {
            send_len = 0;
        }

        if (send_len)
        {
            HAL_UART_Transmit_IT(uart->pUartHandle, uart->pSndBufTail, send_len);
            uart->pSndBufTail += send_len;
            if (uart->pSndBufTail >= (uart->pSndBuffer + uart->mSndBufferSize))
            {
                uart->pSndBufTail = uart->pSndBuffer;
            }
        }
    }
}

void GCHAL_UART_WriteString(HalUart_t *uart, char *str)
{
    GCHAL_UART_Write(uart, (uint8_t *)str, strlen(str));
}

uint16_t GCHAL_UART_Read(HalUart_t *uart, uint8_t *dat, uint16_t len)
{
    uint16_t i;

    if ((uart->pUartHandle == NULL)
        || ((uart->pRcvBuffer == NULL) && (uart->mRcvBufferSize == 0))
        || (len == 0))
    {
        return 0;
    }

    if (uart->pRcvBufHead == uart->pRcvBufTail)
    {
        return 0;
    }

    for (i = 0; i < len; i++)
    {
        dat[i] = *uart->pRcvBufTail;

        uart->pRcvBufTail ++;
        if (uart->pRcvBufTail >= uart->pRcvBuffer + uart->mRcvBufferSize)
        {
            uart->pRcvBufTail = uart->pRcvBuffer;
        }

        if (uart->pRcvBufHead == uart->pRcvBufTail)
        {
            break;
        }
    }

    return i + 1;
}

uint16_t GCHAL_UART_Peek(HalUart_t *uart, uint8_t *dat, uint16_t len)
{
    uint16_t i;
    uint8_t *tail = uart->pRcvBufTail;

    if ((uart->pUartHandle == NULL)
        || ((uart->pRcvBuffer == NULL) && (uart->mRcvBufferSize == 0))
        || (len == 0))
    {
        return 0;
    }

    if (uart->pRcvBufHead == tail)
    {
        return 0;
    }

    for (i = 0; i < len; i++)
    {
        dat[i] = *tail;

        tail ++;
        if (tail >= uart->pRcvBuffer + uart->mRcvBufferSize)
        {
            tail = uart->pRcvBuffer;
        }

        if (uart->pRcvBufHead == tail)
        {
            break;
        }
    }

    return i + 1;
}

void GCHAL_UART_RxCpltTimerExpire(TimerHandle_t timer)
{
    uint8_t i;

    for (i = 0; i < HAL_UART_NUM; i++)
    {
        if (pUart[i] == NULL)
        {
            continue;
        }

        if ((timer == *(pUart[i]->pRecvCpltTrigTimer)) && (pUart[i]->mRecvTrigEvt == HAL_UART_RECV_CPLT_TRIG_EVT_TIM))
        {
            pUart[i]->mRecvCpltFlag = 1;
            if ((pUart[i]->pRcvCpltSemaphore != NULL) && (*(pUart[i]->pRcvCpltSemaphore) != NULL))
            {
                xSemaphoreGiveFromISR(*(pUart[i]->pRcvCpltSemaphore), NULL);
            }

            break;
        }
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    uint8_t i = 0;
    uint16_t len = 0;

    for (i = 0; i < HAL_UART_NUM; i++)
    {
        if (pUart[i] == NULL)
        {
            continue;
        }

        if ((pUart[i]->pUartHandle == huart)
            && (HAL_UART_GetState(pUart[i]->pUartHandle) != HAL_UART_STATE_BUSY_TX)
            && (HAL_UART_GetState(pUart[i]->pUartHandle) != HAL_UART_STATE_BUSY_TX_RX))
        {
            if (pUart[i]->pSndBufHead > pUart[i]->pSndBufTail)
            {
                len = pUart[i]->pSndBufHead - pUart[i]->pSndBufTail;
            }
            else if (pUart[i]->pSndBufHead < pUart[i]->pSndBufTail)
            {
                len = pUart[i]->pSndBuffer + pUart[i]->mSndBufferSize - pUart[i]->pSndBufTail;
            }
            else
            {
                len = 0;
            }

            if (len)
            {
                HAL_UART_Transmit_IT(pUart[i]->pUartHandle, pUart[i]->pSndBufTail, len);
                pUart[i]->pSndBufTail += len;
                if (pUart[i]->pSndBufTail >= (pUart[i]->pSndBuffer + pUart[i]->mSndBufferSize))
                {
                    pUart[i]->pSndBufTail = pUart[i]->pSndBuffer;
                }
            }

            break;
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    uint8_t i = 0;

    for (i = 0; i < HAL_UART_NUM; i++)
    {
        if (pUart[i] == NULL)
        {
            continue;
        }

        if (pUart[i]->pUartHandle == huart)
        {
            if ((pUart[i]->mRcvBufferSize == 0) || (pUart[i]->pRcvBuffer == NULL))
            {
                return;
            }

            if (pUart[i]->pRcvBuffer != NULL)
            {
                HAL_UART_Receive_IT(pUart[i]->pUartHandle, &RcvData, 1);
            }


            if (!((pUart[i]->pRcvBufHead == pUart[i]->pRcvBufTail - 1)
                  || (pUart[i]->pRcvBufHead == pUart[i]->pRcvBufTail + pUart[i]->mRcvBufferSize - 1)))
            {
                *pUart[i]->pRcvBufHead = RcvData;
                pUart[i]->pRcvBufHead ++;
                if (pUart[i]->pRcvBufHead >= pUart[i]->pRcvBuffer + pUart[i]->mRcvBufferSize)
                {
                    pUart[i]->pRcvBufHead = pUart[i]->pRcvBuffer;
                }

                switch (pUart[i]->mRecvTrigEvt)
                {
                    case HAL_UART_RECV_CPLT_TRIG_EVT_TIM:
                        xTimerResetFromISR(*(pUart[i]->pRecvCpltTrigTimer), NULL);
                        break;

                    case HAL_UART_RECV_CPLT_TRIG_EVT_CHAR:
                        if (RcvData == pUart[i]->mRecvCpltTrigChar)
                        {
                            pUart[i]->mRecvCpltFlag = 1;
                            if ((pUart[i]->pRcvCpltSemaphore != NULL) && (*(pUart[i]->pRcvCpltSemaphore) != NULL))
                            {
                                xSemaphoreGiveFromISR(*(pUart[i]->pRcvCpltSemaphore), NULL);
                            }
                        }
                        break;

                    default:
                        break;
                }
            }

            break;
        }
    }
}
