/**
 * Dual CAN driver for the FCC
 * - Franco H
 */


#ifndef INC_CAN_DRIVER_H_
#define INC_CAN_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "FCC_config.h"


/**
 * @brief Check error status of can receive for debugging purposes.
 */
typedef enum {
	OK=0,                  	/**< OK */
	FULL_RXFIFO=1,         	/**< FULL_RXFIFO */
	_HAL_ERROR=2,           	/**< HAL_ERROR */
	INVALID_SERIALIZATION=3	/**< INVALID_SERIALIZATION */
} Receive_Status_t;


/**
 * @brief Generic CAN driver struct wrapper.
 */
typedef struct CAN_Driver_t {
    CAN_HandleTypeDef *hcan1;
    CAN_HandleTypeDef *hcan2;
    CAN_TxHeaderTypeDef tx1, tx2;
    CAN_RxHeaderTypeDef rx1;
    uint8_t  tx_data[8], rx_data[8];
    uint32_t id;
    uint8_t  len;
} CAN_Driver_t;


HAL_StatusTypeDef CAN_InitDriver(CAN_Driver_t *can);
HAL_StatusTypeDef CAN_Transmit1	(CAN_Driver_t *can);
HAL_StatusTypeDef CAN_Transmit2	(CAN_Driver_t *can);
HAL_StatusTypeDef CAN_Receive1	(CAN_Driver_t *can);

//void CAN_16Bit_Serializer(float data_in, uint8_t output_buf[2]);
void CAN_N_Byte_Serializer(uint8_t n, float data_in, uint8_t *output_buf);
void CAN_16Bit_Deserializer(uint16_t data_in_buf[4], uint8_t rx_data[8]);

void CAN_Test(void);

#ifdef __cplusplus
}
#endif
#endif /* INC_CAN_DRIVER_H_ */
