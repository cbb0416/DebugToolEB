/******************************************************************************
*
*           Copyright (c) 2017 ��������΢���ӿƼ����޹�˾.
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
* Description : DBG��ʼ��
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
* Description : ���Դ�ӡ
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
* Description : ���Դ�ӡ��
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
* Description : ��ȡLOG״̬
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
* Description : ����ʹ�ܴ�ӡ��LOG
******************************************************************************/
void SetLogState(uint32_t state);

/******************************************************************************
* Function    : DBG_SetDbgModuleMask
*
* Author      : BingBing.Cheng
*
* Parameters  : mask: ��ʹ�ܵĴ�ӡ����ģ��
*
* Return      : None
*
* Description : ����ʹ�ܴ�ӡ����ģ��
******************************************************************************/
void DBG_SetDbgModuleMask( uint32_t mask );

/******************************************************************************
* Function    : DBG_GetDbgModuleMask
*
* Author      : BingBing.Cheng
*
* Parameters  : None
*
* Return      : ʹ�ܵĴ�ӡģ���Ӧmask
*
* Description : ��ȡ��ǰ��ӡ���Ե�ģ��
******************************************************************************/
uint32_t DBG_GetDbgModuleMask( void );

/******************************************************************************
* Function    : DBG_GetModuleType
*
* Author      : BingBing.Cheng
*
* Parameters  : ���Ƚϵ��ַ���
*
* Return      : DBG_NONE����ҵ���ModuleType
*
* Description : �����ַ�������ID
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
* Description : ��ӡ����ģ�������ַ���
******************************************************************************/
void DBG_PrintAllModuleString( void );

#ifdef __cplusplus
}
#endif

#endif  /*__DBG_H__*/
