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
 * 更新收包计数 (调试用)
 *   控件 ID: 27
 * ============================================================ */
void Display_UpdateRxCount(uint16_t count)
{
    snprintf(uart_tx_buffer, sizeof(uart_tx_buffer),
             "SET_NUM(27,%d,0);\r\n", count);
    UART4_Send(uart_tx_buffer);
}

/* ============================================================
 * 更新题号
 *   控件 ID: 16
 * ============================================================ */
void Display_UpdateQuestionNum(int16_t qnum)
{
    snprintf(uart_tx_buffer, sizeof(uart_tx_buffer),
             "SET_NUM(16,%d,0);\r\n", (int)qnum);
    UART4_Send(uart_tx_buffer);
}

/* ============================================================
 * 更新发送标志
 *   控件 ID: 17
 * ============================================================ */
void Display_UpdateSentFlag(int16_t val)
{
    snprintf(uart_tx_buffer, sizeof(uart_tx_buffer),
             "SET_NUM(17,%d,0);\r\n", (int)val);
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
 * 更新八路坐标 (百位数, 显示 2 位小数)
 *   X 控件 ID: 8, 9, 10, 11, 18, 20, 22, 24
 *   Y 控件 ID: 12,13,14,15, 19, 21, 23, 25
 * ============================================================ */
void Display_UpdateCoords(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                          int16_t x2, int16_t y2, int16_t x3, int16_t y3,
                          int16_t x4, int16_t y4, int16_t x5, int16_t y5,
                          int16_t x6, int16_t y6, int16_t x7, int16_t y7)
{
    snprintf(uart_tx_buffer, sizeof(uart_tx_buffer),
             "SET_NUM(8,%d,2);\r\n"
             "SET_NUM(12,%d,2);\r\n"
             "SET_NUM(9,%d,2);\r\n"
             "SET_NUM(13,%d,2);\r\n"
             "SET_NUM(10,%d,2);\r\n"
             "SET_NUM(14,%d,2);\r\n"
             "SET_NUM(11,%d,2);\r\n"
             "SET_NUM(15,%d,2);\r\n"
             "SET_NUM(16,%d,2);\r\n"
             "SET_NUM(17,%d,2);\r\n"
             "SET_NUM(18,%d,2);\r\n"
             "SET_NUM(19,%d,2);\r\n"
             "SET_NUM(20,%d,2);\r\n"
             "SET_NUM(21,%d,2);\r\n"
             "SET_NUM(22,%d,2);\r\n"
             "SET_NUM(23,%d,2);\r\n",
             x0, y0, x1, y1, x2, y2, x3, y3,
             x4, y4, x5, y5, x6, y6, x7, y7);
    UART4_Send(uart_tx_buffer);
}
