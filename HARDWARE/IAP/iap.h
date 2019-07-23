#ifndef __IAP_H__
#define __IAP_H__
#include "stm32f4xx.h"  

#include "usbh_usr.h" 
#include "usb_core.h"
#include "usb_conf.h"
#include "fattester.h"	

#include "ff.h"  
#include "exfuns.h"   
#include "integer.h"	/* Basic integer types */
#include "ffconf.h"		/* FatFs configuration options */
#include "diskio.h"		/* Declarations of disk I/O functions */
  
  
  
#define IAP_Bootloat_SIZE 		0x8000	//32K 
#define FLASH_APP1_ADDR			(0x08000000+IAP_Bootloat_SIZE) 	//+32*1024	//��һ��Ӧ�ó�����ʼ��ַ(�����FLASH)   
#define CAN_RX_BUFF_LEN			2048			//1K����2KSRAM����CAN�жϵ�����,����MCU��FLASH��С����	
 
  


typedef  void (*iapfun)(void);							//����һ���������͵Ĳ���.  

typedef struct
{
	
	u8  RX_BUFF[CAN_RX_BUFF_LEN];	
	u16 RX_data_len;			//ÿ֡���ݵĳ���
	u8  flag_RX_;
	u8  flag_TX_;
	u8  password;
	u16 TIME_jian;
	u8  RX_err;
	u16 APP_RX_len;				//���յ�bin�ļ��ܳ���
	u32 APP_Bin_len;			//APP��Bin�ļ����ܴ�С
	u32 CRC_PC_vel;
	u8  CRC_buff[4];
	u8  flag_Recive_Bin;
	u8	OK_Recive;
	u32 Flash_write_address;	


}stm32_iap;


extern stm32_iap g_iap;


void IAP_U_Disk_Init(void);
void IAP_U_Disk_UpData(void);

#endif







































