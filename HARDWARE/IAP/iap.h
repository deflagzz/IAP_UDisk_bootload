#ifndef __IAP_H__
#define __IAP_H__
#include "sys.h"  
  
#define IAP_Bootloat_SIZE 		0x8000	//32K
#define FLASH_APP1_ADDR			(0x08000000+IAP_Bootloat_SIZE) 	//+32*1024	//第一个应用程序起始地址(存放在FLASH)   
#define CAN_RX_BUFF_LEN			2048			//1K或者2KSRAM缓存CAN中断的数据,根据MCU的FLASH大小决定	
 
  





typedef  void (*iapfun)(void);							//定义一个函数类型的参数.  

typedef struct
{
	
	u8  RX_BUFF[CAN_RX_BUFF_LEN];	
	u16 RX_data_len;			//每帧数据的长度
	u8  flag_RX_;
	u8  flag_TX_;
	u8  password;
	u16 TIME_jian;
	u8  RX_err;
	u16 APP_RX_len;				//接收的bin文件总长度
	u32 APP_Bin_len;			//APP的Bin文件的总大小
	u32 CRC_PC_vel;
	u8  CRC_buff[4];
	u8  flag_Recive_Bin;
	u8	OK_Recive;
	u32 Flash_write_address;	


}stm32_iap;


extern stm32_iap g_iap;








void iap_load_app(u32 appxaddr);			//跳转到APP程序执行
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 applen);	//在指定地址开始,写入bin

void systeam_ReStart(void);



#endif







































