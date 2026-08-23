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

#define PWM_RESOLUTION        65535u

// CAN IDs
#define FCC_CAN_ID_BASE           0x440u
#define FCC_CAN_ID_TEMP           0x441u
#define FCC_CAN_ID_PRESSURE       0x442u
#define FCC_CAN_ID_SEG_TEMP       0x443u
#define FCC_CAN_ID_TACH           0x444u
#define FCC_CAN_ID_POWER          0x445u

// Motor temp CAN message (DAQ), same 4x16-bit format as FCC_CAN_ID_SEG_TEMP
#define FCC_CAN_ID_MOTOR_TEMP      0x446u
#define MOTOR_TEMP_CAN_TIMEOUT_MS  200u    // fail-safe trip if no msg for this long

// Fan curve
#define MOTOR_FAN_TEMP_FLOOR_C     30.0f   // below this: idle duty
#define MOTOR_FAN_TEMP_CEILING_C   70.0f   // above this: 100%
#define MOTOR_FAN_TEMP_HYST_C       3.0f   // hysteresis band at each edge
#define MOTOR_FAN_MIN_DUTY           0.20f // idle duty (0.0 if you want fans fully off when cold)

// INA236 power monitor (switch circuit current/voltage/power)
#define INA236_I2C_ADDR            (0x40 << 1)   // A1=GND, A0=GND -> 0x40, HAL wants 8-bit
#define INA236_SHUNT_OHMS          0.003f
#define INA236_MAX_EXPECTED_A      100.0f        // <-- confirm/adjust this
#define INA236_I2C_TIMEOUT_MS      10u           // bounded, never block main loop indefinitely

#define FCC_NUM_SEG_TEMPS         4u

// End
#define ADC_IDX_POT        6u   // rank 7 → index 6

