#ifndef __VISION_H
#define __VISION_H

#include "main.h"

/* USART3 句柄 — 定义在 main.c */
extern UART_HandleTypeDef huart3;

/* ============================================================
 * 视觉通讯模块 (USART3, 230400 bps)
 *
 *  TX (MCU → 视觉): A5 + 题号(int16 LE) + 5A  (4 bytes)
 *  RX (视觉 → MCU): B6 + 角度1~4(int16 LE) + X1~4(int16 LE)
 *                       + Y1~4(int16 LE) + 6B
 *                   每包 26 bytes, 一次包含全部 4 个目标
 *  ★ 所有 int16 均为百位数 (实际值 × 100)
 * ============================================================ */

/* --- 帧格式常量 --- */
#define VISION_TX_HEAD      0xA5
#define VISION_TX_TAIL      0x5A
#define VISION_TX_NUM       4       /* HEAD + int16(2) + TAIL */

#define VISION_RX_HEAD      0xB6
#define VISION_RX_TAIL      0x6B
#define VISION_RX_NUM       26      /* HEAD + 12×int16(24) + TAIL */

#define VISION_TIMEOUT      50      /* 超时 ms, 半截包作废 */
#define VISION_TARGETS      4       /* 目标数量 */

/* --- 视觉数据结构 --- */
typedef struct {
    int16_t  angles[VISION_TARGETS];   /* 4 个目标的角度 (百位数) */
    int16_t  xs[VISION_TARGETS];       /* 4 个目标的 X 坐标 (百位数) */
    int16_t  ys[VISION_TARGETS];       /* 4 个目标的 Y 坐标 (百位数) */
    uint8_t  data_ready;               /* 数据就绪 */
} VisionData_t;

extern VisionData_t vision_data;

/* --- API --- */
void Vision_Init(void);
void Vision_SendQuestion(int16_t question_num);
void Vision_FeedByte(uint8_t ch);
uint8_t Vision_DataReady(void);
void Vision_GetData(VisionData_t *out);
void Vision_Reset(void);
uint16_t Vision_GetRxCount(void);

#endif
