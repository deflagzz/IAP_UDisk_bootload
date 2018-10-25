#include "my_include.h"



int main(void)
{        
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//设置系统中断优先级分组2
	delay_init(168);  								//初始化延时函数
	USART3_Configuration(115200);					//初始化串口波特率为115200
	beep_init();
	
	IAP_U_Disk_Init();								//初始化USB
	
	while(1)
	{
		IAP_U_Disk_UpData();						//等待U盘更新数据
	}	
}










