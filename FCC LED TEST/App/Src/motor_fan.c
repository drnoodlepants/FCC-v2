/*
 * motor_fan.c
 *
 *  Created on: 23 Aug 2026
 *      Author: Noodle
 */


#include "motor_fan.h"

static inline float Constrain(float x, float lo, float hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

static inline float Linear_Map(float x, float in_min, float in_max,
                                float out_min, float out_max) {
    if (in_max - in_min == 0.0f) return out_min;
    float scaled = (x - in_min) / (in_max - in_min);
    return out_min + Constrain(scaled, 0.0f, 1.0f) * (out_max - out_min);
}

void MotorFan_Init(MotorFan_t *mf)
{
    mf->last_temp_c  = 0.0f;
    mf->last_rx_tick = HAL_GetTick();
    mf->link_stale   = true;   // fail-safe until first good sample
    mf->state        = FAN_STATE_IDLE;
}

void MotorFan_UpdateTemp(MotorFan_t *mf, float temp_c)
{
    mf->last_temp_c  = temp_c;
    mf->last_rx_tick = HAL_GetTick();
    mf->link_stale   = false;
}

float MotorFan_GetDuty(MotorFan_t *mf)
{
    if (mf->link_stale) return 1.0f;

    float t = mf->last_temp_c;

    switch (mf->state)
    {
        case FAN_STATE_IDLE:
            if (t >= MOTOR_FAN_TEMP_FLOOR_C) mf->state = FAN_STATE_RAMPING;
            break;
        case FAN_STATE_RAMPING:
            if (t >= MOTOR_FAN_TEMP_CEILING_C)
                mf->state = FAN_STATE_MAX;
            else if (t < MOTOR_FAN_TEMP_FLOOR_C - MOTOR_FAN_TEMP_HYST_C)
                mf->state = FAN_STATE_IDLE;
            break;
        case FAN_STATE_MAX:
            if (t < MOTOR_FAN_TEMP_CEILING_C - MOTOR_FAN_TEMP_HYST_C)
                mf->state = FAN_STATE_RAMPING;
            break;
    }

    switch (mf->state)
    {
        case FAN_STATE_IDLE: return MOTOR_FAN_MIN_DUTY;
        case FAN_STATE_MAX:  return 1.0f;
        default:
            return Linear_Map(t, MOTOR_FAN_TEMP_FLOOR_C, MOTOR_FAN_TEMP_CEILING_C,
                               MOTOR_FAN_MIN_DUTY, 1.0f);
    }
}
