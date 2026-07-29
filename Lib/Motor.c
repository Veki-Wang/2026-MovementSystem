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
 * 查询是否已回零
 * ============================================================ */
uint8_t Stepper_IsHomed(StepperMotor *motor)
{
    return motor->homed;
}

/* ============================================================
 * 回零 — 低速往限位开关方向移动，撞到后归零
 *
 * 工作原理:
 *   1. 往限位开关方向慢速移动 (假设开关在 CW 方向)
 *   2. ISR 中每步检查限位开关
 *   3. 撞到开关 → 立即停止 → position = 0 → homed = 1
 *
 * 接线: PB2 → 限位开关(常开) → GND
 *       内部上拉, 撞到→LOW
 * ============================================================ */
void Stepper_Home(StepperMotor *motor, float speed_rpm)
{
    if (speed_rpm <= 0.0f) speed_rpm = 30.0f;  // 默认慢速回零

    motor->homing = 1;
    motor->homed  = 0;

    /* 往限位开关方向走: CW (dir=1) */
    MOTOR_DIR_CW();
    motor->dir  = 1;
    motor->steps_to_go = 0;  // 不限步数, 撞到才停
    motor->ccr_step = Stepper_RPM_to_CCR(speed_rpm);

    motor->htim->Instance->CNT = 0;
    __HAL_TIM_SET_COMPARE(motor->htim, motor->tim_channel, motor->ccr_step);

    if (!motor->running) {
        motor->running = 1;
        MOTOR_ENA_ON();
        HAL_TIM_OC_Start_IT(motor->htim, motor->tim_channel);
    }
}

/* ============================================================
 * 设置软限位 — 回零后限制电机活动范围
 *
 *   min: 最小脉冲值 (通常 ≤0, 零点往反方向可走)
 *   max: 最大脉冲值 (行程上限, 例如 XY 轴行程 = N 脉冲)
 *
 *   示例: Stepper_SetSoftLimit(&motor, -100, 10000);
 *         允许电机从零点反走 100 脉冲, 正走 10000 脉冲
 * ============================================================ */
void Stepper_SetSoftLimit(StepperMotor *motor, int32_t min, int32_t max)
{
    motor->soft_min = min;
    motor->soft_max = max;
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

        /* ★ 回零检测 ★ */
        if (motor->homing && MOTOR_LIMIT_HIT()) {
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

        /* ★ 软限位保护 ★ */
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

    /* 设置下一个比较值 */
    uint32_t current_ccr = motor->htim->Instance->CCR1;
    uint32_t next_ccr    = current_ccr + motor->ccr_step;

    uint32_t cnt_now = motor->htim->Instance->CNT;
    if (next_ccr <= cnt_now + 2) {
        next_ccr = cnt_now + MOTOR_MIN_CCR;
    }

    motor->htim->Instance->CCR1 = (uint16_t)next_ccr;
}
