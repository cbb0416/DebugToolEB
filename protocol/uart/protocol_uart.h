/******************************************************************************
*
*           Copyright (c) 2018 ��������΢���ӿƼ����޹�˾.
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
#define __PROTOCOL_UART_MASTER_SUPPORT__                    /* ͨ�ô���Э��������֧�� */
//#define __PROTOCOL_UART_SLAVE_SUPPORT__                     /* ͨ�ô���Э��ӻ���֧�� */

#define PROTOCOL_UART_VERSION_MAJOR             1           /* ����Э��汾�� */
#define PROTOCOL_UART_VERSION_MINOR             0

#define PROTOCOL_UART_REG_ID_SEGMENT            10000       /* IDE��ģ��REG IDֵ�ָ�*/

#define PROTOCOL_UART_REG_ID_SIZE               2           /* REG IDռ���ֽ��� */
#define PROTOCOL_UART_REG_VALUE_SIZE            4           /* REGֵռ���ֽ��� */
#define PROTOCOL_UART_FRAME_ERR_SIZE            1           /* ֡�����ֽ��� */
#define PROTOCOL_UART_STATE_SIZE                1           /* ״̬�� */
#define PROTOCOL_UART_REG_SET_SIZE              (PROTOCOL_UART_REG_ID_SIZE + PROTOCOL_UART_REG_VALUE_SIZE)  /* ���üĴ���ʱÿ���Ĵ���ռ���ֽ��� */
#define PROTOCOL_UART_REG_GET_SIZE              (PROTOCOL_UART_REG_ID_SIZE)                                 /* ��ȡ�Ĵ���ʱÿ���Ĵ���ռ���ֽ��� */
#define PROTOCOL_UART_REG_RESP_SIZE             (PROTOCOL_UART_REG_ID_SIZE + PROTOCOL_UART_REG_VALUE_SIZE)  /* �ظ��Ĵ���ʱÿ���Ĵ���ռ���ֽ��� */
#define PROTOCOL_UART_STATE_RESP_SIZE(REG_NUM)  (REG_NUM + PROTOCOL_UART_STATE_SIZE + PROTOCOL_UART_FRAME_ERR_SIZE) /* �ظ�״̬��Ϣʱÿ���Ĵ���ռ���ֽ��� */

#define PROTOCOL_UART_M2S_FUNC_MASK             0x40
#define PROTOCOL_UART_S2M_FUNC_MASK             0x40

#define PROTOCOL_UART_M2S_FUNC_SET_REG          0x00
#define PROTOCOL_UART_M2S_FUNC_GET_REG          (PROTOCOL_UART_M2S_FUNC_MASK)
#define PROTOCOL_UART_S2M_FUNC_RESP_REG         0x00
#define PROTOCOL_UART_S2M_FUNC_RESP_STATE       (PROTOCOL_UART_S2M_FUNC_MASK)

#define PROTOCOL_UART_M2S_RESP_FLAG             0x80

#define PROTOCOL_UART_REG_NUM_MAX               16          /* һ֡���ݰ������ļĴ����� */
#define PROTOCOL_UART_FRAME_SIZE_MIN            7           /* һ֡����ռ����С�ֽ��� */
#define PROTOCOL_UART_FRAME_SIZE_MAX            101         /* һ֡����ռ������ֽ��� */

#define PROTOCOL_UART_M2S_FRAME_HEAD            0xA5        /* ������(�װ�)��ӻ�(ͼ����ʾģ��)��������ʱ֡ͷ */
#define PROTOCOL_UART_S2M_FRAME_HEAD            0x5A        /* �Ӵӻ�(ͼ����ʾģ��)������(�װ�)�ظ�����ʱ֡ͷ */
#define PROTOCOL_UART_FRAME_TAIL                0xC3        /* ֡β */

// ֡�������Ͷ���
#define PROTOCOL_UART_FRAME_ERR_FORMAT          0x40        /* ֡��ʽ����(֡���Ȼ�֡ͷ/β����) */
#define PROTOCOL_UART_FRAME_ERR_CRC             0x80        /* CRCУ����� */

// ���üĴ��������������Ͷ���
#define PROTOCOL_UART_REG_ERR_OUT_OF_RANGE      0x40        /* �������ó�����Χ */
#define PROTOCOL_UART_REG_ERR_CANNOT_SET        0x80        /* �Ĵ�������д */

/******************************************************************************
* Types
******************************************************************************/
typedef enum
{
    PROT_UART_RESP_TYPE_NONE,               /* ����Ҫ�ظ� */
    PROT_UART_RESP_TYPE_REG,                /* ��Ҫ�ظ��Ĵ���ֵ */
    PROT_UART_RESP_TYPE_STATE,              /* ��Ҫ�ظ�״̬��Ϣ */
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
    uint8_t mHead;                                      /*< ֡ͷ: 0xA5(����)/0x5A(�ӷ�) */
    uint8_t mReqRespFlag;                               /*< ����ظ���־(�������üĴ���ʱ��Ч) */
    uint8_t mFunc;                                      /*< ������:
                                                            mHead=0xA5: 0 - ���üĴ�����0x40 - ��ȡ�Ĵ���
                                                            mHead=0x5A: 0 - �ظ��Ĵ���ֵ��0x40 - �ظ�״̬��Ϣ */
    uint8_t mReserved;
    uint8_t mOpRegNum;                                  /*< �������ļĴ�������,��Χ: 1 - 16 */
    uint8_t mState;                                     /*< ״̬��(mHead=0x5A��mFunc=1ʱ��Ч) */
    uint8_t mFrameErr;                                  /*< ֡������(mHead=0x5A��mFunc=1ʱ��Ч) */
    uint8_t mOpRegErr[PROTOCOL_UART_REG_NUM_MAX];       /*< �����Ĵ��������� */
    uint16_t mRegID[PROTOCOL_UART_REG_NUM_MAX];         /*< �������ļĴ���ID */
    uint32_t mRegValue[PROTOCOL_UART_REG_NUM_MAX];      /*< �������ļĴ���ֵ */
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
