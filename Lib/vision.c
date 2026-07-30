#include "vision.h"

/* ============================================================
 * 全局变量
 * ============================================================ */
VisionData_t vision_data = {0};

/* ============================================================
 * 静态变量 — 状态机 + 调试计数
 * ============================================================ */
static uint8_t  rx_buf[VISION_RX_NUM];   /* 帧缓冲 26 bytes */
static uint8_t  vf_idx = 0;              /* 当前写入位置 */
static uint32_t vf_last_tick = 0;        /* 上一字节时间戳 */
static uint16_t rx_pkt_count = 0;        /* 成功收包计数 */

/* UART3 中断接收的单字节缓冲 */
static volatile uint8_t vision_rx_byte;

/* ============================================================
 * Vision_Init — 启动 UART3 中断接收
 * ============================================================ */
void Vision_Init(void)
{
    vision_data.data_ready = 0;
    vf_idx = 0;
    vf_last_tick = HAL_GetTick();

    /* 启动单字节中断接收 */
    HAL_UART_Receive_IT(&huart3, (uint8_t *)&vision_rx_byte, 1);
}

/* ============================================================
 * Vision_ReArmRX — 重新使能下一字节接收
 * ============================================================ */
void Vision_ReArmRX(void)
{
    HAL_UART_Receive_IT(&huart3, (uint8_t *)&vision_rx_byte, 1);
}

/* ============================================================
 * Vision_SendQuestion — 发送题号给视觉
 *   帧格式: A5 + int16(LE) + 5A, 共 4 字节
 * ============================================================ */
void Vision_SendQuestion(int16_t question_num)
{
    uint8_t tx[VISION_TX_NUM];
    tx[0] = VISION_TX_HEAD;                         /* A5 */
    tx[1] = (uint8_t)(question_num & 0xFF);         /* 低字节 */
    tx[2] = (uint8_t)((question_num >> 8) & 0xFF);  /* 高字节 */
    tx[3] = VISION_TX_TAIL;                         /* 5A */

    HAL_UART_Transmit(&huart3, tx, VISION_TX_NUM, 10);
}

/* ============================================================
 * 辅助宏 — 从 buffer 中取出 int16 (little-endian)
 * ============================================================ */
static inline int16_t get_int16(const uint8_t *p)
{
    return (int16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

/* ============================================================
 * Vision_FeedByte — 逐字节喂入状态机
 *   (在 HAL_UART_RxCpltCallback 中调用)
 * ============================================================ */
void Vision_FeedByte(uint8_t ch)
{
    /* --- 超时保护: 上一字节距离现在太久 → 半截包作废 --- */
    uint32_t now = HAL_GetTick();
    if (vf_idx > 0 && (now - vf_last_tick) > VISION_TIMEOUT) {
        vf_idx = 0;
    }
    vf_last_tick = now;

    /* ========== 状态 1: 寻找帧头 B6 ========== */
    if (vf_idx == 0) {
        if (ch == VISION_RX_HEAD) {
            rx_buf[0] = ch;
            vf_idx = 1;
        }
        return;
    }

    /* ========== 状态 2: 填充包体 ========== */
    rx_buf[vf_idx++] = ch;

    if (vf_idx < VISION_RX_NUM) {
        return;    /* 未收满 26 字节, 继续等待 */
    }

    /* ========== 状态 3: 收满, 校验帧尾 6B ========== */
    if (rx_buf[VISION_RX_NUM - 1] == VISION_RX_TAIL) {
        /*
         * 数据布局 (从 rx_buf[1] 开始, 每个 int16 = 2 bytes, LE):
         *   [ 1.. 2] 角度1, [ 3.. 4] 角度2, [ 5.. 6] 角度3, [ 7.. 8] 角度4
         *   [ 9..10] X1,    [11..12] X2,    [13..14] X3,    [15..16] X4
         *   [17..18] Y1,    [19..20] Y2,    [21..22] Y3,    [23..24] Y4
         */
        for (uint8_t i = 0; i < VISION_TARGETS; i++) {
            vision_data.angles[i] = get_int16(&rx_buf[1  + i * 2]);
            vision_data.xs[i]     = get_int16(&rx_buf[9  + i * 2]);
            vision_data.ys[i]     = get_int16(&rx_buf[17 + i * 2]);
        }

        vision_data.data_ready = 1;
        rx_pkt_count++;         /* 调试: 成功收包 +1 */
        vf_idx = 0;    /* 准备接收下一帧 */
        return;
    }

    /* ========== 帧尾错误 → 同步丢失, 在 buffer 内重找 B6 ========== */
    uint8_t k;
    for (k = 1; k < VISION_RX_NUM; k++) {
        if (rx_buf[k] == VISION_RX_HEAD) break;
    }

    if (k < VISION_RX_NUM) {
        /* 在 buffer 中间找到了新 B6, 搬到开头 */
        uint8_t n = VISION_RX_NUM - k;
        for (uint8_t i = 0; i < n; i++) {
            rx_buf[i] = rx_buf[k + i];
        }
        vf_idx = n;
    } else {
        /* 整个 buffer 都没有 B6, 彻底重来 */
        vf_idx = 0;
    }
}

/* ============================================================
 * Vision_DataReady — 数据就绪?
 * ============================================================ */
uint8_t Vision_DataReady(void)
{
    return vision_data.data_ready;
}

/* ============================================================
 * Vision_GetData — 取出数据并清除 ready 标志
 * ============================================================ */
void Vision_GetData(VisionData_t *out)
{
    if (out) {
        for (uint8_t i = 0; i < VISION_TARGETS; i++) {
            out->angles[i] = vision_data.angles[i];
            out->xs[i]     = vision_data.xs[i];
            out->ys[i]     = vision_data.ys[i];
        }
    }
    vision_data.data_ready = 0;
}

/* ============================================================
 * Vision_Reset — 复位状态机 (题目切换等)
 * ============================================================ */
void Vision_Reset(void)
{
    vf_idx = 0;
    vision_data.data_ready = 0;
    rx_pkt_count = 0;    /* 新题目, 计数归零 */
}

/* ============================================================
 * Vision_GetRxCount — 返回成功收包数 (调试用)
 * ============================================================ */
uint16_t Vision_GetRxCount(void)
{
    return rx_pkt_count;
}

/* ============================================================
 * HAL_UART_RxCpltCallback — UART 接收完成回调
 *   USART3: 喂入视觉解析器并重新使能接收
 * ============================================================ */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3) {
        Vision_FeedByte(vision_rx_byte);
        HAL_UART_Receive_IT(&huart3, (uint8_t *)&vision_rx_byte, 1);
    }
}
