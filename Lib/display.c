#include "display.h"
#include "uart.h"
#include <stdio.h>

/* ============================================================
 * 初始化 — 切换到主页
 * ============================================================ */
void Display_Init(void)
{
    UART4_Send("page 0\r\n");
}

/* ============================================================
 * 更新题号显示
 *   控件 ID: 20
 * ============================================================ */
void Display_UpdateQuestionNum(int16_t qnum)
{
    snprintf(uart_tx_buffer, sizeof(uart_tx_buffer),
             "SET_NUM(20,%d,0);\r\n", qnum);
    UART4_Send(uart_tx_buffer);
}

/* ============================================================
 * 更新发送标志
 *   控件 ID: 23 — 已发送=1, 未发送=0
 * ============================================================ */
void Display_UpdateSentFlag(int16_t val)
{
    snprintf(uart_tx_buffer, sizeof(uart_tx_buffer),
             "SET_NUM(23,%d,0);\r\n", val);
    UART4_Send(uart_tx_buffer);
}

/* ============================================================
 * 更新收包计数 (调试用)
 *   控件 ID: 21
 * ============================================================ */
void Display_UpdateRxCount(uint16_t count)
{
    snprintf(uart_tx_buffer, sizeof(uart_tx_buffer),
             "SET_NUM(21,%d,0);\r\n", count);
    UART4_Send(uart_tx_buffer);
}

/* ============================================================
 * 更新四路角度 (百位数, 显示 2 位小数)
 *   控件 ID: 1, 5, 6, 7
 *   例: a0=4567 → SET_NUM(1,4567,2) → 显示 45.67
 * ============================================================ */
void Display_UpdateAngles(int16_t a0, int16_t a1, int16_t a2, int16_t a3)
{
    snprintf(uart_tx_buffer, sizeof(uart_tx_buffer),
             "SET_NUM(1,%d,2);\r\n"
             "SET_NUM(5,%d,2);\r\n"
             "SET_NUM(6,%d,2);\r\n"
             "SET_NUM(7,%d,2);\r\n",
             a0, a1, a2, a3);
    UART4_Send(uart_tx_buffer);
}

/* ============================================================
 * 更新四路坐标 (百位数, 显示 2 位小数)
 *   X 控件 ID: 12, 13, 14, 15
 *   Y 控件 ID: 16, 17, 18, 19
 * ============================================================ */
void Display_UpdateCoords(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                          int16_t x2, int16_t y2, int16_t x3, int16_t y3)
{
    snprintf(uart_tx_buffer, sizeof(uart_tx_buffer),
             "SET_NUM(12,%d,2);\r\n"
             "SET_NUM(13,%d,2);\r\n"
             "SET_NUM(14,%d,2);\r\n"
             "SET_NUM(15,%d,2);\r\n"
             "SET_NUM(16,%d,2);\r\n"
             "SET_NUM(17,%d,2);\r\n"
             "SET_NUM(18,%d,2);\r\n"
             "SET_NUM(19,%d,2);\r\n",
             x0, x1, x2, x3,
             y0, y1, y2, y3);
    UART4_Send(uart_tx_buffer);
}
