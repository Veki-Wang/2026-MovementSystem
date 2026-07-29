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
 * 更新四路角度
 *   控件 ID: t0(1), t5(5), t6(6), t7(7)
 * ============================================================ */
void Display_UpdateAngles(int a0_int, int a0_dec,
                          int a1_int, int a1_dec,
                          int a2_int, int a2_dec,
                          int a3_int, int a3_dec)
{
    snprintf(uart_tx_buffer, sizeof(uart_tx_buffer),
             "SET_NUM(1,%d,5);\r\n"
             "SET_NUM(5,%d,5);\r\n"
             "SET_NUM(6,%d,5);\r\n"
             "SET_NUM(7,%d,5);\r\n",
             a0_int, a1_int, a2_int, a3_int);
    UART4_Send(uart_tx_buffer);
}

/* ============================================================
 * 更新四路坐标
 *   X 控件 ID: 12, 13, 14, 15
 *   Y 控件 ID: 16, 17, 18, 19
 * ============================================================ */
void Display_UpdateCoords(int x0, int y0, int x1, int y1,
                          int x2, int y2, int x3, int y3)
{
    snprintf(uart_tx_buffer, sizeof(uart_tx_buffer),
             "SET_NUM(12,%d,5);\r\n"
             "SET_NUM(13,%d,5);\r\n"
             "SET_NUM(14,%d,5);\r\n"
             "SET_NUM(15,%d,5);\r\n"
             "SET_NUM(16,%d,5);\r\n"
             "SET_NUM(17,%d,5);\r\n"
             "SET_NUM(18,%d,5);\r\n"
             "SET_NUM(19,%d,5);\r\n",
             x0, x1, x2, x3,
             y0, y1, y2, y3);
    UART4_Send(uart_tx_buffer);
}
