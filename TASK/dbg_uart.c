#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "hal_uart.h"
#include "usart.h"
#include "dbg.h"
#include "atcmd.h"

#define DBG_UART_RX_BUFFER_SIZE     256
#define DBG_UART_TX_BUFFER_SIZE     64
#define DBG_UART_TEMP_BUFFER_SIZE   256

uint8_t DbgUartRxBuffer[DBG_UART_RX_BUFFER_SIZE];
uint8_t DbgUartTxBuffer[DBG_UART_TX_BUFFER_SIZE];
uint8_t DbgUartTempBuffer[DBG_UART_TEMP_BUFFER_SIZE];

SemaphoreHandle_t DbgUartRxSemaphore;

HalUart_t DbgUart =
{
    &huart5,
    115200,
    DbgUartRxBuffer,
    DbgUartTxBuffer,
    DBG_UART_RX_BUFFER_SIZE,
    DBG_UART_TX_BUFFER_SIZE,
    HAL_UART_RECV_CPLT_TRIG_EVT_CHAR,
    '\n',
    NULL,
    0,
    &DbgUartRxSemaphore,
};

void DbgUartTask(void const *argument)
{
    GCHAL_UART_Init(&DbgUart);
		//DBG_PrintfLn(DBG_NONE, "DbgUartTask Running...\r\n");
	
    /* Infinite loop */
    for (;;)
    {
        if (xSemaphoreTake(*(DbgUart.pRcvCpltSemaphore), portMAX_DELAY) == pdTRUE)
        {
            uint16_t len = 0;
            len = GCHAL_UART_Read(&DbgUart, DbgUartTempBuffer, DBG_UART_TX_BUFFER_SIZE);
            if (len)
            {
                Atcmd_Prase(&DbgUart, DbgUartTempBuffer, len);
            }
        }
    }
}

