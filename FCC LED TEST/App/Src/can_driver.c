/**
 * Dual CAN driver for the FCC
 */

#include "can_driver.h"

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;


// TODO: In the future, could reimplement this to work with the error codes of CAN?

/**
 * @brief Initializes the CAN driver. CAN_Init(...) should be automatically called in the MX_CAN_Init() in main.c.
* @param CAN_Driver_t *can
* @return HAL_StatusTypeDef
* @note Auto-gen: fill details.
*/
HAL_StatusTypeDef CAN_InitDriver(CAN_Driver_t *can) {
    if (!can) return HAL_ERROR;
    if (hcan1.Instance == NULL || hcan2.Instance == NULL) return HAL_ERROR;

    can->hcan1 = &hcan1;
    can->hcan2 = &hcan2;

    if (HAL_CAN_Start(can->hcan1) != HAL_OK) {
        return HAL_ERROR;
    }
    if (HAL_CAN_Start(can->hcan2) != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}



/**
 * @brief 				Deserializes rx data from CAN. Assumes rx data is of form: {lowbyte1, highbyte1, lowbyte2, highbyte2, ...}
 * @param data_in_buf	Translated input data from the CAN bus.
 * @param rx_data		Raw input data from the CAN bus.
 * @return return_type	void
 * @note May be depreciated in the future.
 */
void CAN_16Bit_Deserializer(uint16_t data_in_buf[4], uint8_t rx_data[8]) {
	for (uint8_t i = 0; i < 4; i++) {
		uint8_t byte_idx = i * 2;
		data_in_buf[i] =  (uint16_t)((rx_data[byte_idx + 1] << 8) | (rx_data[byte_idx]));
	}
}



/**
 * @brief	More generalized serializer function for serializing a float value into one or two raw bytes.
 * @param 	n -> The number of bytes.
 * @param 	data_in -> The input float data to serialize.
 * @param 	output_buf[2] -> The buffer containing the serialized data. Intended to work with buffer size of 8 (i.e. tx_data buf).
 * @return 	void
 * @note 	This was made due to not knowing exactly how to serialize the CAN data. This may be depreciated in the future.
 */
void CAN_N_Byte_Serializer(uint8_t n, float data_in, uint8_t *output_buf) {
	switch (n) {
	case 1: {
		// 8-bit truncation is a concern
		data_in 		= (data_in > 255.0f) ? 255.0f : data_in;
		data_in 		= (data_in < 0.0f) ? 0.0f : data_in;
		uint8_t data 	= (uint8_t)lroundf(data_in);
		output_buf[0] 	= data;
		break;
	}
	case 2: {
		uint16_t data_in_16u 	= (uint16_t)lroundf(data_in);
		uint8_t high_byte 		= (uint8_t)(data_in_16u >> 8);
		uint8_t low_byte 		= (uint8_t)(data_in_16u & 0xff);
		output_buf[0] = low_byte;
		output_buf[1] = high_byte;
		break;
	}
	default:
		// invalid choice
		return;
	}
	return;

}



/**
 * @brief Transmits a CAN message over to the 1st CAN bus.
 * @param CAN_Driver_t *can
 * @return HAL_StatusTypeDef
 * @note Auto-gen: fill details.
 */
HAL_StatusTypeDef CAN_Transmit1(CAN_Driver_t *can) {

    uint32_t mbox;
    can->tx1.IDE  = CAN_ID_STD;
    can->tx1.RTR  = CAN_RTR_DATA;
    can->tx1.DLC  = can->len;
    can->tx1.StdId= can->id;

    return HAL_CAN_AddTxMessage(can->hcan1, &can->tx1, can->tx_data, &mbox);
}

/**
 * @brief Transmits a CAN message over to the 2nd CAN bus.
* @param CAN_Driver_t *can
* @return HAL_StatusTypeDef
* @note Auto-gen: fill details.
*/
HAL_StatusTypeDef CAN_Transmit2(CAN_Driver_t *can) {

    uint32_t mbox;
    can->tx2.IDE  = CAN_ID_STD;
    can->tx2.RTR  = CAN_RTR_DATA;
    can->tx2.DLC  = can->len;
    can->tx2.StdId= can->id;

    return HAL_CAN_AddTxMessage(can->hcan2, &can->tx2, can->tx_data, &mbox);
}

/**
 * @brief Receives a CAN message from the 1st CAN bus. We only need 1 receive function for FCC.
* @param CAN_Driver_t *can
* @return HAL_StatusTypeDef
* @note Auto-gen: fill details.
*/
HAL_StatusTypeDef CAN_Receive1(CAN_Driver_t *can) {

    can->rx1.IDE = CAN_ID_STD;
    can->rx1.RTR = CAN_RTR_DATA;
    can->rx1.DLC = can->len;
    can->rx1.StdId = can->id;

    while (HAL_CAN_GetRxFifoFillLevel(can->hcan1, CAN_RX_FIFO0) == 0) { /* spin or add timeout */ }
    return HAL_CAN_GetRxMessage(can->hcan1, CAN_RX_FIFO0, &can->rx1, can->rx_data);
}


/**
 * @brief TODO: Re-implement CAN driver testing using CMock API?
* @param void
* @return void
* @note Auto-gen: fill details.
*/
void CAN_Test(void)
{
	// add some shit here
	return;
}







