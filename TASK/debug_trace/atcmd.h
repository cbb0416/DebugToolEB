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
#ifndef __ATCMD_H__
#define __ATCMD_H__

/******************************************************************************
* Include Files
******************************************************************************/
#include <string.h>
#include "hal_uart.h"

/******************************************************************************
* Types
******************************************************************************/
typedef signed   char   int8;
typedef unsigned char   uint8;
typedef signed   short  int16;
typedef unsigned short  uint16;
typedef signed   int    int32;
typedef unsigned int    uint32;
typedef unsigned char   bool;
#define TRUE	1
#define FALSE	0


/******************************************************************************
* Macros
******************************************************************************/
#define COMMAND_LINE_SIZE (50)

#define SEMICOLON_CHAR          (';')
#define COMMA_CHAR              (',')
#define DOT_CHAR                ('.')
#define QUOTES_CHAR             ('\"')
#define QUERY_CHAR              ('?')
#define EQUALS_CHAR             ('=')
#define STAR_CHAR               ('*')
#define HASH_CHAR               ('#')
#define SPACE_CHAR              (' ')
#define AMPERSAND_CHAR          ('&')
#define BACK_SLASH_CHAR         (92)
#define PLUS_CHAR               ('+')
#define NULL_CHAR               ('\0')
#define ESC_CHAR                (27)
#define CTRL_Z_CHAR             (26)
#define INTERNATIONAL_PREFIX    ('+')

typedef enum ExtendedOperationTag
{
    INVALID_EXTENDED_OPERATION,
    EXTENDED_ACTION,
    EXTENDED_QUERY,
    EXTENDED_ASSIGN,
    EXTENDED_RANGE,
    NUM_OF_EXTENDED_OPERATIONS
} ExtendedOperation_t;

typedef enum ResultCodeTag
{
    /* Result Codes */
    RESULT_CODE_OK,
    RESULT_CODE_CONNECT,
    RESULT_CODE_RING,
    RESULT_CODE_NO_CARRIER,
    RESULT_CODE_ERROR,
    RESULT_CODE_INV,
    RESULT_CODE_NO_DIALTONE,
    RESULT_CODE_BUSY,
    RESULT_CODE_NO_ANSWER,
    RESULT_CODE_PROCEEDING,
    RESULT_CODE_FCERROR,
    RESULT_CODE_INTERMEDIATE,
    RESULT_CODE_INVALID_COMMAND,
} ResultCode_t;

typedef enum {cmd_type_none, cmd_type_at} cmd_type_t;

typedef struct
{
    uint16  position;
    uint16  length;
    //char  character[COMMAND_LINE_SIZE+1];
    char *character;
} AT_CMD_STRUCT;

typedef ResultCode_t (*VgProcFunc)(AT_CMD_STRUCT *commandBuffer_p);

typedef struct AtCmdControlTag
{
    const char         *string;      /* Command string */
    const VgProcFunc   procFunc;     /* Command line processing function */
} AtCmdControl;

void Atcmd_Prase(HalUart_t* uart, uint8 *dat, uint16 len);
extern ResultCode_t prase_atcmd(char *buf, uint16 len);
ResultCode_t parseCommandBuffer (AtCmdControl const *atCommandTable_p,   AT_CMD_STRUCT  *commandBuffer_p);
static bool isAsciiCode (char c);
bool getExtendedParameter (AT_CMD_STRUCT *commandBuffer_p, int32 *value_p, int32 inValue);
ExtendedOperation_t getExtendedOperation (AT_CMD_STRUCT *commandBuffer_p);
int32 getDecimalValue (AT_CMD_STRUCT *commandBuffer_p);
bool getExtendedString (AT_CMD_STRUCT *commandBuffer_p,
                        char *outString,
                        int16 maxStringLength,
                        int16 *outStringLength);
bool hex_to_value (char c, int8 *value);

#endif  //  __ATCMD_H__
