#ifndef CONTROL_H
#define CONTROL_H

#include "main.h"
#include "pid.h"

typedef struct {
    ControlMode mode;
    float       setpoint;
    float       temperature;
    float       humidity;
    float       heater_duty;
    float       fan_duty;
} ControlState;

void Control_Init(ControlState *state);
void Control_SetMode(ControlState *state, ControlMode mode);
void Control_CycleMode(ControlState *state);
void Control_Update(ControlState *state, PID_Controller *pid, float temp);
void Control_ApplyOutputs(ControlState *state, TIM_HandleTypeDef *htim);
const char *Control_ModeString(ControlMode mode);

#endif
