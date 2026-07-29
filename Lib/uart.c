#include "uart.h"
#include "main.h"
typedef unsigned char u8;
volatile uint8_t uart_tx_busy = 0; // 发送忙标志
char uart_tx_buffer[512] = {0};      // 发送缓冲区
//发送一个字节数据
void UART4_SendByte(char data)
{
    HAL_UART_Transmit(&huart4, (uint8_t *)&data, 1, 1000);//1000表示最大等待时间为1000ms
    while(__HAL_UART_GET_FLAG(&huart4,UART_FLAG_TC)==RESET && __HAL_UART_GET_IT_SOURCE(&huart4,UART_IT_RXNE) == RESET); //等待发送完成
}

//发送一个一个字符串
void UART4_Send_Str(char *str)
{

  while(1)
  {
    if((*str) != 0)
    {
        UART4_SendByte(*str);
        str++;
    }
    else 
    {
        break;
    }
  }
}

u8 ok;

void CheckBusy(void)  
{
	while(1)
	{
   if(ok==0x0f)
		 break;
	}		
	
	ok=0;
}



int UART_GET(void)
{
 while(__HAL_UART_GET_FLAG(&huart4,UART_FLAG_RXNE)==RESET && __HAL_UART_GET_IT_SOURCE(&huart4,UART_IT_RXNE) == RESET); //等待接收完成
 u8 res;
 HAL_UART_Receive(&huart4, &res, 1, 1000);
 return res;
}

void UART4_Send(char *databuf)
{
  // UART4_Send_Str("ADDR(0);");//发送数据前先发送485地址
  UART4_Send_Str(databuf);
}
void UART4_Send_DMA(const char *data, uint16_t len)
{
  if (uart_tx_busy)                 // should never happen – guard anyway
      return;

  uart_tx_busy = 1;                 // mark “busy”

  if (HAL_UART_Transmit_DMA(&huart4,
                            (uint8_t *)uart_tx_buffer,
                            len) != HAL_OK)
  {
      uart_tx_busy = 0;            // transmission failed – clear flag
  }
}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == UART4)
  {
    // 发送完成后清除忙标志
    uart_tx_busy = 0;
  }
}
//2,3,7,8,9,
//angle,angle_speed,Iu,Iv,Iw
