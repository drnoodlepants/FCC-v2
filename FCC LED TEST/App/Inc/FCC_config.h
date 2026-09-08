// AUTO-GENERATED FILE. DO NOT EDIT.
// Generated from FCC_config.yaml

#pragma once

// Project: UWA Motorsports FCCumulator Cooling Control
// Version: 1.0.0
// Author:  Franco Heraud
// Contributors: 

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include "main.h"

#define ADC_CH_COUNT              10u
#define ADC_RES_BITS          12
#define FCC_ADC_VREF_VOLTS        3.3f
#define FCC_TIMER_TICK_HZ         1000000u
#define FCC_FAN_PULSES_PER_REV    1

// Status LEDs
#define STATUS_LED1                (uint16_t)0x0040
#define STATUS_LED2                (uint16_t)0x0080
#define STATUS_LED3                (uint16_t)0x0800
#define STATUS_LED4                (uint16_t)0x1000
#define STATUS_LED5                (uint16_t)0x2000

#define PWM_RESOLUTION        639u

// CAN IDs
#define FCC_CAN_ID_BASE           0x440u
#define FCC_CAN_ID_TEMP           0x441u
#define FCC_CAN_ID_PRESSURE       0x442u
#define FCC_CAN_ID_SEG_TEMP       0x443u
#define FCC_CAN_ID_TACH           0x444u
#define FCC_CAN_ID_POWER          0x445u

// Pressure sensors
#define FCC_PRESSURE_SENSOR_COUNT 4u

#define FCC_PRESS_PIN1_ADC_CH         1u
#define FCC_PRESS_PIN2_ADC_CH         2u
#define FCC_PRESS_PIN3_ADC_CH         3u
#define FCC_PRESS_PIN4_ADC_CH         4u
// Temperature sensors
#define FCC_TEMP_SENSOR_COUNT     4u

#define FCC_TEMP_TIN1_ADC_CH         5u
#define FCC_TEMP_TIN2_ADC_CH         6u
#define FCC_TEMP_TIN3_ADC_CH         7u
#define FCC_TEMP_TIN4_ADC_CH         8u

// Control thresholds
#define FCC_TEMP_THRESHOLD1_DEGC  40.0f
#define FCC_TEMP_THRESHOLD2_DEGC  60.0f
#define FCC_TEMP_HYSTERESIS_DEGC  3.0f

#define MOTOR_FAN_MIN_DUTY           0.20f // idle duty (0.0 if you want fans fully off when cold)

#define FCC_NUM_SEG_TEMPS         4u

// End
#define ADC_IDX_POT        6u   // rank 7 → index 6

