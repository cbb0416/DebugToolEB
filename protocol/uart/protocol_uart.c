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
/******************************************************************************
* Include Files
******************************************************************************/
#include "protocol_uart.h"
#include "crc.h"

/******************************************************************************
* Local Functions
******************************************************************************/
#ifdef __PROTOCOL_UART_MASTER_SUPPORT__
static uint8_t ProtUart_MasterDecode(uint8_t *dat, uint16_t len, ProtUartFrameData_t *res);
#endif  /* __PROTOCOL_UART_MASTER_SUPPORT__ */

#ifdef __PROTOCOL_UART_SLAVE_SUPPORT__
static uint8_t ProtUart_SlaveDecode(uint8_t *dat, uint16_t len, ProtUartFrameData_t *res);
static uint8_t ProtUart_SlaveSetReg(uint16_t id, uint32_t value);
static uint32_t ProtUart_SlaveGetReg(uint16_t id);
#endif  /* __PROTOCOL_UART_SLAVE_SUPPORT__ */

/**
  * @brief  uart receive data decode
  * @note
  * @param  dat: data
  * @param  len: data lenght
  * @param  res: decode result
  * @retval 0: decode error, 1: decode correct
  */
uint8_t ProtUart_Decode(uint8_t *dat, uint16_t len, ProtUartFrameData_t *res)
{
    uint8_t ret = 0;
    uint8_t frame_valid_len = 0;

    //  frame head check
    if (1
#ifdef __PROTOCOL_UART_SLAVE_SUPPORT__
        && (dat[0] != PROTOCOL_UART_M2S_FRAME_HEAD)
#endif  /* __PROTOCOL_UART_SLAVE_SUPPORT__ */
#ifdef __PROTOCOL_UART_MASTER_SUPPORT__
        && (dat[0] != PROTOCOL_UART_S2M_FRAME_HEAD)
#endif  /* __PROTOCOL_UART_MASTER_SUPPORT__ */
       )
    {
        res->mFrameErr |= PROTOCOL_UART_FRAME_ERR_FORMAT;
        return ret;
    }

    //  frame tail check
    if (dat[len - 1] != PROTOCOL_UART_FRAME_TAIL)
    {
        res->mFrameErr |= PROTOCOL_UART_FRAME_ERR_FORMAT;
        return ret;
    }

    //  length check
    res->mOpRegNum = (dat[1] & 0x0F) + 1;
#ifdef __PROTOCOL_UART_SLAVE_SUPPORT__
    if (dat[0] == PROTOCOL_UART_M2S_FRAME_HEAD)
    {
        if ((dat[1] & PROTOCOL_UART_M2S_FUNC_MASK) == PROTOCOL_UART_M2S_FUNC_GET_REG)  //  get register
        {
            frame_valid_len = res->mOpRegNum * PROTOCOL_UART_REG_GET_SIZE + 5;
        }
        else                //  set register
        {
            frame_valid_len = res->mOpRegNum * PROTOCOL_UART_REG_SET_SIZE + 5;
        }
    }
#endif  /* __PROTOCOL_UART_SLAVE_SUPPORT__ */
#ifdef __PROTOCOL_UART_MASTER_SUPPORT__
    if (dat[0] == PROTOCOL_UART_S2M_FRAME_HEAD)
    {
        if ((dat[1] & PROTOCOL_UART_S2M_FUNC_MASK) == PROTOCOL_UART_S2M_FUNC_RESP_STATE)  //  response state information
        {
            frame_valid_len =  PROTOCOL_UART_STATE_RESP_SIZE(res->mOpRegNum) + 5;
        }
        else                //  response register
        {
            frame_valid_len = res->mOpRegNum * PROTOCOL_UART_REG_RESP_SIZE + 5;
        }
    }
#endif  /* __PROTOCOL_UART_SLAVE_SUPPORT__ */

    if (frame_valid_len != len)
    {
        res->mFrameErr |= PROTOCOL_UART_FRAME_ERR_FORMAT;
        return ret;
    }


    //  frame crc check
    if (CRC_Crc16Modbus(&dat[1], len - 2))
    {
        res->mFrameErr |= PROTOCOL_UART_FRAME_ERR_CRC;
        return ret;
    }

    if (dat[0] == PROTOCOL_UART_M2S_FRAME_HEAD)
    {
#ifdef __PROTOCOL_UART_SLAVE_SUPPORT__
        ret = ProtUart_SlaveDecode(dat, len - 3, res);
#endif  /* __PROTOCOL_UART_SLAVE_SUPPORT__ */
    }
    else if (dat[0] == PROTOCOL_UART_S2M_FRAME_HEAD)
    {
#ifdef __PROTOCOL_UART_MASTER_SUPPORT__
        ret = ProtUart_MasterDecode(dat, len - 3, res);
#endif  /* __PROTOCOL_UART_MASTER_SUPPORT__ */
    }

    return ret;
}

#ifdef __PROTOCOL_UART_SLAVE_SUPPORT__
/**
  * @brief  Slave receive data decoding
  * @note
  * @param  dat: data
  * @param  len: data lenght
  * @param  res: decode result
  * @retval 0: decode error, 1: decode correct
  */
static uint8_t ProtUart_SlaveDecode(uint8_t *dat, uint16_t len, ProtUartFrameData_t *res)
{
    uint8_t ret = 0;
    uint8_t i = 0;

    res->mHead = dat[0];
    if ((dat[1] & PROTOCOL_UART_M2S_FUNC_MASK) == PROTOCOL_UART_M2S_FUNC_GET_REG)  //  get register
    {
        res->mFunc = PROTOCOL_UART_M2S_FUNC_GET_REG;
        for (i = 0; i < res->mOpRegNum; i++)
        {
            res->mRegID[i] = (dat[PROTOCOL_UART_REG_GET_SIZE * i + 2] << 8) | dat[PROTOCOL_UART_REG_GET_SIZE * i + 3];
        }
    }
    else        //  set register
    {
        res->mFunc = PROTOCOL_UART_M2S_FUNC_SET_REG;
        for (i = 0; i < res->mOpRegNum; i++)
        {
            res->mRegID[i] = (dat[PROTOCOL_UART_REG_SET_SIZE * i + 2] << 8) | dat[PROTOCOL_UART_REG_SET_SIZE * i + 3];
            res->mRegValue[i] = (dat[PROTOCOL_UART_REG_SET_SIZE * i + 4] << 24)
                                | (dat[PROTOCOL_UART_REG_SET_SIZE * i + 5] << 16)
                                | (dat[PROTOCOL_UART_REG_SET_SIZE * i + 6] << 8)
                                | dat[PROTOCOL_UART_REG_SET_SIZE * i + 7];
        }

        if (dat[1] & PROTOCOL_UART_M2S_RESP_FLAG)
        {
            res->mReqRespFlag = 1;
        }
        else
        {
            res->mReqRespFlag = 0;
        }
    }

    ret = 1;
    return ret;
}

/**
  * @brief  Slave frame data encoding
  * @note
  * @param  dat: encode data
  * @param  frame: frame data
  * @retval data lenght
  */
uint16_t ProtUart_SlaveEncode(uint8_t *dat, ProtUartFrameData_t *frame)
{
    uint16_t ret = 0;
    uint8_t i;
    uint16_t crc = 0;
    uint16_t valid_data_len = 0;

    if ((frame->mOpRegNum > PROTOCOL_UART_REG_NUM_MAX)
        || (frame->mOpRegNum == 0))
    {
        return ret;
    }

    dat[0] = PROTOCOL_UART_S2M_FRAME_HEAD;
    if (frame->mFunc & PROTOCOL_UART_S2M_FUNC_RESP_STATE)   //  response state information
    {
        dat[1] = PROTOCOL_UART_S2M_FUNC_RESP_STATE;

        dat[2] = frame->mState;
        dat[3] = frame->mFrameErr;
        for (i = 0; i < frame->mOpRegNum; i++)
        {
            dat[4 + i * PROTOCOL_UART_REG_ID_SIZE] = frame->mRegID[i] >> 8;
            dat[5 + i * PROTOCOL_UART_REG_ID_SIZE] = frame->mRegID[i];
        }

        valid_data_len = 3 + frame->mOpRegNum * PROTOCOL_UART_REG_ID_SIZE;
    }
    else        //  response register
    {
        dat[1] = PROTOCOL_UART_S2M_FUNC_RESP_REG;
        for (i = 0; i < frame->mOpRegNum; i++)
        {
            dat[2 + i * PROTOCOL_UART_REG_RESP_SIZE] = frame->mRegID[i] >> 8;
            dat[3 + i * PROTOCOL_UART_REG_RESP_SIZE] = frame->mRegID[i];
            dat[4 + i * PROTOCOL_UART_REG_RESP_SIZE] = frame->mRegValue[i] >> 24;
            dat[5 + i * PROTOCOL_UART_REG_RESP_SIZE] = frame->mRegValue[i] >> 16;
            dat[6 + i * PROTOCOL_UART_REG_RESP_SIZE] = frame->mRegValue[i] >> 8;
            dat[7 + i * PROTOCOL_UART_REG_RESP_SIZE] = frame->mRegValue[i];
        }

        valid_data_len = 1 + frame->mOpRegNum * PROTOCOL_UART_REG_RESP_SIZE;
    }

    dat[1] |= ((frame->mOpRegNum - 1) & 0x0F);

    crc = CRC_Crc16Modbus(&dat[1], valid_data_len);
    dat[valid_data_len + 1] = crc >> 8;
    dat[valid_data_len + 2] = crc;
    dat[valid_data_len + 3] = PROTOCOL_UART_FRAME_TAIL;

    ret = valid_data_len + 4;

    return ret;
}

/**
  * @brief  Slave receive data handle
  * @note
  * @param  dat: respond data
  * @param  frame: frame data
  * @retval respond data lenght
  */
uint16_t ProtUart_SlaveHandle(uint8_t *dat, ProtUartFrameData_t *frame)
{
    uint8_t i;
    uint16_t ret = 0;

    if (frame->mHead != PROTOCOL_UART_M2S_FRAME_HEAD)
    {
        return ret;
    }

    if (frame->mFunc & PROTOCOL_UART_M2S_FUNC_GET_REG)
    {
        for (i = 0; i < frame->mOpRegNum; i++)
        {
            frame->mRegValue[i] = ProtUart_SlaveGetReg(frame->mRegID[i]);
        }
        frame->mFunc = PROTOCOL_UART_S2M_FUNC_RESP_REG;
    }
    else
    {
        for (i = 0; i < frame->mOpRegNum; i++)
        {
            frame->mOpRegErr[i] = ProtUart_SlaveSetReg(frame->mRegID[i], frame->mRegValue[i]);
        }
        frame->mFrameErr = 0;
        frame->mState = 0;
        if (frame->mReqRespFlag)
        {
            frame->mFunc = PROTOCOL_UART_S2M_FUNC_RESP_STATE;
        }
        else
        {
            return 0;
        }
    }

    return ProtUart_SlaveEncode(dat, frame);
}

/**
  * @brief  Set register
  * @note
  * @param  id: register id
  * @param  value: register value
  * @retval 0: set correct, other: occur error
  */
static uint8_t ProtUart_SlaveSetReg(uint16_t id, uint32_t value)
{
    uint8_t ret = 0;
    if (id <= PROTOCOL_UART_REG_ID_SEGMENT)
    {
        AHMIUsrDef_SetReg(id, value);
    }

    return ret;
}

/**
  * @brief  Get register
  * @note
  * @param  id: register id
  * @retval respond register value
  */
static uint32_t ProtUart_SlaveGetReg(uint16_t id)
{
    uint32_t val = 0;

    if (id <= PROTOCOL_UART_REG_ID_SEGMENT)
    {
        return AHMIUsrDef_GetReg(id);
    }
}

/**
  * @brief  Extract frame
  * @note
  * @param  buffer: valid data buffer
  * @param  dat: input data
  * @retval 0: no valid frame, other: length of valid frame.
  */
uint16_t ProtUart_SlaveFrameExtra(uint8_t *buffer, uint8_t dat)
{
    uint16_t ret = 0;

    static FrameExtraState_t SlaveFrameExtraState = FRAME_EXTRA_STATE_HEAD;
    static uint16_t SlaveFrameExtraPosi = 0;
    static uint16_t SlaveFrameExtraLen = 0;

    switch (SlaveFrameExtraState)
    {
        case FRAME_EXTRA_STATE_HEAD:
            if (dat == PROTOCOL_UART_M2S_FRAME_HEAD)
            {
                buffer[0] = PROTOCOL_UART_M2S_FRAME_HEAD;
                SlaveFrameExtraPosi = 1;
                SlaveFrameExtraState = FRAME_EXTRA_STATE_LEN;
            }
            break;

        case FRAME_EXTRA_STATE_LEN:
            buffer[SlaveFrameExtraPosi++] = dat;
            if (dat & PROTOCOL_UART_M2S_FUNC_GET_REG)
            {
                SlaveFrameExtraLen = ((dat & 0x0F) + 1) * 2 + 5;
            }
            else
            {
                SlaveFrameExtraLen = ((dat & 0x0F) + 1) * 6 + 5;
            }
            SlaveFrameExtraState = FRAME_EXTRA_STATE_DATA;
            break;

        case FRAME_EXTRA_STATE_DATA:
            buffer[SlaveFrameExtraPosi++] = dat;
            if (SlaveFrameExtraPosi > SlaveFrameExtraLen)
            {
                SlaveFrameExtraPosi = 0;
                SlaveFrameExtraLen = 0;
                SlaveFrameExtraState = FRAME_EXTRA_STATE_HEAD;
            }

            if ((dat == PROTOCOL_UART_FRAME_TAIL)
                && (SlaveFrameExtraPosi == SlaveFrameExtraLen))
            {
                ret = SlaveFrameExtraLen;
                SlaveFrameExtraPosi = 0;
                SlaveFrameExtraLen = 0;
                SlaveFrameExtraState = FRAME_EXTRA_STATE_HEAD;
            }
            break;

        default:
            SlaveFrameExtraPosi = 0;
            SlaveFrameExtraLen = 0;
            SlaveFrameExtraState = FRAME_EXTRA_STATE_HEAD;
            break;
    }

    return ret;
}
#endif  /* __PROTOCOL_UART_SLAVE_SUPPORT__ */

#ifdef __PROTOCOL_UART_MASTER_SUPPORT__
/**
  * @brief  Master receive data decoding
  * @note
  * @param  dat: data
  * @param  len: data lenght
  * @param  res: decode result
  * @retval 0: decode error, 1: decode correct
  */
static uint8_t ProtUart_MasterDecode(uint8_t *dat, uint16_t len, ProtUartFrameData_t *res)
{
    uint8_t ret = 0;
    uint8_t i = 0;

    res->mHead = dat[0];
    if ((dat[1] & PROTOCOL_UART_S2M_FUNC_MASK) == PROTOCOL_UART_S2M_FUNC_RESP_STATE)  //  response state information
    {
        res->mFunc = PROTOCOL_UART_S2M_FUNC_RESP_STATE;
        res->mState = dat[2];
        res->mFrameErr = dat[3];
        for (i = 0; i < res->mOpRegNum; i++)
        {
            res->mRegID[i] = (dat[PROTOCOL_UART_REG_GET_SIZE * i + 2] << 8) | dat[PROTOCOL_UART_REG_GET_SIZE * i + 3];
        }
    }
    else        //  response register
    {
        res->mFunc = PROTOCOL_UART_S2M_FUNC_RESP_REG;
        for (i = 0; i < res->mOpRegNum; i++)
        {
            res->mRegID[i] = (dat[PROTOCOL_UART_REG_SET_SIZE * i + 2] << 8) | dat[PROTOCOL_UART_REG_SET_SIZE * i + 3];
            res->mRegValue[i] = (dat[PROTOCOL_UART_REG_SET_SIZE * i + 4] << 24)
                                | (dat[PROTOCOL_UART_REG_SET_SIZE * i + 5] << 16)
                                | (dat[PROTOCOL_UART_REG_SET_SIZE * i + 6] << 8)
                                | dat[PROTOCOL_UART_REG_SET_SIZE * i + 7];
        }
    }

    ret = 1;
    return ret;
}

/**
  * @brief  Add register to frame
  * @note   If the frame is get register value, the value can be set freely.
  * @param  frame: frame
  * @param  reg_id: register id
  * @param  value: register value
  * @retval 0: not full, 1: frame full, add fail
  */
uint8_t ProtUart_MasterAddReg(ProtUartFrameData_t *frame, uint16_t reg_id, uint32_t value)
{
    uint8_t ret = 0;

    if (frame->mOpRegNum >= PROTOCOL_UART_REG_NUM_MAX)
    {
        ret = 1;
        return ret;
    }

    frame->mRegID[frame->mOpRegNum] = reg_id;
    frame->mRegValue[frame->mOpRegNum] = value;
    frame->mOpRegNum++;

    return ret;
}

/**
  * @brief  Set response flag in master frame
  * @note
  * @param  frame: frame data
  * @param  resp_flag: 0: No response required, 1: Response required
  * @retval None
  */
void ProtUart_MasterSetResp(ProtUartFrameData_t *frame, uint8_t resp_flag)
{
    if (resp_flag)
    {
        frame->mReqRespFlag = 1;
    }
    else
    {
        frame->mReqRespFlag = 0;
    }
}

/**
  * @brief  Set function flag in master frame
  * @note
  * @param  frame: frame data
  * @param  func: 0: set register, other: get register
  * @retval None
  */
void ProtUart_MasterSetFunc(ProtUartFrameData_t *frame, uint8_t func)
{
    if (func)
    {
        frame->mFunc = PROTOCOL_UART_M2S_FUNC_GET_REG;
    }
    else
    {
        frame->mFunc = PROTOCOL_UART_M2S_FUNC_SET_REG;
    }
}


/**
  * @brief  Master frame data encoding
  * @note
  * @param  dat: encode data
  * @param  frame: frame data
  * @retval data lenght
  */
uint16_t ProtUart_MasterEncode(uint8_t *dat, ProtUartFrameData_t *frame)
{
    uint16_t ret = 0;
    uint8_t i;
    uint16_t crc = 0;
    uint16_t valid_data_len = 0;

    if ((frame->mOpRegNum > PROTOCOL_UART_REG_NUM_MAX)
        || (frame->mOpRegNum == 0))
    {
        return ret;
    }

    dat[0] = PROTOCOL_UART_M2S_FRAME_HEAD;
    if (frame->mFunc & PROTOCOL_UART_M2S_FUNC_GET_REG)   //  get register
    {
        dat[1] = PROTOCOL_UART_M2S_FUNC_GET_REG;

        for (i = 0; i < frame->mOpRegNum; i++)
        {
            dat[2 + i * PROTOCOL_UART_REG_ID_SIZE] = frame->mRegID[i] >> 8;
            dat[3 + i * PROTOCOL_UART_REG_ID_SIZE] = frame->mRegID[i];
        }

        valid_data_len = 1 + frame->mOpRegNum * PROTOCOL_UART_REG_ID_SIZE;
    }
    else        //  set register
    {
        dat[1] = PROTOCOL_UART_M2S_FUNC_SET_REG;
        for (i = 0; i < frame->mOpRegNum; i++)
        {
            dat[2 + i * PROTOCOL_UART_REG_RESP_SIZE] = frame->mRegID[i] >> 8;
            dat[3 + i * PROTOCOL_UART_REG_RESP_SIZE] = frame->mRegID[i];
            dat[4 + i * PROTOCOL_UART_REG_RESP_SIZE] = frame->mRegValue[i] >> 24;
            dat[5 + i * PROTOCOL_UART_REG_RESP_SIZE] = frame->mRegValue[i] >> 16;
            dat[6 + i * PROTOCOL_UART_REG_RESP_SIZE] = frame->mRegValue[i] >> 8;
            dat[7 + i * PROTOCOL_UART_REG_RESP_SIZE] = frame->mRegValue[i];
        }

        valid_data_len = 1 + frame->mOpRegNum * PROTOCOL_UART_REG_SET_SIZE;
    }

    dat[1] |= ((frame->mOpRegNum - 1) & 0x0F);
    if (frame->mReqRespFlag)
    {
        dat[1] |= PROTOCOL_UART_M2S_RESP_FLAG;
    }

    crc = CRC_Crc16Modbus(&dat[1], valid_data_len);
    dat[valid_data_len + 1] = crc >> 8;
    dat[valid_data_len + 2] = crc;
    dat[valid_data_len + 3] = PROTOCOL_UART_FRAME_TAIL;

    ret = valid_data_len + 4;

    return ret;
}

/**
  * @brief  Extract frame
  * @note
  * @param  buffer: valid data buffer
  * @param  dat: input data
  * @retval 0: no valid frame, other: length of valid frame.
  */
uint16_t ProtUart_MasterFrameExtra(uint8_t *buffer, uint8_t dat)
{
    uint16_t ret = 0;

    static FrameExtraState_t FrameExtraState = FRAME_EXTRA_STATE_HEAD;
    static uint16_t FrameExtraPosi = 0;
    static uint16_t FrameExtraLen = 0;

    switch (FrameExtraState)
    {
        case FRAME_EXTRA_STATE_HEAD:
            if (dat == PROTOCOL_UART_S2M_FRAME_HEAD)
            {
                buffer[0] = PROTOCOL_UART_S2M_FRAME_HEAD;
                FrameExtraPosi = 1;
                FrameExtraState = FRAME_EXTRA_STATE_LEN;
            }
            break;

        case FRAME_EXTRA_STATE_LEN:
            buffer[FrameExtraPosi++] = dat;
            if (dat & PROTOCOL_UART_S2M_FUNC_RESP_STATE)
            {
                FrameExtraLen = (dat & 0x0F) + 1 + 7;
            }
            else
            {
                FrameExtraLen = ((dat & 0x0F) + 1) * 6 + 5;
            }
            FrameExtraState = FRAME_EXTRA_STATE_DATA;
            break;

        case FRAME_EXTRA_STATE_DATA:
            buffer[FrameExtraPosi++] = dat;
            if (FrameExtraPosi > FrameExtraLen)
            {
                FrameExtraPosi = 0;
                FrameExtraLen = 0;
                FrameExtraState = FRAME_EXTRA_STATE_HEAD;
            }

            if ((dat == PROTOCOL_UART_FRAME_TAIL)
                && (FrameExtraPosi == FrameExtraLen))
            {
                ret = FrameExtraLen;
                FrameExtraPosi = 0;
                FrameExtraLen = 0;
                FrameExtraState = FRAME_EXTRA_STATE_HEAD;
            }
            break;

        default:
            FrameExtraPosi = 0;
            FrameExtraLen = 0;
            FrameExtraState = FRAME_EXTRA_STATE_HEAD;
            break;
    }

    return ret;
}

#endif  /* __PROTOCOL_UART_MASTER_SUPPORT__ */


/**
  * @brief  Clear frame data
  * @note
  * @param  frame: frame data
  * @retval None
  */
void ProtUart_ClrFrameData(ProtUartFrameData_t *frame)
{
    frame->mFrameErr = 0;
    frame->mFunc = 0;
    frame->mHead = 0;
    frame->mOpRegNum = 0;
    frame->mRegID[0] = 0;
    frame->mRegValue[0] = 0;
    frame->mReqRespFlag = 0;
    frame->mState = 0;
}
