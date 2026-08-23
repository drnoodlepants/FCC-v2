/*
 * ina236.h
 *
 *  Created on: 23 Aug 2026
 *      Author: Noodle
 */

#ifndef INC_INA236_H_
#define INC_INA236_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "FCC_config.h"

// Register map
#define INA236_REG_CONFIG       0x00u
#define INA236_REG_SHUNT_V      0x01u
#define INA236_REG_BUS_V        0x02u
#define INA236_REG_POWER        0x03u
#define INA236_REG_CURRENT      0x04u
#define INA236_REG_CALIBRATION  0x05u
#define INA236_REG_MASK_ENABLE  0x06u
#define INA236_REG_ALERT_LIMIT  0x07u
#define INA236_REG_MANUF_ID     0xFEu
#define INA236_REG_DIE_ID       0xFFu

typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint16_t addr;              // 8-bit shifted address

    float current_lsb_A;        // derived from calibration
    float power_lsb_W;          // = 32 * current_lsb_A on the INA236

    float bus_voltage_V;
    float shunt_voltage_V;
    float current_A;
    float power_W;

    bool  comms_ok;             // false if last transaction timed out/NAK'd
} INA236_t;

HAL_StatusTypeDef INA236_Init(INA236_t *dev, I2C_HandleTypeDef *hi2c,
                               float shunt_ohms, float max_expected_current_A);

// Reads bus voltage, current, power. Bounded I2C timeout - never spins forever.
// Sets dev->comms_ok = false on failure; stale cached values are left in place.
HAL_StatusTypeDef INA236_Update(INA236_t *dev);

#ifdef __cplusplus
}
#endif
#endif /* INC_INA236_H_ */
