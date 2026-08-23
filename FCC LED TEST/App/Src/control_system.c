/**
 * Main FCC Control Code
 * - Franco Heraud
 */


#include "control_system.h"

// Intended for some future error debugging...
bool has_can_failed = false;
volatile bool over_current_warning = false;
/**
 * @brief Returns largest temperature out of all input temperatures going into MCU.
 * @param SensorInputs_t *si
 * @return float
 * @note Auto-gen: fill details.
 */
static float Highest_Input_Temp(SensorInputs_t *si) {
	float largest_temp = -1000.0f;
	for (uint8_t i = 0; i < 4; i++) {
		if (largest_temp < si->temp_c[i])
			largest_temp = si->temp_c[i];
	}
	return largest_temp;
}

/**
 * @brief Returns largest temperature out of all segment temperatures recorded from
 * the CAN bus.
 * @param SensorInputs_t *si
 * @return float
 * @note Auto-gen: fill details.
 */
static float Highest_Seg_Temp(SensorInputs_t *si) {
	float largest_temp = -1000.0f;
	for (uint8_t i = 0; i < FCC_NUM_SEG_TEMPS; i++) {
		if (largest_temp < si->seg_temp_c[i])
			largest_temp = si->seg_temp_c[i];
	}
	return largest_temp;
}

/**
 * @brief Sends all the data in the temp + pressure buffers over to the relevant CAN bus.
 * @param SensorInputs_t *si, CAN_Driver_t *can,
 * @return void
 * @note Auto-gen: fill details.
 */
void CAN_TransmitAll_SensorData(SensorInputs_t *si, CAN_Driver_t *can) {

	// Serializing pressure inputs + Transmit them:
	const uint8_t serialization_bytes = 1;
	memset((void*)can->tx_data, 0, sizeof(can->tx_data));
	for (uint8_t i = 0; i < FCC_PRESSURE_SENSOR_COUNT; i++) {
		CAN_N_Byte_Serializer(serialization_bytes, si->pressure_kpa[i], &can->tx_data[i*serialization_bytes]);
	}
	can->tx2.StdId 	= FCC_CAN_ID_PRESSURE;
	can->tx2.DLC 	= 4;
	CAN_Transmit2(can);

	// Serializing temperature inputs + Transmit them:
	memset((void*)can->tx_data, 0, sizeof(can->tx_data));
	for (uint8_t i = 0; i < FCC_TEMP_SENSOR_COUNT; i++) {
		CAN_N_Byte_Serializer(serialization_bytes, si->temp_c[i], &can->tx_data[i*serialization_bytes]);
	}
	can->tx2.StdId 	= FCC_CAN_ID_TEMP;
	can->tx2.DLC 	= 4;
	CAN_Transmit2(can);

	// Assume seg temp values are in ideal format:
	memset((void*)can->tx_data, 0, sizeof(can->tx_data));
	for (uint8_t i = 0; i < FCC_NUM_SEG_TEMPS; i++) {
		CAN_N_Byte_Serializer(serialization_bytes, si->seg_temp_c[i], &can->tx_data[i*serialization_bytes]);
	}
	can->tx2.StdId 	= FCC_CAN_ID_TEMP;
	can->tx2.DLC 	= 4;
	CAN_Transmit2(can);

	return;
}


/**
 * @brief Logs and processes power consumption data via CAN.
 * @param FCC
 * @param si
 * @param can
 */
void CAN_Transmit_PowerConsumption(FCC_t *FCC, SensorInputs_t *si, CAN_Driver_t *can) {

	memset((void*)can->tx_data, 0, sizeof(can->tx_data));
	uint8_t switch_power_u = (uint8_t)lroundf(si->switch_power);
	uint8_t switch_voltage_u = (uint8_t)lroundf(si->switch_voltage);
	uint8_t switch_current_u = (uint8_t)lroundf(si->switch_current);

	can->tx_data[0] = switch_power_u;
	can->tx_data[1] = switch_voltage_u;
	can->tx_data[2] = switch_current_u;
	can->tx2.StdId = FCC_CAN_ID_POWER;
	can->tx2.DLC = 3;
	CAN_Transmit2(can);

	const float max_allowed_current = 60.0f;
	if (si->switch_current > max_allowed_current) {
		FCC->led = SW_OVERCURRENT;
		over_current_warning = true;
	} else {
		FCC->led = SW_UNDERCURRENT;
		over_current_warning = false;
	}

	return;
}


/**
 * Capable of setting/resetting the switch circuit pin and updating the corresponding led state in the FCC struct...
 * @param FCC
 * @param en -> Set low when there is
 */
void Set_SwitchEnable(FCC_t *FCC, bool en) {
	if (en) {
		FCC->led = SW_UNDERCURRENT;
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	}
	else {
		FCC->led = SW_OVERCURRENT;

		// IMMEDIATE SHUTDOWN
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	}
	return;
}


/**
 * @brief	Toggles the status leds depending on the led_t flags...
 * @param 	FCC
 * @return 	void
 */
void Toggle_Status_LEDs(FCC_t *FCC) {
	switch (FCC->led) {
	case SW_OVERCURRENT:
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_SET);
		break;
	case SW_UNDERCURRENT:
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET);
		break;
	}
	return;
}



__weak void FCC_OverCurrent_Handler(void) {
	//...
}

/**
 * @brief Main control loop. Uses a simple threshold algorithm with hysteresis.
 * @param FCC_t *FCC, SensorInputs_t *si, CAN_Driver_t *can
 * @return void
 * @note Intended to be polled over and over again.
 */
void FCC_Control_Loop(FCC_t *FCC, SensorInputs_t *si, CAN_Driver_t *can) {

	static Ctrl_State_t control_state = BASE_MODE;

	// handle over current warning flag
	control_state = (over_current_warning) ? OVERCURRENT_FAULT : control_state;

	Store_Temperature_Readings(si);
	Store_Pressure_Readings(si);

	float temp_seg_max 	= Highest_Seg_Temp(si);
	float temp_in_max	= Highest_Input_Temp(si);
	float temp_max;
	if (temp_seg_max > temp_in_max) temp_max = temp_seg_max;
	else temp_max = temp_in_max;


	switch (control_state) {
	case (BASE_MODE):
		si->ch1_duty_cycle = 0.3f;
		si->ch2_duty_cycle = 0.3f;
		si->ch3_duty_cycle = 0.3f;
		if (temp_max > FCC_TEMP_THRESHOLD1_DEGC)
			control_state = ABOVE_40DEG;
		break;

	case (ABOVE_40DEG):
		si->ch1_duty_cycle = 0.7f;
		si->ch2_duty_cycle = 0.7f;
		si->ch3_duty_cycle = 0.7f;
		if (temp_max > FCC_TEMP_THRESHOLD2_DEGC)
			control_state = ABOVE_50DEG;

		else if (temp_max < FCC_TEMP_THRESHOLD1_DEGC - FCC_TEMP_HYSTERESIS_DEGC)
			control_state = BASE_MODE;

		break;

	case (ABOVE_50DEG):
		si->ch1_duty_cycle = 1.0f;
		si->ch2_duty_cycle = 1.0f;
		si->ch3_duty_cycle = 1.0f;

		if (temp_max < FCC_TEMP_THRESHOLD2_DEGC - FCC_TEMP_HYSTERESIS_DEGC)
			control_state = ABOVE_40DEG;

		break;

	case (OVERCURRENT_FAULT) :
		FCC_OverCurrent_Handler();
		if (!over_current_warning)
			control_state = BASE_MODE;
		break;
	}

	PWM_SetAll(si);
	return ;
}


