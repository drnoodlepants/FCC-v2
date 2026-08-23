/*
* sensor_inputs.c
* Created on: Nov 1, 2025
* Author: Franc
*/

#include "sensor_inputs.h"

extern TIM_HandleTypeDef htim2;

// voltage divider resistors
const float r_series = 30e3f, r_pulldown = 47.6e3f;

/**
 * @brief Helper constrain function.
* @param float x, float low, float high
* @return x -> Constrained between low and high.
* @note Auto-gen: fill details.
*/
static float Constrain(float x, float low, float high) {
	if (x < low) return low;
	if (x > high) return high;
	return x;
}

/**
 * @brief Helper function which linearly maps a float value from one range to another
* @param float value, float in_min, float in_max, float out_min, float out_max
* @return return_type
* @note Auto-gen: fill details.
*/
static inline float Linear_Map(float x,
                               float in_min,  float in_max,
                               float out_min, float out_max) {
    if (in_max - in_min == 0.0f)
        return out_min;

    float scaled = (x - in_min) / (in_max - in_min);
    return out_min + scaled * (out_max - out_min);
}

/**
 * @brief 	Converts output sensor voltage to kPa for BOSCH 0 261 230 280. Assumes linear relationship in pressure calibration as found in spec sheet.
* @param 	float volts
* @return 	float p_abs_kPa
* @note		Tested and verified.
*/
static float Voltage_To_kPa_Bosch(float volts) {
	float p_abs_kPa = Linear_Map(volts, 0.4f, 4.65f, 20.0f, 300.0f);
	return p_abs_kPa;
}


/**
 * @brief 	Converts output sensor voltage to kPa for MIP series pressure sensor.
 * @param 	float volts
 * @return 	float p_abs_kPa
 * @note 	Tested and verified.
 */
static float Voltage_To_kPa_MIP(float volts) {
    const float v_supply    = 5.0f;			// volts
    const float p_min  		= 0.0f;     	// psi
    const float p_max  		= 52.973f;   	// psi (new, calibrated value)

    float ratio 	= (volts / v_supply - 0.10f) / 0.80f;
    ratio 			= Constrain(ratio, 0.0f, 1.0f);
    float p_psi 	= p_min + ratio * (p_max - p_min);

    const float psi_to_kpa = 6.894757f;
    return psi_to_kpa * p_psi;
}


/**
 * @brief 	Temperature sensor driver for the A-1325. Uses the Stein-Hart equation to compute the temperature.
 * @param 	Output voltage from sensor
 * @return 	float temp_degC -> Temperature in degrees Celsius
 * @note 	Tested and verified. Altered so that the calcs match the actual circuit in the schematic.
 */
static float Voltage_To_DegC_A1325(float volts) {
	const float r_25 		= 2752.0f;					// ohms
	const float v_supply 	= 3.067f;					// V (formula is radiometric)

	float r_therm = (v_supply * r_series * r_pulldown)	// ohms -> thermal resistance
				/ ((v_supply - volts) * r_pulldown - r_series * volts);

	float ratio 			= r_therm / r_25;
	float ln_ratio 			= log(ratio);

	const float a = 0.003354016f, b = 0.000250275f, c = 2.42945e-6f, d = -7.3121e-8f;

	float temp_inv_K	= a + b * ln_ratio + c * ln_ratio * ln_ratio + d * ln_ratio * ln_ratio * ln_ratio;
	float temp_degC		= (1 / temp_inv_K) - 273.15f;

	return temp_degC;
}

/**
 * @brief 	Temperature sensor driver for the BOSCH 0 261 230 280.
 * Uses beta parameter equation to compute the temperature.
 * Beta found from: https://www.ametherm.com/thermistor/ntc-thermistor-beta
 * @param 	float volts -> Output voltage from sensor
 * @return 	float temp_degC -> Temperature in degrees Celsius
 * @note 	Tested and Verified. Altered so that the calcs match the actual circuit in the schematic.
 */
static float Voltage_To_DegC_Bosch(float volts) {
	const float r_25 = 2057.0f;				// ohms
	const float temp_ref 	= 298.15f;		// 25 deg C
	const float v_supply 	= 3.067f;		// V (formula is radiometric)
	const float beta 		= 3463.0f;		// (ratio)

	float r_therm = (v_supply * r_series * r_pulldown) 	// ohms -> thermal resistance
			/ ((v_supply - volts) * r_pulldown - r_series * volts);

	float temp_inv_K 	=  (1 / temp_ref) + (1 / beta) * log(r_therm / r_25);
	float temp_degC 	=  (1 / temp_inv_K) - 273.15f;

	return temp_degC;
}

// TODO: Double check the pull down resistor value in the voltage dividers currently
// present on the FCC PCB?

// FIXME: Once the FCC gets re-manufactured, theres a good chance the pull down resistors
// will actually be 57.6k -> remember to change!

/**
 * @brief Sensor initialization function. Acts as a constructor for FCC_t.
 * @param SensorInputs_t *si, ADC_HandleTypeDef *adc, TIM_HandleTypeDef *tim
 * @return HAL_StatusTypeDef
 * @note Auto-gen: fill details.
 */
HAL_StatusTypeDef Sensors_Init(SensorInputs_t *si, ADC_HandleTypeDef *adc, TIM_HandleTypeDef *tim) {

	if (!si || !adc || !tim) return HAL_ERROR;

	memset((void*)si->adc_raw, 0, sizeof(si->adc_raw));
	memset((void*)si->adc_V, 0, sizeof(si->adc_V));
	memset((void*)si->temp_c, 0, sizeof(si->temp_c));
	memset((void*)si->pressure_kpa, 0, sizeof(si->pressure_kpa));

	si->adc = adc;
	si->tim = tim;

	si->ch1_duty_cycle = 0.0f;
	si->ch2_duty_cycle = 0.0f;
	si->ch3_duty_cycle = 0.0f;

	return HAL_OK;
}

/**
 * @brief Checks ADC DMA status and the status of all 3 PWM channels.
* @param SensorInputs_t *si
* @return HAL_StatusTypeDef
* @note Auto-gen: fill details.
*/
HAL_StatusTypeDef Sensors_Start(SensorInputs_t *si) {
	if (!si || !si->adc || !si->tim) return HAL_ERROR;


	if (HAL_ADC_Start_DMA(si->adc, (uint32_t*)si->adc_raw, ADC_CH_COUNT) != HAL_OK)
		return HAL_ERROR;

	if (HAL_TIM_PWM_Start(si->tim, TIM_CHANNEL_1) != HAL_OK) return HAL_ERROR;
	if (HAL_TIM_PWM_Start(si->tim, TIM_CHANNEL_2) != HAL_OK) return HAL_ERROR;
	if (HAL_TIM_PWM_Start(si->tim, TIM_CHANNEL_3) != HAL_OK) return HAL_ERROR;

	//PWM_SetAll(si);

	return HAL_OK;
}

/**
 * @brief Performs a single ADC read and updates the adc voltage buffer (in V)
* @param SensorInputs_t *si
* @return void
* @note Auto-gen: fill details.
*/
void Update_ADC_Buffers(SensorInputs_t *si) {

	HAL_ADC_Start(si->adc);

	for (uint8_t i = 0; i < ADC_CH_COUNT; i++)
		si->adc_V[i] = (si->adc_raw[i] / ((float)pow(2,ADC_RES_BITS) - 1.0f)) * 3.30f;
}

/**
 * @brief Store converted temperature readings to temperature buffer.
* @param SensorInputs_t *si
* @return void
* @note Auto-gen: fill details.
*/
void Store_Temperature_Readings(SensorInputs_t *si) {
	for (uint8_t i = 0; i < 2; i++)
		si->temp_c[i] = Voltage_To_DegC_A1325(si->adc_V[i]);

	for (uint8_t i = 2; i < FCC_TEMP_SENSOR_COUNT; i++)
		si->temp_c[i] = Voltage_To_DegC_Bosch(si->adc_V[i]);
}

/**
 * @brief Store converted p readings to pressure buffer.
* @param SensorInputs_t *si
* @return void
* @note Auto-gen: fill details.
*/
void Store_Pressure_Readings(SensorInputs_t *si) {
	for (uint8_t i = 0; i < 2; i++)
		si->pressure_kpa[i] = Voltage_To_kPa_Bosch(si->adc_V[i + 4]);

	for (uint8_t i = 2; i < FCC_PRESSURE_SENSOR_COUNT; i++)
		si->pressure_kpa[i] = Voltage_To_kPa_MIP(si->adc_V[i + 4]);
}

/**
 * @brief Apply the struct member duty cycles to TIM1 CH1, CH2 & CH3.
* @param SensorInputs_t *si
* @return void
* @note Auto-gen: fill details.
*/
void PWM_SetAll(SensorInputs_t *si) {
	if (!si || !si->tim) return;

	float d1 = Constrain(si->ch1_duty_cycle, 0.0f, 100.0f);
	float d2 = Constrain(si->ch2_duty_cycle, 0.0f, 100.0f);
	float d3 = Constrain(si->ch3_duty_cycle, 0.0f, 100.0f);

	// default res is 255
	uint32_t d1_scaled = (uint32_t)(d1 * (float)PWM_RESOLUTION);
	uint32_t d2_scaled = (uint32_t)(d2 * (float)PWM_RESOLUTION);
	uint32_t d3_scaled = (uint32_t)(d3 * (float)PWM_RESOLUTION);

	__HAL_TIM_SET_COMPARE(si->tim, TIM_CHANNEL_1, d1_scaled);
	__HAL_TIM_SET_COMPARE(si->tim, TIM_CHANNEL_2, d2_scaled);
	__HAL_TIM_SET_COMPARE(si->tim, TIM_CHANNEL_3, d3_scaled);

}


/**
 * @brief Read segment temperature data coming from CAN bus.
* @param SensorInputs_t *si, CAN_Driver_t *can
* @return bool
* @note Auto-gen: fill details.
*/
Receive_Status_t Update_Segment_Temperature_Values(SensorInputs_t *si, CAN_Driver_t *can) {

	can->rx1.StdId 	= FCC_CAN_ID_SEG_TEMP;
	can->rx1.DLC	= 8;
	HAL_StatusTypeDef can_rx_status = CAN_Receive1(can);

	if (can_rx_status != HAL_OK)
		return _HAL_ERROR;

	if (HAL_CAN_GetRxFifoFillLevel(can->hcan1, CAN_RX_FIFO0) == 0)
	    return FULL_RXFIFO;

	uint16_t deserialized_buf[FCC_NUM_SEG_TEMPS] = {0};
	CAN_16Bit_Deserializer(deserialized_buf, can->rx_data);

	for (uint8_t i = 0; i < FCC_NUM_SEG_TEMPS; i++) {
		si->seg_temp_c[i] = (float)can->rx_data[i];
	}
	return OK;
}


// TODO: Verify PULSES_PER_REVOLUTION!
const uint8_t PULSES_PER_REVOLUTION = 1;
static volatile uint32_t tach_last_ticks  = 0;
static volatile uint32_t tach_delta_ticks = 0;
static volatile uint8_t  tach_new_period  = 0;


/**
 * @brief ISR callback function used for the fan speed logic.
* @param uint16_t GPIO_Pin
* @return void
* @note Auto-gen: fill details.
*/


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == THERM_IN_Pin) {
        uint32_t now = __HAL_TIM_GET_COUNTER(&htim2);

        uint32_t delta = now - tach_last_ticks;
        tach_last_ticks = now;

        if (delta > 0) {
            tach_delta_ticks = delta;
            tach_new_period  = 1;
        }
    }
}


/**
 * @brief Call to update the fan speed into si->fan_rpm.
 * @param SensorInputs_t *si
 * @return void
 * @note Auto-gen: fill details.
 */
void Update_Fan_Speed(SensorInputs_t *si) {
	if (!tach_new_period || tach_delta_ticks == 0) si->fan_rpm = 0.0f;
	const float tick_freq_hz = (float)pow(10, 6);
	float pulse_freq_hz = tick_freq_hz / (float)tach_delta_ticks;
	si->fan_rpm = (pulse_freq_hz * 60) / (float)PULSES_PER_REVOLUTION;
	return;
}


/**
 * @brief Measures current from the switch circuit. This translates the output from the
 * current sense amplifier.
 * @param SensorInputs_t *si
 * @note shunt res is 0.003 ohms, gain was 50
 * datasheet btw is: https://www.ti.com/lit/ds/symlink/ina293-q1.pdf?ts=1763285906829&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FINA293-Q1%252Fpart-details%252FINA293B2QDBVRQ1
 */
static void Measure_SwitchCircuit_Current(SensorInputs_t *si) {
	const float v_supply 	= 3.3f; 			// V
	const float gain 		= 50.0f;			// (ratio)
	const float r_shunt 	= 0.003;			// ohms
	const float max_allowed_current = 50.0f;	// placeholder for now (determine this!)

	const float v_div_ratio = r_pulldown / (r_pulldown + r_series);

	float v_sense_out 		= si->adc_V[8]; 	// V
	v_sense_out = Constrain(v_sense_out, 0.0f, v_supply);

	si->switch_current = v_sense_out / (v_div_ratio * gain * r_shunt);

	si->switch_current = (v_sense_out * (r_series + r_pulldown)) / (0.15f * r_pulldown); // A (from KCL)

	if (si->switch_current > max_allowed_current) {
		// handle over current...
	}

	return;
}

// FIXME: I am worried that the PCB is currently powering the current sense amplifier using 3.3V and then
// we use a dedicated 5V->3.3V voltage divider for the amp output? Not sure if we're okay with this? Especially
// if we want to use the full 0-3.3V range for the ADC (for accuracy)? May need to change v_supply!

/**
 * @brief Logs the switch circuit voltage after being stepped down by a voltage divider and going through a
 * voltage follower op amp.
 * @param si
 * @note Added warning flag for over voltage.
 */
static void Measure_SwitchCircuit_Voltage(SensorInputs_t *si) {
	float v_state = si->adc_V[9]; // V
	const float max_voltage = 2.262f; // V
	//const float saturation_voltage = 3.3f;
	const float v_div_ratio = (2.21f / 12.21f);
	si->switch_voltage = v_state / v_div_ratio;

	if (si->switch_voltage > max_voltage) {
		// handle warning flag...
	}
	return;
}


/**
 * @brief Computes P = VI...
 * @param si
 * @note Updated ch count to 10
 */
void Store_PowerConsumption_Data(SensorInputs_t *si) {
	Measure_SwitchCircuit_Voltage(si);
	Measure_SwitchCircuit_Current(si);
	si->switch_power = si->switch_current * si->switch_voltage;
	return;
}




