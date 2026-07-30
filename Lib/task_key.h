#ifndef _TASK_KEY_H_
#define _TASK_KEY_H_

#include "main.h"

/* 按键引脚由 CubeMX 在 main.h 中定义: KEY1=PB12, KEY2=PB13, KEY3=PB15 */

/* --- 扫描/任务状态 --- */
typedef struct {
    int  question_num;   // 当前题号 1~3 (KEY1 轮询)
    int  send_question;  // 发送题号标志 (KEY2 触发, 主循环消费后清零)
    int  scan;           // 扫描模式 0/1/2 (KEY3 轮询)
} ScanInit_t;

extern ScanInit_t scan;
extern int last_key_display;    /* 最后按键值, 给屏显示用 */

unsigned char Key_GetCode(void);
unsigned char Key_Get(void);
void Key_LoopDetect(void);
void key_task(void);
void Key_Scan(void);   /* 完整扫描: 消抖 + 处理, ~200ms */

#endif
