#include "Motor.h"

/* ============================================================
 * 初始化 — MX_TIMx_Init() 之后调用
 * ============================================================ */
void Stepper_Init(StepperMotor *motor, TIM_HandleTypeDef *htim, uint32_t channel,
                  GPIO_TypeDef *dir_port, uint16_t dir_pin,
                  GPIO_TypeDef *ena_port, uint16_t ena_pin,
                  GPIO_TypeDef *limit_port, uint16_t limit_pin)
{
    motor->htim        = htim;
    motor->tim_channel = channel;

    motor->dir_port    = dir_port;
    motor->dir_pin     = dir_pin;
    motor->ena_port    = ena_port;
    motor->ena_pin     = ena_pin;
    motor->limit_port  = limit_port;
    motor->limit_pin   = limit_pin;

    motor->position    = 0;
    motor->target      = 0;
    motor->steps_to_go = 0;
    motor->ccr_step    = 0;
    motor->running     = 0;
    motor->dir         = 1;
    motor->edge_toggle = 0;
    motor->homing      = 0;
    motor->homed       = 0;
    motor->soft_min    = 0;
    motor->soft_max    = 0;

    TIM_OC_InitTypeDef sConfigOC = {0};

    htim->Instance->CR1 = 0;
    htim->Init.Prescaler         = MOTOR_TIM_PSC;
    htim->Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim->Init.Period            = MOTOR_TIM_ARR;
    htim->Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_OC_Init(htim);

    sConfigOC.OCMode     = TIM_OCMODE_TOGGLE;
    sConfigOC.Pulse      = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_OC_ConfigChannel(htim, &sConfigOC, channel);
}

/* 辅助宏 — 操作当前电机的 GPIO */
#define _DIR_CW(m)     HAL_GPIO_WritePin((m)->dir_port, (m)->dir_pin, GPIO_PIN_SET)
#define _DIR_CCW(m)    HAL_GPIO_WritePin((m)->dir_port, (m)->dir_pin, GPIO_PIN_RESET)
#define _ENA_ON(m)     do { if ((m)->ena_port) HAL_GPIO_WritePin((m)->ena_port, (m)->ena_pin, GPIO_PIN_RESET); } while(0)
#define _ENA_OFF(m)    do { if ((m)->ena_port) HAL_GPIO_WritePin((m)->ena_port, (m)->ena_pin, GPIO_PIN_SET); } while(0)
#define _LIMIT_HIT(m)  (HAL_GPIO_ReadPin((m)->limit_port, (m)->limit_pin) == GPIO_PIN_RESET)

/* ============================================================
 * 设置速度 — 连续恒速
 * ============================================================ */
void Stepper_SetSpeed(StepperMotor *motor, float rpm)
{
    if (rpm == 0.0f) {
        Stepper_Stop(motor);
        return;
    }

    if (rpm > 0.0f) {
        _DIR_CW(motor);
        motor->dir = 1;
    } else {
        _DIR_CCW(motor);
        motor->dir = 0;
        rpm = -rpm;
    }

    motor->ccr_step = Stepper_RPM_to_CCR(rpm);

    motor->htim->Instance->CNT = 0;
    __HAL_TIM_SET_COMPARE(motor->htim, motor->tim_channel, motor->ccr_step);

    if (!motor->running) {
        motor->running = 1;
        motor->steps_to_go = 0;
        _ENA_ON(motor);
        HAL_TIM_OC_Start_IT(motor->htim, motor->tim_channel);
    }
}

/* ============================================================
 * 相对移动
 * ============================================================ */
void Stepper_MoveRel(StepperMotor *motor, int32_t pulses, float rpm)
{
    if (pulses == 0 || rpm <= 0.0f) return;

    if (pulses > 0) {
        _DIR_CW(motor);
        motor->dir = 1;
        motor->steps_to_go = pulses;
    } else {
        _DIR_CCW(motor);
        motor->dir = 0;
        motor->steps_to_go = -pulses;
    }

    motor->target = motor->position + pulses;
    motor->ccr_step = Stepper_RPM_to_CCR(rpm);

    motor->htim->Instance->CNT = 0;
    __HAL_TIM_SET_COMPARE(motor->htim, motor->tim_channel, motor->ccr_step);

    if (!motor->running) {
        motor->running = 1;
        _ENA_ON(motor);
        HAL_TIM_OC_Start_IT(motor->htim, motor->tim_channel);
    }
}

/* ============================================================
 * 绝对移动
 * ============================================================ */
void Stepper_MoveAbs(StepperMotor *motor, int32_t target_pos, float rpm)
{
    int32_t delta = target_pos - motor->position;
    Stepper_MoveRel(motor, delta, rpm);
}

/* ============================================================
 * 急停
 * ============================================================ */
void Stepper_Stop(StepperMotor *motor)
{
    HAL_TIM_OC_Stop_IT(motor->htim, motor->tim_channel);
    motor->running     = 0;
    motor->steps_to_go = 0;
    motor->ccr_step    = 0;
    _ENA_OFF(motor);
}

/* ============================================================
 * 查询是否到位
 * ============================================================ */
uint8_t Stepper_IsDone(StepperMotor *motor)
{
    return (motor->steps_to_go == 0) ? 1 : 0;
}

/* ============================================================
 * 查询是否已回零
 * ============================================================ */
uint8_t Stepper_IsHomed(StepperMotor *motor)
{
    return motor->homed;
}

/* ============================================================
 * 回零
 * ============================================================ */
void Stepper_Home(StepperMotor *motor, float speed_rpm)
{
    if (speed_rpm <= 0.0f) speed_rpm = 30.0f;

    motor->homing = 1;
    motor->homed  = 0;

    _DIR_CW(motor);
    motor->dir  = 1;
    motor->steps_to_go = 0;
    motor->ccr_step = Stepper_RPM_to_CCR(speed_rpm);

    motor->htim->Instance->CNT = 0;
    __HAL_TIM_SET_COMPARE(motor->htim, motor->tim_channel, motor->ccr_step);

    if (!motor->running) {
        motor->running = 1;
        _ENA_ON(motor);
        HAL_TIM_OC_Start_IT(motor->htim, motor->tim_channel);
    }
}

/* ============================================================
 * 设置软限位
 * ============================================================ */
void Stepper_SetSoftLimit(StepperMotor *motor, int32_t min, int32_t max)
{
    motor->soft_min = min;
    motor->soft_max = max;
}

/* ============================================================
 * 定时器中断回调
 * ============================================================ */
void Stepper_IRQHandler(StepperMotor *motor)
{
    if (motor->htim->Channel != HAL_TIM_ACTIVE_CHANNEL_1) {
        return;
    }

    motor->edge_toggle ^= 1;
    motor->edge_toggle &= 1;

    if (motor->edge_toggle == 1) {
        /* 回零检测 */
        if (motor->homing && _LIMIT_HIT(motor)) {
            Stepper_Stop(motor);
            motor->position = 0;
            motor->homed    = 1;
            motor->homing   = 0;
            return;
        }

        if (motor->dir == 1) {
            motor->position++;
        } else {
            motor->position--;
        }

        /* 软限位 */
        if (motor->homed) {
            if (motor->position >= motor->soft_max && motor->dir == 1) {
                Stepper_Stop(motor);
                return;
            }
            if (motor->position <= motor->soft_min && motor->dir == 0) {
                Stepper_Stop(motor);
                return;
            }
        }

        if (motor->steps_to_go > 0) {
            motor->steps_to_go--;
            if (motor->steps_to_go == 0) {
                Stepper_Stop(motor);
                return;
            }
        }
    }

    /* 下一个比较值 */
    uint32_t current_ccr = motor->htim->Instance->CCR1;
    uint32_t next_ccr    = current_ccr + motor->ccr_step;

    uint32_t cnt_now = motor->htim->Instance->CNT;
    if (next_ccr <= cnt_now + 2) {
        next_ccr = cnt_now + MOTOR_MIN_CCR;
    }

    motor->htim->Instance->CCR1 = (uint16_t)next_ccr;
}
