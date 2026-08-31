/*
 * motor_fan.h
 *
 *  Created on: 23 Aug 2026
 *      Author: Noodle
 */


#pragma once

#include "FCC_config.h"
#include "can_driver.h"

typedef enum {
    FAN_STATE_IDLE,     // below floor - min duty
    FAN_STATE_RAMPING,  // between floor and ceiling - linear
    FAN_STATE_MAX        // above ceiling - full duty
} FanThermalState_t;

typedef struct {
    CAN_HandleTypeDef *hcan;
    float    last_temp_c;
    uint32_t last_rx_tick;
    bool     link_stale;       // true if no valid msg within timeout -> fail-safe
    FanThermalState_t state;
} MotorFan_t;

void  MotorFan_Init(MotorFan_t *mf, CAN_HandleTypeDef *hcan);

// Consolidated startup: configures the CAN RX filter, starts the CAN
// peripheral, and starts PWM output on the given timer/channel. Call once
// after MotorFan_Init(), before entering the main loop.
HAL_StatusTypeDef MotorFan_Start(MotorFan_t *mf, TIM_HandleTypeDef *htim);

// Call every loop iteration. Non-blocking: checks FIFO once, consumes at most
// one frame, and updates the stale/fail-safe flag against HAL_GetTick().
void  MotorFan_Poll(MotorFan_t *mf);

// Returns duty 0.0-1.0. If the CAN link is stale, fails safe to full duty
// rather than guessing - better to over-cool than silently stop cooling.
float MotorFan_GetDuty(MotorFan_t *mf);
