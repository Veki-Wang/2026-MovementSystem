#ifndef __UART_TV_H
#define __UART_TV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "main.h"

extern volatile uint8_t uart_tx_busy; // 发送忙标志
extern char uart_tx_buffer[512];        // 发送缓冲区

void UART4_SendByte(char data);
int UART_GET(void); 
void CheckBusy(void);
void UART4_Send_Str(char *str);
void UART4_Send(char *databuf);
void UART4_Send_DMA(const char *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif
