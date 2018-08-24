/******************************************************************************
*        
*     		Copyright (c) 2017 苏州速显微电子科技有限公司.   
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
#include "atcmd.h"
#include "atcmd_customer.h"
#include "stdio.h"
#include "stdlib.h"
#include "ctype.h"
#include "stdarg.h"
#include "string.h"
#include "hal_uart.h"

#ifdef __ATCMD_SUPPORT__
/******************************************************************************
* Local Functions
******************************************************************************/
static ResultCode_t custom_command_hdlr(AT_CMD_STRUCT *CommandLine);
cmd_type_t CheckCommandType(char *cmd);


/******************************************************************************
* Constants
******************************************************************************/
const AtCmdControl auAtCommandTable[] =
{
    { (char *)"+MCUCMD",    vgMcuCtl},
    { (char *)"+DBGCFG",	vgLogCfg},
		
    { NULL,                 NULL  }
};

/******************************************************************************
* Process
******************************************************************************/
void Atcmd_Prase(HalUart_t *uart, uint8 *dat, uint16 len)
{
    cmd_type_t cmd_type = cmd_type_none;
    cmd_type = CheckCommandType((char *)dat);

    if (cmd_type_at == cmd_type)
    {
        GCHAL_UART_Write(uart, dat, len);
        if (prase_atcmd((char *)dat, len) == RESULT_CODE_OK)
        {
			GCHAL_UART_WriteString(uart, "OK\r\n");

        }
        else
        {
            GCHAL_UART_WriteString(uart, "ERROR\r\n");
        }
    }
}

cmd_type_t CheckCommandType(char *cmd)
{
    cmd_type_t result_type = cmd_type_none;

    if ((toupper(cmd[0]) == 'A') && (toupper(cmd[1]) == 'T'))
    {
        result_type = cmd_type_at;
    }

    return result_type;
}


ResultCode_t prase_atcmd(char *buf, uint16 len)
{
    ResultCode_t        retcode = RESULT_CODE_ERROR;
    AT_CMD_STRUCT       at_buffer;

    if (CheckCommandType(buf) != cmd_type_at)
    {
        return retcode;
    }

    at_buffer.position = 0;
    at_buffer.length = len;
    at_buffer.character = buf;

    retcode = custom_command_hdlr(&at_buffer);

    return retcode;
}

static ResultCode_t custom_command_hdlr(AT_CMD_STRUCT *CommandLine)
{
    ResultCode_t        retcode;

    retcode = parseCommandBuffer (auAtCommandTable,   CommandLine);

    return retcode;
}

ResultCode_t parseCommandBuffer (AtCmdControl const *atCommandTable_p,   AT_CMD_STRUCT  *commandBuffer_p)
{
    bool             commandFound = FALSE;
    AtCmdControl  const *currCmd;
    char                 commandLength = 0;
    bool             compare;
    int                  commandIndex;
    ResultCode_t         retcode;

    if (commandBuffer_p->length > 0)
    {
        commandFound = FALSE;
        for (currCmd = atCommandTable_p; (commandFound == FALSE) && (currCmd->string != NULL); currCmd++)
        {
            commandLength = strlen(currCmd->string);
            compare = TRUE;
            commandIndex = 2;

            while (((commandIndex - 2) < commandLength) && (compare == TRUE))
            {

                if (toupper (currCmd->string[commandIndex - 2]) !=
                    toupper (commandBuffer_p->character[commandIndex]))
                {
                    compare = FALSE;
                    break;
                }
                else
                {
                    commandIndex++;
                }
            }

            //fix bug jxin 20080107 假如输入两个AT命令，其中一个命令cmd1就是另外一个命令cmd2的前半部分，系统会认为是cmd1
            if (compare)
            {
                if (isAsciiCode(commandBuffer_p->character[commandIndex]))
                {
                    compare = FALSE;
                }
            }

            if (compare == TRUE)
            {
                commandFound = TRUE;
                break;
            }
        }


        if (commandFound == TRUE)
        {
            commandBuffer_p->position += (commandLength + 2);
            retcode = (currCmd->procFunc)(commandBuffer_p);
        }
        else
        {
            retcode = RESULT_CODE_INVALID_COMMAND;
        }
    }
    return retcode;
}

static bool isAsciiCode (char c)
{
    bool result = TRUE;

    if ((c >= 'a') && (c <= 'z'))
    {
    }
    else
    {
        if ((c >= 'A') && (c <= 'Z'))
        {
        }
        else
        {
            result = FALSE;
        }
    }

    return (result);
}

bool getExtendedParameter (AT_CMD_STRUCT *commandBuffer_p,
                           int32 *value_p,
                           int32 inValue)
{
    bool result = TRUE;
    bool useInValue = TRUE;
    int32   value;

    if (commandBuffer_p->position < commandBuffer_p->length - 1)
    {
        if (commandBuffer_p->character[commandBuffer_p->position] == COMMA_CHAR)
        {
            commandBuffer_p->position++;
        }
        else
        {
            if (commandBuffer_p->character[commandBuffer_p->position] != SEMICOLON_CHAR)
            {
                if (isdigit (commandBuffer_p->character[commandBuffer_p->position]))
                {
                    value = getDecimalValue (commandBuffer_p);
                    *value_p = value;
                    useInValue = FALSE;
                    if (commandBuffer_p->position < commandBuffer_p->length - 1)
                    {
                        if (commandBuffer_p->character[commandBuffer_p->position] == COMMA_CHAR)
                        {
                            commandBuffer_p->position++;
                        }
                    }
                }
                else
                {
                    result = FALSE;
                }
            }
        }
    }

    if (useInValue == TRUE)
    {
        *value_p = inValue;
    }

    return (result);
}

ExtendedOperation_t getExtendedOperation (AT_CMD_STRUCT *commandBuffer_p)
{
    ExtendedOperation_t result;

    if (commandBuffer_p->position < commandBuffer_p->length - 1)
    {
        switch (commandBuffer_p->character[commandBuffer_p->position])
        {
            case QUERY_CHAR:  /* AT+...? */
            {
                commandBuffer_p->position++;
                result = EXTENDED_QUERY;
                break;
            }
            case EQUALS_CHAR:  /* AT+...= */
            {
                commandBuffer_p->position++;
                if ((commandBuffer_p->position < commandBuffer_p->length - 1 ) &&
                    (commandBuffer_p->character[commandBuffer_p->position] ==
                     QUERY_CHAR))
                {
                    commandBuffer_p->position++;
                    result = EXTENDED_RANGE;
                }
                else
                {
                    result = EXTENDED_ASSIGN;
                }
                break;
            }
            default:  /* AT+CLTS */
            {
                result = EXTENDED_ACTION;
                break;
            }
        }
    }
    else
    {
        result = EXTENDED_ACTION;
    }

    return (result);
}

bool getExtendedString (AT_CMD_STRUCT *commandBuffer_p,
                        char *outString,
                        int16 maxStringLength,
                        int16 *outStringLength)
{
    bool result = TRUE;
    int16   length = 0;
    int8    first, second;

    *outString = (char)0;
    *outStringLength = 0;

    if ( commandBuffer_p->position < commandBuffer_p->length - 1 )
    {
        if (commandBuffer_p->character[commandBuffer_p->position] == COMMA_CHAR)
        {
            commandBuffer_p->position++;
        }
        else
        {
            if (commandBuffer_p->character[commandBuffer_p->position] != SEMICOLON_CHAR)
            {
                if (commandBuffer_p->character[commandBuffer_p->position] != QUOTES_CHAR)
                {
                    result = FALSE;
                }
                else
                {
                    commandBuffer_p->position++;
                    while ((commandBuffer_p->position < commandBuffer_p->length - 1) &&
                           (commandBuffer_p->character[commandBuffer_p->position] != QUOTES_CHAR) &&
                           (length <= maxStringLength) &&
                           (result == TRUE))
                    {
                        if (commandBuffer_p->character[commandBuffer_p->position] == '\\')
                        {
                            commandBuffer_p->position++;
                            /* here must come two hexa digits */
                            if (commandBuffer_p->position + 2 < commandBuffer_p->length - 1)
                            {
                                if ((hex_to_value (commandBuffer_p->character[commandBuffer_p->position++], &first)) &&
                                    (hex_to_value (commandBuffer_p->character[commandBuffer_p->position++], &second)))
                                {
                                    *outString++ = (char)(16 * first + second);
                                    length++;
                                }
                                else
                                {
                                    result = FALSE;
                                }
                            }
                            else
                            {
                                result = FALSE;
                            }
                        }
                        else
                        {
                            *outString++ = (char)(commandBuffer_p->character[commandBuffer_p->position++]);
                            length++;
                        }
                    }

                    *outStringLength = length;
                    *outString = (char)0;

                    if (((commandBuffer_p->character[commandBuffer_p->position] == QUOTES_CHAR) &&
                         (length <= maxStringLength)) == FALSE)
                    {
                        result = FALSE;
                    }
                    else
                    {
                        commandBuffer_p->position++;
                        if (commandBuffer_p->character[commandBuffer_p->position] == COMMA_CHAR)
                        {
                            commandBuffer_p->position++;
                        }
                    }
                }
            }
        }
    }

    return (result);
}



int32 getDecimalValue (AT_CMD_STRUCT *commandBuffer_p)
{
    int32 value = 0;

    while ((commandBuffer_p->position < commandBuffer_p->length - 1) &&
           (isdigit (commandBuffer_p->character[commandBuffer_p->position])))
    {
        value *= 10;
        value += (commandBuffer_p->character[commandBuffer_p->position] - '0');
        commandBuffer_p->position++;
    }

    return (value);
}

/******************************************************************************
* Function    : hex_to_value
* 
* Author      : ChengBingBing
* 
* Parameters  : 
* 
* Return      : 
* 
* Description : 
******************************************************************************/
bool hex_to_value (char c, int8 *value)
{
    bool result = TRUE;

    if ((c >= '0') && (c <= '9'))
    {
        *value = c - '0';
    }
    else
    {
        if ((c >= 'a') && (c <= 'f'))
        {
            *value = c - 'a' + 10;
        }
        else
        {
            if ((c >= 'A') && (c <= 'F'))
            {
                *value = c - 'A' + 10;
            }
            else
            {
                result = FALSE;
            }
        }
    }

    return (result);
}
#endif	/*__ATCMD_SUPPORT__*/
