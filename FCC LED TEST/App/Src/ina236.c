/*
 * ina236.c
 *
 *  Created on: 23 Aug 2026
 *      Author: Noodle
 */


#include "ina236.h"

static HAL_StatusTypeDef INA236_WriteReg(INA236_t *dev, uint8_t reg, uint16_t value)
{
    uint8_t buf[2] = { (uint8_t)(value >> 8), (uint8_t)(value & 0xFFu) };
    return HAL_I2C_Mem_Write(dev->hi2c, dev->addr, reg, I2C_MEMADD_SIZE_8BIT,
                              buf, 2, INA236_I2C_TIMEOUT_MS);
}

static HAL_StatusTypeDef INA236_ReadReg(INA236_t *dev, uint8_t reg, int16_t *value)
{
    uint8_t buf[2];
    HAL_StatusTypeDef st = HAL_I2C_Mem_Read(dev->hi2c, dev->addr, reg, I2C_MEMADD_SIZE_8BIT,
                                             buf, 2, INA236_I2C_TIMEOUT_MS);
    if (st == HAL_OK)
        *value = (int16_t)(((uint16_t)buf[0] << 8) | buf[1]);
    return st;
}

HAL_StatusTypeDef INA236_Init(INA236_t *dev, I2C_HandleTypeDef *hi2c,
                               float shunt_ohms, float max_expected_current_A)
{
    if (!dev || !hi2c || shunt_ohms <= 0.0f || max_expected_current_A <= 0.0f)
        return HAL_ERROR;

    dev->hi2c      = hi2c;
    dev->addr      = INA236_I2C_ADDR;
    dev->comms_ok  = false;

    // Current_LSB = max_expected_current / 2^15  (TI-recommended starting point)
    dev->current_lsb_A = max_expected_current_A / 32768.0f;
    // Power_LSB = 32 * Current_LSB, fixed on the INA236 (confirmed against TI's
    // published INA236 library - this differs from the INA226's 25x)
    dev->power_lsb_W   = 32.0f * dev->current_lsb_A;

    // SHUNT_CAL = 0.00512 / (Current_LSB * Rshunt)
    uint16_t cal = (uint16_t)((0.00512f / (dev->current_lsb_A * shunt_ohms)) + 0.5f);
    if (cal == 0) return HAL_ERROR;   // shunt/current combo out of range - don't arm a dead cal

    // We leave ADCRANGE at its power-on default (0 = +/-81.92 mV full scale,
    // 2.5 uV/bit shunt LSB), continuous shunt+bus conversion - matches the
    // shunt_voltage readback math below. If you ever change ADCRANGE, the
    // 2.5uV/bit constant in INA236_Update() needs to change too.

    if (INA236_WriteReg(dev, INA236_REG_CALIBRATION, cal) != HAL_OK)
        return HAL_ERROR;

    dev->comms_ok = true;
    return HAL_OK;
}

HAL_StatusTypeDef INA236_Update(INA236_t *dev)
{
    if (!dev || !dev->hi2c) return HAL_ERROR;

    int16_t raw_bus, raw_current, raw_power;

    if (INA236_ReadReg(dev, INA236_REG_BUS_V, &raw_bus) != HAL_OK)
    {
        dev->comms_ok = false;
        return HAL_ERROR;
    }
    if (INA236_ReadReg(dev, INA236_REG_CURRENT, &raw_current) != HAL_OK)
    {
        dev->comms_ok = false;
        return HAL_ERROR;
    }
    if (INA236_ReadReg(dev, INA236_REG_POWER, &raw_power) != HAL_OK)
    {
        dev->comms_ok = false;
        return HAL_ERROR;
    }

    dev->bus_voltage_V = (float)raw_bus * 0.0016f;             // 1.6 mV/bit, fixed
    dev->current_A     = (float)raw_current * dev->current_lsb_A;
    dev->power_W       = (float)raw_power   * dev->power_lsb_W;
    dev->comms_ok       = true;

    return HAL_OK;
}
