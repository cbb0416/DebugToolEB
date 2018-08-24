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
#include "atcmd_customer.h"
#include "dbg.h"
#include "protocol_uart.h"

#ifdef __ATCMD_SUPPORT__
/******************************************************************************
* Function    : vgMcuCtl
*
* Author      : ChengBingBing
*
* Parameters  :
*
* Return      :
*
* Description :
******************************************************************************/
ResultCode_t vgMcuCtl(AT_CMD_STRUCT *commandBuffer_p)
{
    ResultCode_t        result = RESULT_CODE_OK;
    ExtendedOperation_t operation = getExtendedOperation (commandBuffer_p);
    int32  value;
    int32 time;

    switch (operation)
    {
        case EXTENDED_ASSIGN: /* AT+MCUCMD= */
        {
            if ((getExtendedParameter (commandBuffer_p, &value, 0) == TRUE))
            {
                switch (value )
                {
                    case 0:
                        DBG_PrintfLn(DBG_NONE, "AHMI_EB Version: MLD02A0E%02dV%02d_BL60", 1, 2);
                        break;
                    case 1:
                        /* AT+MCUCMD=1,func,resp,num,reg1 id,reg1 value,...regN id,regN value: 关闭LOG打印状态
                        * func: 0: 设置寄存器，1: 读取寄存器
                        * resp: 0: 不需要回复，1: 需要回复
                        * num:  待操作寄存器数量
                        * reg id: 待操作寄存器id
                        * reg value: 待操作寄存器值(仅设置寄存器有效，读取寄存器时可给任意值)
                        */
                    {
                        ProtUartFrameData_t MasterRecvFrame;
                        int32_t func;
                        int32_t resp;
                        int32_t num;
                        int32_t reg_id[16];
                        int32_t reg_value[16];
                        uint8_t i;
                        uint16_t len = 0;
                        uint8_t buffer[120];

                        if ((getExtendedParameter (commandBuffer_p, &func, 0) == FALSE)
                            || (func > 1))
                        {
                            result = RESULT_CODE_ERROR;
                            break;
                        }

                        if ((getExtendedParameter (commandBuffer_p, &resp, 0) == FALSE)
                            || (resp > 1))
                        {
                            result = RESULT_CODE_ERROR;
                            break;
                        }

                        if ((getExtendedParameter (commandBuffer_p, &num, 0) == FALSE)
                            || (num > 16)
                            || (num == 0))
                        {
                            result = RESULT_CODE_ERROR;
                            break;
                        }

                        for (i = 0; i < num; i++)
                        {
                            if ((getExtendedParameter(commandBuffer_p, &reg_id[i], 0) == FALSE)
                                || (reg_id[i] == 0)
                                || (reg_id[i] > 0xFFFF))
                            {
                                result = RESULT_CODE_ERROR;
                                break;
                            }

                            if (getExtendedParameter(commandBuffer_p, &reg_value[i], 0) == FALSE)
                            {
                                result = RESULT_CODE_ERROR;
                                break;
                            }

                        }

                        ProtUart_MasterSetFunc(&MasterRecvFrame, (uint8_t)func);
                        ProtUart_MasterSetResp(&MasterRecvFrame, (uint8_t)resp);
                        MasterRecvFrame.mOpRegNum = (uint8_t)num;
                        for (i = 0; i < num; i++)
                        {
                            MasterRecvFrame.mRegID[i] = (uint16_t)reg_id[i];
                            MasterRecvFrame.mRegValue[i] = (uint32_t)reg_value[i];
                        }
                        len = ProtUart_MasterEncode(buffer, &MasterRecvFrame);
                        if (len)
                        {
                            extern HalUart_t ComUart;

                            GCHAL_UART_Write(&ComUart, buffer, len);
                        }
                    }
                    break;
#if 0
                    case 0:
                        /* AT+MCUCMD=0
                         * Get Software Version
                         */
#if (__PRODUCT_BRANDS__ == __PRODUCT_BRANDS_BOLODA__) && defined(__60V_BATT_SUPPORT__)
                        DBG_PrintfLn(DBG_NONE, "AHMI_EB Version: MLD02A0E%02dV%02d_BL60", SW_VERSION_MAJOR, SW_VERSION_MINOR);
#elif (__PRODUCT_BRANDS__ == __PRODUCT_BRANDS_BOLODA__) && defined(__72V_BATT_SUPPORT__)
                        DBG_PrintfLn(DBG_NONE, "AHMI_EB Version: MLD02A0E%02dV%02d_BL72", SW_VERSION_MAJOR, SW_VERSION_MINOR);
#elif (__PRODUCT_BRANDS__ == __PRODUCT_BRANDS_LEVDEO__) && defined(__60V_BATT_SUPPORT__)
                        DBG_PrintfLn(DBG_NONE, "AHMI_EB Version: MLD02A0E%02dV%02d_LD60", SW_VERSION_MAJOR, SW_VERSION_MINOR);
#elif (__PRODUCT_BRANDS__ == __PRODUCT_BRANDS_LEVDEO__) && defined(__72V_BATT_SUPPORT__)
                        DBG_PrintfLn(DBG_NONE, "AHMI_EB Version: MLD02A0E%02dV%02d_LD72", SW_VERSION_MAJOR, SW_VERSION_MINOR);
#elif (__PRODUCT_BRANDS__ == __PRODUCT_BRANDS_BYVIN__) && defined(__60V_BATT_SUPPORT__)
                        DBG_PrintfLn(DBG_NONE, "AHMI_EB Version: MLD02A0E%02dV%02d_BD60", SW_VERSION_MAJOR, SW_VERSION_MINOR);
#elif (__PRODUCT_BRANDS__ == __PRODUCT_BRANDS_BYVIN__) && defined(__72V_BATT_SUPPORT__)
                        DBG_PrintfLn(DBG_NONE, "AHMI_EB Version: MLD02A0E%02dV%02d_BD72", SW_VERSION_MAJOR, SW_VERSION_MINOR);
#endif
                        break;
#ifdef __DEBUG_SUPPORT__
                    case 2:
                        /* AT+MCUCMD=2
                         * Enable Print Log of All Drivers
                         */
                        DBG_SetDbgModuleMask(~0);
                        printf("Drivers Log Enable\r\n");
                        break;
                    case 3:
                        /* AT+MCUCMD=2
                         * Enable Print Log of All Drivers
                         */
                        DBG_SetDbgModuleMask(0);
                        printf("Drivers Log Disable\r\n");
                        break;
#endif  /*__DEBUG_SUPPORT__*/
#ifdef __PARA_MANAGE_SUPPORT__
                    case 4:
                        ParaManage_RestoreFactorySetting();
                        break;
                    case 5:
                        __set_FAULTMASK(1);
                        NVIC_SystemReset();
                        break;
#endif  /*__PARA_MANAGE_SUPPORT__*/

#ifdef  __WWDG_SUPPORT__
                    case 6:
                        while (1);
                        break;
#endif
                    case 7:
                        DBG_PrintfLn(DBG_NONE, "Tick: %d", Tick);
                        break;

                    case 8:
                        DBG_PrintfLn(DBG_NONE, "MotorType: %d", MotorType);
                        break;

                    case 9: /*AT+CMUCMD=9,year,month,day,hour,minute,second,week, 设置RTC时间*/
                    {
                        int32_t value[7];
                        uint8_t i;
                        for (i = 0; i < 7; i++)
                        {
                            if (getExtendedParameter (commandBuffer_p,
                                                      &value[i], 0) == FALSE)
                            {
                                result = RESULT_CODE_ERROR;
                                break;
                            }
                        }

                        if (result != RESULT_CODE_ERROR)
                        {
                            RTC_t rtc;
                            rtc.Year = (uint16_t)value[0];
                            rtc.Month = (uint8_t)value[1];
                            rtc.Day = (uint8_t)value[2];
                            rtc.Hour = (uint8_t)value[3];
                            rtc.Minute = (uint8_t)value[4];
                            rtc.Second = (uint8_t)value[5];
                            rtc.Week = (uint8_t)value[6];
                            RTC_Write(&rtc);
                        }
                    }
                    break;
#endif
                    default:
                        result = RESULT_CODE_ERROR;
                        break;
                }
            }
            break;
        }

        default:
        {
            result = RESULT_CODE_ERROR;
            break;
        }
    }

    return result;
}


/******************************************************************************
* Function    : vgLogCfg
*
* Author      : Jack
*
* Parameters  :
*
* Return      :
*
* Description : LOG配置
******************************************************************************/
ResultCode_t vgLogCfg(AT_CMD_STRUCT *commandBuffer_p)
{
    ResultCode_t        result = RESULT_CODE_OK;
    ExtendedOperation_t operation = getExtendedOperation (commandBuffer_p);
    int32  value;
    uint32 id = 0;
    uint8_t i;

    switch (operation)
    {
        case EXTENDED_ASSIGN: /* AT+LOGCFG= */

            if (getExtendedParameter (commandBuffer_p, &value, 0) == TRUE)
            {
                switch (value)
                {
#ifdef __DEBUG_SUPPORT__
                    case 0:
                        /* AT+DBGCFG=0,"yyyy","yyyy",...,"yyyy": 关闭LOG打印状态
                         * yyyy: 待关闭的驱动层模块，具体模块名称可下发AT+LOGCFG?查询
                         */
                    case 1:
                        /* AT+DBGCFG=1,"yyyy","yyyy",...,"yyyy": 使能LOG打印状态
                         * yyyy: 待使能的驱动层模块，具体模块名称可下发AT+LOGCFG?查询
                         */
                        for (i = 0; i < 32; i++)
                        {
                            int16 len;
                            uint8_t p_cache[10];
                            memset(p_cache, 0 , 10);
                            if (getExtendedString(commandBuffer_p, (char *)p_cache, 11, &len) == TRUE)
                            {
                                uint32_t type = DBG_GetModuleType((char *)p_cache);
                                if (type != DBG_NONE)
                                {
                                    if (value == 0)
                                    {
                                        if (type == DBG_ALL)
                                        {
                                            id = DBG_NONE;
                                        }
                                        else
                                        {
                                            id &= ~type;
                                        }
                                    }
                                    else
                                    {
                                        if (type == DBG_ALL)
                                        {
                                            id = DBG_ALL;
                                        }
                                        else
                                        {
                                            id |= type;
                                        }
                                    }
                                }
                                else
                                {
                                    DBG_PrintfLn(DBG_NONE, "Error Str: %s", p_cache);
                                    result = RESULT_CODE_ERROR;
                                    return result;
                                }
                            }
                            else
                            {
                                result = RESULT_CODE_ERROR;
                                return result;
                            }

                            if (commandBuffer_p->position == commandBuffer_p->length - 2) //\r\n占2字节
                            {
                                break;
                            }
                        }

                        DBG_SetDbgModuleMask(id);

                        break;
#endif/*__DEBUG_SUPPORT__*/

                    default:
                        break;
                }
            }


            break;
        case EXTENDED_QUERY:
#ifdef __DEBUG_SUPPORT__
            DBG_PrintAllModuleString();
#endif
            break;
        default:
            result = RESULT_CODE_ERROR;
            break;
    }

    return result;
}

#endif  /*__ATCMD_SUPPORT__*/
