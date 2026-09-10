/*
 * motor_fan.h
 *
 *  Created on: 23 Aug 2026
 *      Author: Noodle
 */


#pragma once

#include "FCC_config.h"

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

void  MotorFan_Init(MotorFan_t *mf);

// Call whenever a fresh temperature sample arrives (from CAN, bench test, whatever).
void  MotorFan_UpdateTemp(MotorFan_t *mf, float temp_c);

// Call every loop iteration regardless of whether a new CAN frame arrived -
// re-arms link_stale if too much time has passed since the last update.
void  MotorFan_CheckStale(MotorFan_t *mf);

// Returns duty 0.0-1.0. Fails safe to full duty if link_stale.
float MotorFan_GetDuty(MotorFan_t *mf);
