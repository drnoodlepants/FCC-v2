// pot_fan.c
// Reads the pot (PC0, ADC_CHANNEL_10, DMA index 6) and linearly maps it to
// the fan PWM output (TIM1 CH3, PA10).

#include <stdio.h>
#include <pot_fan.h>

// ── Helpers ───────────────────────────────────────────────────────────────────

static inline float Constrain(float x, float lo, float hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

// ── Public API ────────────────────────────────────────────────────────────────

HAL_StatusTypeDef PotFan_Init(PotFan_t *pf,
                               ADC_HandleTypeDef *adc,
                               TIM_HandleTypeDef *pwm_tim)
{
    if (!pf || !adc || !pwm_tim) return HAL_ERROR;

    pf->adc      = adc;
    pf->pwm_tim  = pwm_tim;
    pf->pot_fraction = 0.0f;
    pf->pwm_compare  = 0u;

    for (uint8_t i = 0; i < ADC_CH_COUNT; i++)
        pf->adc_raw[i] = 0u;

    return HAL_OK;
}

HAL_StatusTypeDef PotFan_Start(PotFan_t *pf)
{
    if (!pf || !pf->adc || !pf->pwm_tim) return HAL_ERROR;

    // Start ADC with DMA – fills adc_raw[] continuously in the background.
    if (HAL_ADC_Start_DMA(pf->adc,
                           (uint32_t *)pf->adc_raw,
                           ADC_CH_COUNT) != HAL_OK)
        return HAL_ERROR;

    // Start PWM on TIM1 CH3 (PA10).
    if (HAL_TIM_PWM_Start(pf->pwm_tim, TIM_CHANNEL_3) != HAL_OK)
        return HAL_ERROR;

    return HAL_OK;
}

/*void PotFan_Update(PotFan_t *pf)
{
    if (!pf || !pf->pwm_tim) return;

    uint16_t raw = pf->adc_raw[ADC_IDX_POT];
    const float adc_max = (float)((1u << ADC_RES_BITS) - 1u);
    pf->pot_fraction = Constrain((float)raw / adc_max, 0.0f, 0.8f);
    pf->pwm_compare = (uint32_t)(pf->pot_fraction * (float)PWM_RESOLUTION);

    __HAL_TIM_SET_COMPARE(pf->pwm_tim, TIM_CHANNEL_3, pf->pwm_compare);
}*/

void PotFan_Update(PotFan_t *pf)
{
    if (!pf || !pf->pwm_tim) return;

    // 1. Read raw ADC value and clamp it to valid ADC hardware range [0, 1]
    uint16_t raw = pf->adc_raw[ADC_IDX_POT];
    const float adc_max = (float)((1u << ADC_RES_BITS) - 1u);
    float normalized_adc = Constrain((float)raw / adc_max, 0.0f, 1.0f);

    // 2. Linear map: 0.0–1.0 knob rotation maps to 0.0–0.8 (0–80% duty cycle)
    pf->pot_fraction = normalized_adc * 0.8f;

    // 3. Scale directly to timer ARR compare register
    pf->pwm_compare = (uint32_t)(pf->pot_fraction * (float)PWM_RESOLUTION);

    __HAL_TIM_SET_COMPARE(pf->pwm_tim, TIM_CHANNEL_3, pf->pwm_compare);
}
