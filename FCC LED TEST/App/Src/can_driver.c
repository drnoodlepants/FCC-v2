// can_driver.c
#include "can_driver.h"

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

HAL_StatusTypeDef CAN_InitDriver(CAN_Driver_t *can)
{
    if (!can) return HAL_ERROR;
    if (hcan1.Instance == NULL || hcan2.Instance == NULL) return HAL_ERROR;

    can->hcan1 = &hcan1;
    can->hcan2 = &hcan2;

    if (HAL_CAN_Start(can->hcan1) != HAL_OK) return HAL_ERROR;
    if (HAL_CAN_Start(can->hcan2) != HAL_OK) return HAL_ERROR;

    return HAL_OK;
}

// Filter tightened to FCC_CAN_ID_MOTOR_TEMP (0x446) per the open item -
// catch-all (mask 0x0000) is no longer needed now that the split is explicit.
HAL_StatusTypeDef CAN_ConfigMotorTempFilter(CAN_HandleTypeDef *hcan)
{
    CAN_FilterTypeDef filter = {0};

    filter.FilterIdHigh         = (FCC_CAN_ID_MOTOR_TEMP << 5);
    filter.FilterIdLow          = 0x0000;
    filter.FilterMaskIdHigh     = 0xFFE0;   // exact-match mask for an 11-bit std ID #(FFE0)
    filter.FilterMaskIdLow      = 0x0000;
    filter.FilterFIFOAssignment = CAN_RX_FIFO0;
    filter.FilterBank           = 0;
    filter.FilterMode           = CAN_FILTERMODE_IDMASK;
    filter.FilterScale          = CAN_FILTERSCALE_32BIT;
    filter.FilterActivation     = CAN_FILTER_ENABLE;
    filter.SlaveStartFilterBank = 14;

    return HAL_CAN_ConfigFilter(hcan, &filter);
}

// catch all for testing, can be removed later
HAL_StatusTypeDef CAN_ConfigCatchAllFilter(CAN_HandleTypeDef *hcan)
{
    CAN_FilterTypeDef filter = {0};

    filter.FilterIdHigh         = 0x0000;
    filter.FilterIdLow          = 0x0000;
    filter.FilterMaskIdHigh     = 0x0000;
    filter.FilterMaskIdLow      = 0x0000;
    filter.FilterFIFOAssignment = CAN_RX_FIFO0;
    filter.FilterBank           = 0;
    filter.FilterMode           = CAN_FILTERMODE_IDMASK;
    filter.FilterScale          = CAN_FILTERSCALE_32BIT;
    filter.FilterActivation     = CAN_FILTER_ENABLE;
    filter.SlaveStartFilterBank = 14;

    return HAL_CAN_ConfigFilter(hcan, &filter);
}

CAN_RxStatus_t CAN_Receive1(CAN_Driver_t *can)
{
    if (!can || !can->hcan1) return CAN_RX_HAL_ERROR;

    if (HAL_CAN_GetRxFifoFillLevel(can->hcan1, CAN_RX_FIFO0) == 0)
        return CAN_RX_EMPTY;   // non-blocking - nothing waiting this poll

    if (HAL_CAN_GetRxMessage(can->hcan1, CAN_RX_FIFO0, &can->rx1, can->rx_data) != HAL_OK)
        return CAN_RX_HAL_ERROR;

    if (can->rx1.StdId != FCC_CAN_ID_MOTOR_TEMP)
        return CAN_RX_WRONG_ID;

    return CAN_RX_OK;
}

HAL_StatusTypeDef CAN_Transmit1(CAN_Driver_t *can)
{
    uint32_t mbox;
    can->tx1.IDE   = CAN_ID_STD;
    can->tx1.RTR   = CAN_RTR_DATA;
    can->tx1.DLC   = can->len;
    can->tx1.StdId = can->id;
    return HAL_CAN_AddTxMessage(can->hcan1, &can->tx1, can->tx_data, &mbox);
}

HAL_StatusTypeDef CAN_Transmit2(CAN_Driver_t *can)
{
    uint32_t mbox;
    can->tx2.IDE   = CAN_ID_STD;
    can->tx2.RTR   = CAN_RTR_DATA;
    can->tx2.DLC   = can->len;
    can->tx2.StdId = can->id;
    return HAL_CAN_AddTxMessage(can->hcan2, &can->tx2, can->tx_data, &mbox);
}

void CAN_16Bit_Deserializer(uint16_t data_in_buf[4], uint8_t rx_data[8])
{
    for (uint8_t i = 0; i < 4; i++) {
        uint8_t byte_idx = i * 2;
        data_in_buf[i] = (uint16_t)((rx_data[byte_idx + 1] << 8) | rx_data[byte_idx]);
    }
}

void CAN_N_Byte_Serializer(uint8_t n, float data_in, uint8_t *output_buf)
{
    switch (n) {
        case 1: {
            data_in = (data_in > 255.0f) ? 255.0f : data_in;
            data_in = (data_in < 0.0f) ? 0.0f : data_in;
            output_buf[0] = (uint8_t)lroundf(data_in);
            break;
        }
        case 2: {
            uint16_t v = (uint16_t)lroundf(data_in);
            output_buf[0] = (uint8_t)(v & 0xff);
            output_buf[1] = (uint8_t)(v >> 8);
            break;
        }
        default:
            return;
    }
}
