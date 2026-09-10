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
#include "stm32f4xx_hal.h"

extern CAN_HandleTypeDef hcan;

/**
 * @brief Check error status of can receive for debugging purposes.
 */
typedef enum {
    CAN_RX_OK = 0,
    CAN_RX_EMPTY = 1,       // no frame waiting - not an error, just nothing to do
    CAN_RX_HAL_ERROR = 2,
    CAN_RX_WRONG_ID = 3     // frame received but not the one we filtered for
} CAN_RxStatus_t;


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
HAL_StatusTypeDef CAN_ConfigMotorTempFilter(CAN_HandleTypeDef *hcan);
HAL_StatusTypeDef CAN_ConfigCatchAllFilter(CAN_HandleTypeDef *hcan);

HAL_StatusTypeDef CAN_Transmit1	(CAN_Driver_t *can);
HAL_StatusTypeDef CAN_Transmit2	(CAN_Driver_t *can);

CAN_RxStatus_t    CAN_Receive1(CAN_Driver_t *can);

//void CAN_16Bit_Serializer(float data_in, uint8_t output_buf[2]);
void CAN_N_Byte_Serializer(uint8_t n, float data_in, uint8_t *output_buf);
void CAN_16Bit_Deserializer(uint16_t data_in_buf[4], uint8_t rx_data[8]);

void CAN_Test(void);

#ifdef __cplusplus
}
#endif
#endif /* INC_CAN_DRIVER_H_ */
