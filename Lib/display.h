#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "main.h"

/* ============================================================
 * 串口屏显示模块 (UART4, 115200bps)
 *
 *  角度控件 ID: 1, 5, 6, 7
 *  坐标控件 ID: 12~19 (X: 12-15, Y: 16-19)
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
                          int16_t x2, int16_t y2, int16_t x3, int16_t y3);

#endif
