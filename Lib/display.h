#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "main.h"

/* ============================================================
 * 串口屏显示模块 (UART4, 115200bps)
 *
 *  角度控件 ID: 1, 5, 6, 7
 *  X 坐标 ID: 8, 9, 10, 11, 18, 20, 22, 24
 *  Y 坐标 ID: 12,13,14,15, 19, 21, 23, 25
 *  辅助控件: 题号=16, 发送标志=17, 收包计数=27
 *
 *  ★ 参数为百位数 (实际值 × 100), 显示 2 位小数
 *    例: 1234 → 屏幕上显示 12.34
 * ============================================================ */

void Display_Init(void);
void Display_UpdateQuestionNum(int16_t qnum);
void Display_UpdateSentFlag(int16_t val);
void Display_UpdateRxCount(uint16_t count);
void Display_UpdateAngles(int16_t a0, int16_t a1, int16_t a2, int16_t a3);
void Display_UpdateCoords(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                          int16_t x2, int16_t y2, int16_t x3, int16_t y3,
                          int16_t x4, int16_t y4, int16_t x5, int16_t y5,
                          int16_t x6, int16_t y6, int16_t x7, int16_t y7);

#endif
