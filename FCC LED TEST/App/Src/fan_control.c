#include "fan_control.h"
#include "temperature_sensor.h"

static int  s_last_temperature_c = 0;
static bool s_fan_enabled        = false;

void FanControl_Reset(void)
{
    s_last_temperature_c = 0;
    s_fan_enabled        = false;
}

bool FanControl_ShouldEnableCooling(void)
{
    s_last_temperature_c = TemperatureSensor_Read();

    if (s_last_temperature_c >= FAN_CONTROL_TEMP_ON_C)
    {
        s_fan_enabled = true;
    }
    else if (s_last_temperature_c <= FAN_CONTROL_TEMP_OFF_C)
    {
        s_fan_enabled = false;
    }

    return s_fan_enabled;
}

int FanControl_GetLastTemperature(void)
{
    return s_last_temperature_c;
}
