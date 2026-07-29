#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "main.h"

/* ============================================================
 * 串口屏显示模块 (UART4, 115200bps)
 *
 *  角度控件 ID: 1, 5, 6, 7
 *  坐标控件 ID: 12~19 (X: 12-15, Y: 16-19)
 * ============================================================ */

void Display_Init(void);
void Display_UpdateAngles(int a0_int, int a0_dec,
                          int a1_int, int a1_dec,
                          int a2_int, int a2_dec,
                          int a3_int, int a3_dec);
void Display_UpdateCoords(int x0, int y0, int x1, int y1,
                          int x2, int y2, int x3, int y3);

#endif
