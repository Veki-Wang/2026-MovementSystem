#ifndef __MOTOR_H
#define __MOTOR_H

#include "main.h"

/* ============================================================
 * 步进电机驱动 (DM542 + 1.8° + 1600 脉冲/转)
 *
 * 定时器参数:
 *   PSC = 169  → 计数器时钟 = 1 MHz
 *   ARR = 65535
 *   OC Toggle: f_out = 1MHz / (2 × CCR)
 *
 * 速度:
 *   CCR = 18750 / rpm
 * ============================================================ */

#define MOTOR_TIM_PSC           169U
#define MOTOR_TIM_ARR           65535U
#define MOTOR_TIM_TICK_HZ       1000000U
#define MOTOR_PULSES_PER_REV    1600U

#define MOTOR_MAX_CCR           65535U
#define MOTOR_MIN_CCR           3U

typedef struct {
    TIM_HandleTypeDef   *htim;
    uint32_t             tim_channel;

    /* 引脚 — 每个电机独立 */
    GPIO_TypeDef        *dir_port;
    uint16_t             dir_pin;
    GPIO_TypeDef        *ena_port;
    uint16_t             ena_pin;
    GPIO_TypeDef        *limit_port;
    uint16_t             limit_pin;

    volatile int32_t     position;
    int32_t              target;
    int32_t              steps_to_go;

    uint32_t             ccr_step;
    uint8_t              edge_toggle;
    uint8_t              running;
    uint8_t              dir;
    uint8_t              homing;
    uint8_t              homed;
    int32_t              soft_min;
    int32_t              soft_max;
} StepperMotor;

void Stepper_Init(StepperMotor *motor, TIM_HandleTypeDef *htim, uint32_t channel,
                  GPIO_TypeDef *dir_port, uint16_t dir_pin,
                  GPIO_TypeDef *ena_port, uint16_t ena_pin,
                  GPIO_TypeDef *limit_port, uint16_t limit_pin);
void Stepper_SetSpeed(StepperMotor *motor, float rpm);
void Stepper_MoveRel(StepperMotor *motor, int32_t pulses, float rpm);
void Stepper_MoveAbs(StepperMotor *motor, int32_t target_pos, float rpm);
void Stepper_Stop(StepperMotor *motor);
uint8_t Stepper_IsDone(StepperMotor *motor);
uint8_t Stepper_IsHomed(StepperMotor *motor);
void Stepper_Home(StepperMotor *motor, float speed_rpm);
void Stepper_SetSoftLimit(StepperMotor *motor, int32_t min, int32_t max);
void Stepper_IRQHandler(StepperMotor *motor);

static inline uint32_t Stepper_RPM_to_CCR(float rpm)
{
    if (rpm <= 0.0f) return 0;
    uint32_t ccr = (uint32_t)(18750.0f / rpm);
    if (ccr < MOTOR_MIN_CCR) ccr = MOTOR_MIN_CCR;
    if (ccr > MOTOR_MAX_CCR) ccr = MOTOR_MAX_CCR;
    return ccr;
}

static inline float Stepper_CCR_to_RPM(uint32_t ccr)
{
    if (ccr == 0) return 0.0f;
    return 18750.0f / (float)ccr;
}

#endif
