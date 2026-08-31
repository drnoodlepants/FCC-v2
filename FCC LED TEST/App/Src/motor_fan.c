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

void MotorFan_Init(MotorFan_t *mf, CAN_HandleTypeDef *hcan)
{
    mf->hcan         = hcan;
    mf->last_temp_c  = 0.0f;
    mf->last_rx_tick = HAL_GetTick();
    mf->link_stale   = true;     // fail-safe until first good message
    mf->state        = FAN_STATE_IDLE;
}

HAL_StatusTypeDef MotorFan_Start(MotorFan_t *mf, TIM_HandleTypeDef *htim)
{
    CAN_FilterTypeDef filter = {0};

    // Catch-all for now (mask 0x0000) - open item to tighten this to
    // FCC_CAN_ID_MOTOR_TEMP (0x446) once other nodes are active on the bus.
    filter.FilterIdHigh         = 0x0000;
    filter.FilterIdLow          = 0x0000;
    filter.FilterMaskIdHigh     = 0x0000;
    filter.FilterMaskIdLow      = 0x0000;
    filter.FilterFIFOAssignment = CAN_RX_FIFO0;
    filter.FilterBank           = 0;
    filter.FilterMode           = CAN_FILTERMODE_IDMASK;
    filter.FilterScale          = CAN_FILTERSCALE_32BIT;
    filter.FilterActivation     = CAN_FILTER_ENABLE;
    filter.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(mf->hcan, &filter) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_CAN_Start(mf->hcan) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // Only TIM_CHANNEL_3 is configured in MX_TIM1_Init() - starting other
    // channels here would return HAL_ERROR and silently skip this one.
    if (HAL_TIM_PWM_Start(htim, TIM_CHANNEL_3) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

void MotorFan_Poll(MotorFan_t *mf)
{
    // Non-blocking: only touch the FIFO if something's actually waiting.
    if (HAL_CAN_GetRxFifoFillLevel(mf->hcan, CAN_RX_FIFO0) > 0)
    {
        CAN_RxHeaderTypeDef rx_header;
        uint8_t rx_data[8];

        if (HAL_CAN_GetRxMessage(mf->hcan, CAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK
            && rx_header.StdId == FCC_CAN_ID_MOTOR_TEMP)
        {
            uint16_t deserialized[4] = {0};
CAN_16Bit_Deserializer(deserialized, rx_data);
mf->last_temp_c = (float)deserialized[0];
            mf->last_rx_tick = HAL_GetTick();
            mf->link_stale   = false;
        }
    }

    // Fail-safe: SysTick-based staleness check, same non-blocking pattern
    // as your heartbeat. If the DAQ goes quiet, we notice within one poll.
    if ((HAL_GetTick() - mf->last_rx_tick) > MOTOR_TEMP_CAN_TIMEOUT_MS)
    {
        mf->link_stale = true;
    }
}

float MotorFan_GetDuty(MotorFan_t *mf)
{
    if (mf->link_stale)
    {
        // Lost the DAQ - don't guess. Run cooling flat out until comms return.
        return 1.0f;
    }

    float t = mf->last_temp_c;

    // Hysteresis at the state edges only; linear ramp in between.
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
        case FAN_STATE_RAMPING:
        default:
            return Linear_Map(t, MOTOR_FAN_TEMP_FLOOR_C, MOTOR_FAN_TEMP_CEILING_C,
                               MOTOR_FAN_MIN_DUTY, 1.0f);
    }
}
