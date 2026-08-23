#ifndef FAN_CONTROL_H
#define FAN_CONTROL_H

#include <stdbool.h>

#define FAN_CONTROL_TEMP_ON_C          55
#define FAN_CONTROL_TEMP_HYSTERESIS_C   5
#define FAN_CONTROL_TEMP_OFF_C (FAN_CONTROL_TEMP_ON_C - FAN_CONTROL_TEMP_HYSTERESIS_C)

void FanControl_Reset(void);
bool FanControl_ShouldEnableCooling(void);
int  FanControl_GetLastTemperature(void);

#endif // FAN_CONTROL_H
