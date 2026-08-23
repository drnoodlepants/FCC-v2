/*
* sensor_inputs.h
*
* Created on: Nov 1, 2025
* Author: Franc
*
*/



#ifndef INC_SENSOR_INPUTS_H_
#define INC_SENSOR_INPUTS_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "can_driver.h"
#include "FCC_config.h"

typedef struct {
	ADC_HandleTypeDef *adc;
	TIM_HandleTypeDef *tim;
	GPIO_TypeDef *gpio;

	// adc dma buffers
	volatile uint16_t adc_raw[ADC_CH_COUNT];
	volatile float adc_V[ADC_CH_COUNT];

	// data buffers for temperature and pressure values
	float temp_c[FCC_TEMP_SENSOR_COUNT];
	float pressure_kpa[FCC_PRESSURE_SENSOR_COUNT];
	float seg_temp_c[FCC_NUM_SEG_TEMPS];

	// pwm
	float ch1_duty_cycle;
	float ch2_duty_cycle;
	float ch3_duty_cycle;

	float fan_rpm;

	// power consumption
	float switch_current;
	float switch_voltage;
	float switch_power;

} SensorInputs_t;

HAL_StatusTypeDef Sensors_Init(SensorInputs_t *si, ADC_HandleTypeDef *adc, TIM_HandleTypeDef *tim);
HAL_StatusTypeDef Sensors_Start(SensorInputs_t *si);

void Store_Temperature_Readings(SensorInputs_t *si);
void Store_Pressure_Readings(SensorInputs_t *si);
void PWM_SetAll(SensorInputs_t *si);
void Update_Fan_Speed(SensorInputs_t *si);
void Update_ADC_Buffers(SensorInputs_t *si);
void Store_PowerConsumption_Data(SensorInputs_t *si);


#endif /* INC_SENSOR_INPUTS_H_ */
