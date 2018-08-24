/******************************************************************************
*
*           Copyright (c) 2017 苏州速显微电子科技有限公司.
*
*******************************************************************************
*  file name:
*  author:              BingBing.Cheng
*  version:             1.00
*  file description:
*******************************************************************************
*  revision history:    date               version                  author
*
*  change summary:
*
******************************************************************************/
/******************************************************************************
* Include Files
******************************************************************************/
#include "dbg.h"
#include "string.h"
#include "stdarg.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "atcmd.h"
#include <stdio.h>
#include "hal_uart.h"

#ifdef __DEBUG_SUPPORT__
extern HalUart_t DbgUart;

/******************************************************************************
* Variables (Extern, Global and Static)
******************************************************************************/
static const dbg_header_t dbg_module_table[34] =
{
    {DBG_RTC,           "RTC" },
    {DBG_ALARM,         "ALARM" },
    {DBG_KEY,           "KEY" },
    {DBG_CAN,           "CAN"   },
    {DBG_GRAPCTRL,      "GRAPCTRL" },
    {DBG_SOC,			"SOC"},
    {DBG_IO,			"IO"},

    {DBG_ALL,           "ALL"   }
};
static uint32_t DbgModuleMask = 0;
static char DbgBuffer[DBG_BUFFER_SIZE];


/******************************************************************************
* Local Functions
******************************************************************************/

/******************************************************************************
* Function    : DBG_Printf
*
* Author      : ChengBingBing
*
* Parameters  : fmt
*
* Return      : None
*
* Description : 调试打印
******************************************************************************/
void DBG_Printf(uint32 dbg_id, const char *fmt, ...)
{
    uint16_t str_len = 0;
    va_list ap;
    uint8 i;

    for (i = 0; dbg_id != 0; i++)
    {
        dbg_id >>= 1;
        dbg_id &= 0x7FFFFFFF;
    }

    vTaskSuspendAll();

    if (i > 0)
    {
        str_len = sprintf(DbgBuffer, "%c%s%c", '[', dbg_module_table[i - 1].hdr_str , ']');
    }
    va_start(ap, fmt);
    str_len += vsnprintf((DbgBuffer + str_len), (DBG_BUFFER_SIZE - str_len), fmt, ap);
    va_end(ap);

    if (str_len <= DBG_BUFFER_SIZE - 1)
    {
        DbgBuffer[str_len] = '\0';
		GCHAL_UART_WriteString(&DbgUart, DbgBuffer);
    }

    xTaskResumeAll();
}

/******************************************************************************
* Function    : DBG_PrintfLn
*
* Author      : ChengBingBing
*
* Parameters  : fmt
*
* Return      : None
*
* Description : 调试打印行
******************************************************************************/
void DBG_PrintfLn(uint32 dbg_id, const char *fmt, ...)
{
    uint16_t str_len = 0;
    va_list ap;
    uint8 i;

    for (i = 0; dbg_id != 0; i++)
    {
        dbg_id >>= 1;
        dbg_id &= 0x7FFFFFFF;
    }

    vTaskSuspendAll();

    if (i > 0)
    {
        str_len = sprintf(DbgBuffer, "%c%s%c", '[', dbg_module_table[i - 1].hdr_str , ']');
    }
    va_start(ap, fmt);
    str_len += vsnprintf((DbgBuffer + str_len), (DBG_BUFFER_SIZE - str_len), fmt, ap);
    va_end(ap);

    if (str_len <= DBG_BUFFER_SIZE - 3)
    {
        DbgBuffer[str_len] = '\r';
        DbgBuffer[str_len + 1] = '\n';
        DbgBuffer[str_len + 2] = '\0';
		GCHAL_UART_WriteString(&DbgUart, DbgBuffer);
    }

    xTaskResumeAll();
}

/******************************************************************************
* Function    : fputc
*
* Author      : ChengBingBing
*
* Parameters  :
*
* Return      :
*
* Description :
******************************************************************************/
int fputc(int ch, FILE *f)
{
    uint8_t dat = (uint8_t)ch;

    GCHAL_UART_Write(&DbgUart, &dat, 1);

    return ch;
}


/******************************************************************************
* Function    : DBG_SetDbgModuleMask
*
* Author      : BingBing.Cheng
*
* Parameters  : mask: 待使能的打印调试模块
*
* Return      : None
*
* Description : 设置使能打印调试模块
******************************************************************************/
void DBG_SetDbgModuleMask( uint32_t mask )
{
    DbgModuleMask = mask;
}

/******************************************************************************
* Function    : DBG_GetDbgModuleMask
*
* Author      : BingBing.Cheng
*
* Parameters  : None
*
* Return      : 使能的打印模块对应mask
*
* Description : 获取当前打印调试的模块
******************************************************************************/
uint32_t DBG_GetDbgModuleMask( void )
{
    return DbgModuleMask;
}

/******************************************************************************
* Function    : DBG_GetModuleType
*
* Author      : BingBing.Cheng
*
* Parameters  : 待比较的字符串
*
* Return      : DBG_NONE或查找到的ModuleType
*
* Description : 根据字符串查找ID
******************************************************************************/
DbgModuleType_enum DBG_GetModuleType(char *str)
{
    DbgModuleType_enum ret = DBG_NONE;
    uint8_t i;

    for (i = 0; i < 32; i++)
    {
        if (!memcmp(str, dbg_module_table[i].hdr_str, strlen(dbg_module_table[i].hdr_str)))
        {
            ret = dbg_module_table[i].dbg_id;
            break;
        }
    }

    return ret;
}

/******************************************************************************
* Function    : PrintAllModuleString
*
* Author      : BingBing.Cheng
*
* Parameters  : None
*
* Return      : None
*
* Description : 打印所有模块名字字符串
******************************************************************************/
void DBG_PrintAllModuleString( void )
{
    uint8_t i;

    printf("Middle and App Layer Module Name: ");
    for (i = 0; i < 34; i++)
    {
        if (dbg_module_table[i].hdr_str != NULL)
        {
            printf("%s", dbg_module_table[i].hdr_str);
        }
        else
        {
            break;
        }
        printf(",");

    }
    printf("\r\n");
}
#endif  /*__DEBUG_SUPPORT__*/
