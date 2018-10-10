#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h" 


#define g_battery_RXbuff_len 128
#define g_battery_TXbuff_len 128


extern u8 g_battery_TXbuff[g_battery_TXbuff_len];


extern char g_warning[256];


void USART3_Configuration(u32 bound);

void DMA_Uart3_Init(void);  //锂电池电量监测232转TTL

void Uart3_Start_DMA_Tx(u16 size);



#endif
