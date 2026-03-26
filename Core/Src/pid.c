#include "pid.h"

void PID_Init(PID_Controller *pid, float kp, float ki, float kd,
              float dt, float out_min, float out_max)
{
    pid->kp = kp;  pid->ki = ki;  pid->kd = kd;
    pid->dt = dt;
    pid->output_min = out_min;
    pid->output_max = out_max;
    pid->setpoint   = 0.0f;
    pid->integral   = 0.0f;
    pid->prev_error = 0.0f;
}

float PID_Compute(PID_Controller *pid, float measurement)
{
    float error = pid->setpoint - measurement;

    float p_term = pid->kp * error;

    pid->integral += error * pid->dt;
    float i_term = pid->ki * pid->integral;

    float derivative = (error - pid->prev_error) / pid->dt;
    float d_term = pid->kd * derivative;
    pid->prev_error = error;

    float output = p_term + i_term + d_term;

    if (output > pid->output_max) {
        output = pid->output_max;
        pid->integral -= error * pid->dt;
    } else if (output < pid->output_min) {
        output = pid->output_min;
        pid->integral -= error * pid->dt;
    }

    return output;
}

void PID_Reset(PID_Controller *pid)
{
    pid->integral   = 0.0f;
    pid->prev_error = 0.0f;
}

void PID_SetSetpoint(PID_Controller *pid, float sp)
{
    pid->setpoint = sp;
}
