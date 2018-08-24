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
#ifndef __DBG_H__
#define __DBG_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* Include Files
******************************************************************************/
#include "hal_uart.h"

/******************************************************************************
* Macros
******************************************************************************/
#define DBG_BUFFER_SIZE                     256

/******************************************************************************
* Types
******************************************************************************/
typedef enum
{
    DBG_NONE        = 0x00000000,

    DBG_RTC         = 0x00000001,
    DBG_ALARM       = 0x00000002,
    DBG_KEY         = 0x00000003,
    DBG_CAN         = 0x00000004,
    DBG_GRAPCTRL    = 0x00000010,
    DBG_SOC    		= 0x00000020,
    DBG_IO    		= 0x00000040,

    DBG_ALL         = 0x7FFFFFFF,
} DbgModuleType_enum;

typedef struct
{
    DbgModuleType_enum dbg_id;
    const char *hdr_str;
} dbg_header_t;

/******************************************************************************
* Function    : DBG_Init
*
* Author      : ChengBingBing
*
* Parameters  : None
*
* Return      : None
*
* Description : DBG初始化
******************************************************************************/
void DBG_Init( void );

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
void DBG_Printf(uint32_t dbg_id, const char *fmt, ...);

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
void DBG_PrintfLn(uint32_t dbg_id, const char *fmt, ...);

/******************************************************************************
* Function    : GetLogState
*
* Author      : BingBing.Cheng
*
* Parameters  : None
*
* Return      : None
*
* Description : 获取LOG状态
******************************************************************************/
uint32_t GetLogState( void );

/******************************************************************************
* Function    : SetLogState
*
* Author      : BingBing.Cheng
*
* Parameters  : None
*
* Return      : None
*
* Description : 设置使能打印的LOG
******************************************************************************/
void SetLogState(uint32_t state);

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
void DBG_SetDbgModuleMask( uint32_t mask );

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
uint32_t DBG_GetDbgModuleMask( void );

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
DbgModuleType_enum DBG_GetModuleType(char *str);

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
void DBG_PrintAllModuleString( void );

#ifdef __cplusplus
}
#endif

#endif  /*__DBG_H__*/
