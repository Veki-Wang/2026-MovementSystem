#ifndef __MOTOR_H
#define __MOTOR_H

#include "main.h"

/* ============================================================
 * 步进电机驱动 (DM542 + 1.8° 步进电机 + 1600 脉冲/转)
 *
 * 硬件连接:
 *   STEP: TIM2_CH1 → PA0  → DM542 PUL+
 *   DIR:  GPIO PB0        → DM542 DIR+
 *   ENA:  GPIO PB1        → DM542 ENA+
 *   PUL-/DIR-/ENA- → GND (共阴接法)
 *
 * 定时器参数 (TIM2, APB1 Timer Clock = 170 MHz):
 *   PSC = 169  → 计数器时钟 = 1 MHz
 *   ARR = 65535
 *   OC Toggle 模式: f_out = 1MHz / (2 × CCR)
 *
 * 速度换算:
 *   f_pulse = rpm × 1600 / 60
 *   CCR = 1,000,000 / (2 × f_pulse) = 18,750 / rpm
 * ============================================================ */

#define MOTOR_TIM_PSC           169U
#define MOTOR_TIM_ARR           65535U
#define MOTOR_TIM_TICK_HZ       1000000U
#define MOTOR_PULSES_PER_REV    1600U

#define MOTOR_MAX_CCR           65535U
#define MOTOR_MIN_CCR           3U

#define MOTOR_DIR_PORT          GPIOB
#define MOTOR_DIR_PIN           GPIO_PIN_0
#define MOTOR_ENA_PORT          GPIOB
#define MOTOR_ENA_PIN           GPIO_PIN_1

#define MOTOR_DIR_CW()          HAL_GPIO_WritePin(MOTOR_DIR_PORT, MOTOR_DIR_PIN, GPIO_PIN_SET)
#define MOTOR_DIR_CCW()         HAL_GPIO_WritePin(MOTOR_DIR_PORT, MOTOR_DIR_PIN, GPIO_PIN_RESET)
#define MOTOR_ENA_ON()          /* 悬空, 暂不用 */
#define MOTOR_ENA_OFF()         /* 悬空, 暂不用 */

/* 限位开关 (常开, 撞到→LOW) */
#define MOTOR_LIMIT_PORT        GPIOB
#define MOTOR_LIMIT_PIN         GPIO_PIN_2
#define MOTOR_LIMIT_HIT()       (HAL_GPIO_ReadPin(MOTOR_LIMIT_PORT, MOTOR_LIMIT_PIN) == GPIO_PIN_RESET)

typedef struct {
    TIM_HandleTypeDef   *htim;
    uint32_t             tim_channel;

    volatile int32_t     position;
    int32_t              target;
    int32_t              steps_to_go;

    uint32_t             ccr_step;
    uint8_t              edge_toggle;
    uint8_t              running;
    uint8_t              dir;
    uint8_t              homing;        // 正在回零中
    uint8_t              homed;         // 已完成回零
    int32_t              soft_min;      // 软限位最小值 (脉冲数)
    int32_t              soft_max;      // 软限位最大值 (脉冲数)
} StepperMotor;

void Stepper_Init(StepperMotor *motor, TIM_HandleTypeDef *htim, uint32_t channel);
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
