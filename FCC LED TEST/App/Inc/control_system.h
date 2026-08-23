/*
 * control_system.h
 *
 *  Created on: Nov 3, 2025
 *      Author: Franc
 */

#ifndef INC_CONTROL_SYSTEM_H_
#define INC_CONTROL_SYSTEM_H_

#include "can_driver.h"
#include "sensor_inputs.h"
#include "FCC_config.h"


/**
 * @brief Defines PWM modes of controlling each of the 3 fans.
 */
typedef enum {
	BASE_MODE,  /**< BASE_MODE */
	ABOVE_40DEG,/**< ABOVE_40DEG */
	ABOVE_50DEG, /**< ABOVE_50DEG */
	OVERCURRENT_FAULT
} Ctrl_State_t;


typedef enum {
	SW_OVERCURRENT,
	SW_UNDERCURRENT
} LED_t;

typedef struct {
	Ctrl_State_t ctrl;
	LED_t led;
} FCC_t;


void FCC_Control_Loop(FCC_t *FCC, SensorInputs_t *si, CAN_Driver_t *can);
void CAN_TransmitAll_SensorData(SensorInputs_t *si, CAN_Driver_t *can);
void Set_SwitchEnable(FCC_t *FCC, bool en);
void Toggle_Status_LEDs(FCC_t *FCC);
void CAN_Transmit_PowerConsumption(FCC_t *FCC, SensorInputs_t *si, CAN_Driver_t *can);
void Set_SwitchEnable(FCC_t *FCC, bool en);


#endif /* INC_CONTROL_SYSTEM_H_ */
