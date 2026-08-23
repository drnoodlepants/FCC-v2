// pot_fan.h
// Reads the pot ADC channel and linearly maps it to the fan PWM duty cycle.
// Used for hardware validation before the thermal control loop is enabled.

#pragma once

#include "FCC_config.h"

// ── Driver state ──────────────────────────────────────────────────────────────

typedef struct {
    ADC_HandleTypeDef *adc;
    TIM_HandleTypeDef *pwm_tim;     // TIM1

    volatile uint16_t adc_raw[ADC_CH_COUNT];  // DMA fills this
    float   pot_fraction;           // 0.0 – 1.0 after read
    uint32_t pwm_compare;           // last value written to CCR
} PotFan_t;

// ── API ───────────────────────────────────────────────────────────────────────

HAL_StatusTypeDef PotFan_Init (PotFan_t *pf,
                                ADC_HandleTypeDef *adc,
                                TIM_HandleTypeDef *pwm_tim);

HAL_StatusTypeDef PotFan_Start(PotFan_t *pf);

// Call from the main loop: reads pot, writes PWM.
void PotFan_Update(PotFan_t *pf);
