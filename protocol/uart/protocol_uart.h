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
#ifndef __PROTOCOL_UART_H__
#define __PROTOCOL_UART_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* Include Files
******************************************************************************/
#include "stm32f1xx.h"

/******************************************************************************
* Macros
******************************************************************************/
#define __PROTOCOL_UART_MASTER_SUPPORT__                    /* 通用串口协议主机端支持 */
//#define __PROTOCOL_UART_SLAVE_SUPPORT__                     /* 通用串口协议从机端支持 */

#define PROTOCOL_UART_VERSION_MAJOR             1           /* 串口协议版本号 */
#define PROTOCOL_UART_VERSION_MINOR             0

#define PROTOCOL_UART_REG_ID_SEGMENT            10000       /* IDE和模块REG ID值分割*/

#define PROTOCOL_UART_REG_ID_SIZE               2           /* REG ID占用字节数 */
#define PROTOCOL_UART_REG_VALUE_SIZE            4           /* REG值占用字节数 */
#define PROTOCOL_UART_FRAME_ERR_SIZE            1           /* 帧错误字节数 */
#define PROTOCOL_UART_STATE_SIZE                1           /* 状态码 */
#define PROTOCOL_UART_REG_SET_SIZE              (PROTOCOL_UART_REG_ID_SIZE + PROTOCOL_UART_REG_VALUE_SIZE)  /* 设置寄存器时每个寄存器占用字节数 */
#define PROTOCOL_UART_REG_GET_SIZE              (PROTOCOL_UART_REG_ID_SIZE)                                 /* 读取寄存器时每个寄存器占用字节数 */
#define PROTOCOL_UART_REG_RESP_SIZE             (PROTOCOL_UART_REG_ID_SIZE + PROTOCOL_UART_REG_VALUE_SIZE)  /* 回复寄存器时每个寄存器占用字节数 */
#define PROTOCOL_UART_STATE_RESP_SIZE(REG_NUM)  (REG_NUM + PROTOCOL_UART_STATE_SIZE + PROTOCOL_UART_FRAME_ERR_SIZE) /* 回复状态信息时每个寄存器占用字节数 */

#define PROTOCOL_UART_M2S_FUNC_MASK             0x40
#define PROTOCOL_UART_S2M_FUNC_MASK             0x40

#define PROTOCOL_UART_M2S_FUNC_SET_REG          0x00
#define PROTOCOL_UART_M2S_FUNC_GET_REG          (PROTOCOL_UART_M2S_FUNC_MASK)
#define PROTOCOL_UART_S2M_FUNC_RESP_REG         0x00
#define PROTOCOL_UART_S2M_FUNC_RESP_STATE       (PROTOCOL_UART_S2M_FUNC_MASK)

#define PROTOCOL_UART_M2S_RESP_FLAG             0x80

#define PROTOCOL_UART_REG_NUM_MAX               16          /* 一帧数据包含最大的寄存器数 */
#define PROTOCOL_UART_FRAME_SIZE_MIN            7           /* 一帧数据占用最小字节数 */
#define PROTOCOL_UART_FRAME_SIZE_MAX            101         /* 一帧数据占用最大字节数 */

#define PROTOCOL_UART_M2S_FRAME_HEAD            0xA5        /* 从主机(底板)向从机(图形显示模块)发送数据时帧头 */
#define PROTOCOL_UART_S2M_FRAME_HEAD            0x5A        /* 从从机(图形显示模块)向主机(底板)回复数据时帧头 */
#define PROTOCOL_UART_FRAME_TAIL                0xC3        /* 帧尾 */

// 帧错误类型定义
#define PROTOCOL_UART_FRAME_ERR_FORMAT          0x40        /* 帧格式错误(帧长度或帧头/尾错误) */
#define PROTOCOL_UART_FRAME_ERR_CRC             0x80        /* CRC校验错误 */

// 设置寄存器错误类型类型定义
#define PROTOCOL_UART_REG_ERR_OUT_OF_RANGE      0x40        /* 参数设置超出范围 */
#define PROTOCOL_UART_REG_ERR_CANNOT_SET        0x80        /* 寄存器不可写 */

/******************************************************************************
* Types
******************************************************************************/
typedef enum
{
    PROT_UART_RESP_TYPE_NONE,               /* 不需要回复 */
    PROT_UART_RESP_TYPE_REG,                /* 需要回复寄存器值 */
    PROT_UART_RESP_TYPE_STATE,              /* 需要回复状态信息 */
} ProtUartRespType_t;

typedef struct
{
    uint8_t     mState;
    uint8_t     mFrameErr;
    uint8_t     mRegErr[PROTOCOL_UART_REG_NUM_MAX];
} StateInfo_t;

typedef struct
{
    ProtUartRespType_t  mType;
    StateInfo_t         mState;
    uint8_t             mRegNum;
    uint16_t            mRegID[PROTOCOL_UART_REG_NUM_MAX];
    uint32_t            mRegValue[PROTOCOL_UART_REG_NUM_MAX];
} ProtUartMasterResp_t;

typedef struct
{
    uint8_t mHead;                                      /*< 帧头: 0xA5(主发)/0x5A(从发) */
    uint8_t mReqRespFlag;                               /*< 请求回复标志(仅在设置寄存器时有效) */
    uint8_t mFunc;                                      /*< 功能码:
                                                            mHead=0xA5: 0 - 设置寄存器，0x40 - 读取寄存器
                                                            mHead=0x5A: 0 - 回复寄存器值，0x40 - 回复状态信息 */
    uint8_t mReserved;
    uint8_t mOpRegNum;                                  /*< 待操作的寄存器数量,范围: 1 - 16 */
    uint8_t mState;                                     /*< 状态码(mHead=0x5A且mFunc=1时有效) */
    uint8_t mFrameErr;                                  /*< 帧错误码(mHead=0x5A且mFunc=1时有效) */
    uint8_t mOpRegErr[PROTOCOL_UART_REG_NUM_MAX];       /*< 操作寄存器错误码 */
    uint16_t mRegID[PROTOCOL_UART_REG_NUM_MAX];         /*< 待操作的寄存器ID */
    uint32_t mRegValue[PROTOCOL_UART_REG_NUM_MAX];      /*< 待操作的寄存器值 */
} ProtUartFrameData_t;

typedef enum
{
	FRAME_EXTRA_STATE_NONE,
		
	FRAME_EXTRA_STATE_HEAD = FRAME_EXTRA_STATE_NONE,
	FRAME_EXTRA_STATE_LEN,
	FRAME_EXTRA_STATE_DATA,
} FrameExtraState_t;

/******************************************************************************
* Extern Functions
******************************************************************************/
/**
  * @brief  uart receive data decode
  * @note
  * @param  dat: data
  * @param  len: data lenght
  * @param  res: decode result
  * @retval 0: decode error, 1: decode correct
  */
uint8_t ProtUart_Decode(uint8_t *dat, uint16_t len, ProtUartFrameData_t *res);

/**
  * @brief  Slave frame data encoding
  * @note
  * @param  dat: encode data
  * @param  frame: frame data
  * @retval data lenght
  */
uint16_t ProtUart_SlaveEncode(uint8_t *dat, ProtUartFrameData_t *frame);

/**
  * @brief  Slave receive data handle
  * @note
  * @param  dat: respond data
  * @param  frame: frame data
  * @retval respond data lenght
  */
uint16_t ProtUart_SlaveHandle(uint8_t *dat, ProtUartFrameData_t *frame);

/**
  * @brief  Extract frame
  * @note
  * @param  buffer: valid data buffer
  * @param  dat: input data
  * @retval 0: no valid frame, other: length of valid frame.
  */
uint16_t ProtUart_SlaveFrameExtra(uint8_t *buffer, uint8_t dat);

/**
  * @brief  Add register to frame
  * @note   If the frame is get register value, the value can be set freely.
  * @param  frame: frame
  * @param  reg_id: register id
  * @param  value: register value
  * @retval 0: not full, 1: frame full, add fail
  */
uint8_t ProtUart_MasterAddReg(ProtUartFrameData_t *frame, uint16_t reg_id, uint32_t value);

/**
  * @brief  Set response flag in master frame
  * @note
  * @param  frame: frame data
  * @param  resp_flag: 0: No response required, 1: Response required
  * @retval None
  */
void ProtUart_MasterSetResp(ProtUartFrameData_t *frame, uint8_t resp_flag);

/**
  * @brief  Set function flag in master frame
  * @note
  * @param  frame: frame data
  * @param  func: 0: set register, other: get register
  * @retval None
  */
void ProtUart_MasterSetFunc(ProtUartFrameData_t *frame, uint8_t func);

/**
  * @brief  Master frame data encoding
  * @note
  * @param  dat: encode data
  * @param  frame: frame data
  * @retval data lenght
  */
uint16_t ProtUart_MasterEncode(uint8_t *dat, ProtUartFrameData_t *frame);

/**
  * @brief  Extract frame
  * @note
  * @param  buffer: valid data buffer
  * @param  dat: input data
  * @retval 0: no valid frame, other: length of valid frame.
  */
uint16_t ProtUart_MasterFrameExtra(uint8_t *buffer, uint8_t dat);

/**
  * @brief  Clear frame data
  * @note
  * @param  frame: frame data
  * @retval None
  */
void ProtUart_ClrFrameData(ProtUartFrameData_t *frame);

#ifdef __cplusplus
}
#endif

#endif
