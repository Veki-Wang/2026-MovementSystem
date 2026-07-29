#include "Motor.h"

/* ============================================================
 * 初始化 — MX_TIM2_Init() 之后调用
 * ============================================================ */
void Stepper_Init(StepperMotor *motor, TIM_HandleTypeDef *htim, uint32_t channel)
{
    motor->htim        = htim;
    motor->tim_channel = channel;
    motor->position    = 0;
    motor->target      = 0;
    motor->steps_to_go = 0;
    motor->ccr_step    = 0;
    motor->running     = 0;
    motor->dir         = 1;
    motor->edge_toggle = 0;

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
        MOTOR_DIR_CW();
        motor->dir = 1;
    } else {
        MOTOR_DIR_CCW();
        motor->dir = 0;
        rpm = -rpm;
    }

    motor->ccr_step = Stepper_RPM_to_CCR(rpm);

    motor->htim->Instance->CNT = 0;
    __HAL_TIM_SET_COMPARE(motor->htim, motor->tim_channel, motor->ccr_step);

    if (!motor->running) {
        motor->running = 1;
        motor->steps_to_go = 0;
        MOTOR_ENA_ON();
        HAL_TIM_OC_Start_IT(motor->htim, motor->tim_channel);
    }
}

/* ============================================================
 * 相对移动 — 走 pulses 个脉冲后自动停
 * ============================================================ */
void Stepper_MoveRel(StepperMotor *motor, int32_t pulses, float rpm)
{
    if (pulses == 0 || rpm <= 0.0f) return;

    if (pulses > 0) {
        MOTOR_DIR_CW();
        motor->dir = 1;
        motor->steps_to_go = pulses;
    } else {
        MOTOR_DIR_CCW();
        motor->dir = 0;
        motor->steps_to_go = -pulses;
    }

    motor->target = motor->position + pulses;
    motor->ccr_step = Stepper_RPM_to_CCR(rpm);

    motor->htim->Instance->CNT = 0;
    __HAL_TIM_SET_COMPARE(motor->htim, motor->tim_channel, motor->ccr_step);

    if (!motor->running) {
        motor->running = 1;
        MOTOR_ENA_ON();
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
    MOTOR_ENA_OFF();
}

/* ============================================================
 * 查询是否到位
 * ============================================================ */
uint8_t Stepper_IsDone(StepperMotor *motor)
{
    return (motor->steps_to_go == 0) ? 1 : 0;
}

/* ============================================================
 * 定时器中断回调 — HAL_TIM_OC_DelayElapsedCallback() 中调用
 * ============================================================ */
void Stepper_IRQHandler(StepperMotor *motor)
{
    if (motor->htim->Channel != HAL_TIM_ACTIVE_CHANNEL_1) {
        return;
    }

    motor->edge_toggle ^= 1;
    motor->edge_toggle &= 1;

    if (motor->edge_toggle == 1) {
        /* 上升沿: 一个脉冲完成 */
        if (motor->dir == 1) {
            motor->position++;
        } else {
            motor->position--;
        }

        if (motor->steps_to_go > 0) {
            motor->steps_to_go--;
            if (motor->steps_to_go == 0) {
                Stepper_Stop(motor);
                return;
            }
        }
    }

    /* 设置下一个比较值 */
    uint32_t current_ccr = motor->htim->Instance->CCR1;
    uint32_t next_ccr    = current_ccr + motor->ccr_step;

    uint32_t cnt_now = motor->htim->Instance->CNT;
    if (next_ccr <= cnt_now + 2) {
        next_ccr = cnt_now + MOTOR_MIN_CCR;
    }

    motor->htim->Instance->CCR1 = (uint16_t)next_ccr;
}
