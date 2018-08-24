#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "hal_uart.h"
#include "protocol_uart.h"
#include "usart.h"
#include "dbg.h"

#define COM_UART_RX_BUFFER_SIZE     1024
#define COM_UART_TX_BUFFER_SIZE     1024
#define COM_UART_RX_HDLR_BUF_SIZE   1024

uint8_t ComUartRxBuffer[COM_UART_RX_BUFFER_SIZE];
uint8_t ComUartTxBuffer[COM_UART_TX_BUFFER_SIZE];
uint8_t ComUartTempBuffer[COM_UART_RX_HDLR_BUF_SIZE];
static ProtUartFrameData_t MasterRecvFrame;

SemaphoreHandle_t ComUartRxSemaphore;

extern HalUart_t DbgUart;

HalUart_t ComUart =
{
    &huart4,
    115200,
    ComUartRxBuffer,
    ComUartTxBuffer,
    COM_UART_RX_BUFFER_SIZE,
    COM_UART_TX_BUFFER_SIZE,
    HAL_UART_RECV_CPLT_TRIG_EVT_CHAR,
    0xC3,
    NULL,
    0,
    &ComUartRxSemaphore,
};

void GenericUartTask(void const *argument)
{
    GCHAL_UART_Init(&ComUart);

    /* Infinite loop */
    for (;;)
    {
        if (xSemaphoreTake(*(ComUart.pRcvCpltSemaphore), portMAX_DELAY) == pdTRUE)
        {
            uint16_t len = 0;
            uint16_t len1 = 0;
            uint8_t res = 0;
            uint8_t i = 0;

            do
            {
                len = GCHAL_UART_Read(&ComUart, &res, 1);
                if (len)
                {
                    len1 = ProtUart_MasterFrameExtra(ComUartTempBuffer, res);
                    if (len1)
                    {
                        res = ProtUart_Decode(ComUartTempBuffer, len1, &MasterRecvFrame);
                        if (res)
                        {
							if (MasterRecvFrame.mFunc & PROTOCOL_UART_S2M_FUNC_RESP_STATE)
							{
	                            DBG_PrintfLn(DBG_NONE, "State: %02X", MasterRecvFrame.mState);
	                            DBG_PrintfLn(DBG_NONE, "Frame Err: %02X", MasterRecvFrame.mFrameErr);
	                            for (i = 0; i < MasterRecvFrame.mOpRegNum; i++)
	                            {
	                                DBG_PrintfLn(DBG_NONE, "Reg Op Err[%d]: %02X", i, MasterRecvFrame.mOpRegErr[i]);
	                            }
							}
							else
							{
								DBG_PrintfLn(DBG_NONE, "");
	                            for (i = 0; i < MasterRecvFrame.mOpRegNum; i++)
	                            {
	                                DBG_PrintfLn(DBG_NONE, "Reg ID: %d, Value: %d", MasterRecvFrame.mRegID[i],MasterRecvFrame.mRegValue[i]);
	                            }
							}
                        }
                    }
                }
            }
            while (len != 0);
        }
    }
}
