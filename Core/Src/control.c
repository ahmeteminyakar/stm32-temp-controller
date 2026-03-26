#include "control.h"
#include "main.h"

void Control_Init(ControlState *state)
{
    state->mode        = MODE_AUTO;
    state->setpoint    = 25.0f;
    state->temperature = 0.0f;
    state->humidity    = 0.0f;
    state->heater_duty = 0.0f;
    state->fan_duty    = 0.0f;
}

void Control_SetMode(ControlState *state, ControlMode mode)
{
    state->mode = mode;
}

void Control_CycleMode(ControlState *state)
{
    state->mode = (ControlMode)((state->mode + 1) % 3);
}

const char *Control_ModeString(ControlMode mode)
{
    switch (mode) {
    case MODE_HEAT: return "HEAT";
    case MODE_COOL: return "COOL";
    case MODE_AUTO: return "AUTO";
    default:        return "????";
    }
}

void Control_Update(ControlState *state, PID_Controller *pid, float temp)
{
    state->temperature = temp;
    pid->setpoint = state->setpoint;

    float output = PID_Compute(pid, temp);

    switch (state->mode) {
    case MODE_HEAT:
        state->heater_duty = (output > 0.0f) ? output : 0.0f;
        state->fan_duty    = 0.0f;
        break;

    case MODE_COOL:
        state->heater_duty = 0.0f;
        state->fan_duty    = (output < 0.0f) ? -output : 0.0f;
        break;

    case MODE_AUTO:
        if (output > 0.0f) {
            state->heater_duty = output;
            state->fan_duty    = 0.0f;
        } else {
            state->heater_duty = 0.0f;
            state->fan_duty    = -output;
        }
        break;
    }
}

void Control_ApplyOutputs(ControlState *state, TIM_HandleTypeDef *htim)
{
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(htim);

    uint32_t heater_ccr = (uint32_t)((state->heater_duty / 100.0f) * (float)arr);
    uint32_t fan_ccr    = (uint32_t)((state->fan_duty    / 100.0f) * (float)arr);

    __HAL_TIM_SET_COMPARE(htim, HEATER_TIM_CHANNEL, heater_ccr);
    __HAL_TIM_SET_COMPARE(htim, FAN_TIM_CHANNEL,    fan_ccr);
}
