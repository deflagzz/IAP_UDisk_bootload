#include "sys.h"
#include "delay.h"
#include "usart.h"

#include "stmflash.h"
#include "malloc.h" 
#include "iap.h" 


stm32_iap g_iap;
USBH_HOST  USB_Host;
USB_OTG_CORE_HANDLE  USB_OTG_Core;


iapfun jump2app; 
u32 iapbuf[512]; 	//2K字节缓存  
//appxaddr:应用程序的起始地址
//appbuf:应用程序CODE.
//appsize:应用程序大小(字节).
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize)
{
	u32 t;
	u16 i=0;
	u32 temp;
	u32 fwaddr=appxaddr;//当前写入的地址
	u8 *dfu=appbuf;
	for(t=0;t<appsize;t+=4)
	{						   
		temp=(u32)dfu[3]<<24;   
		temp|=(u32)dfu[2]<<16;    
		temp|=(u32)dfu[1]<<8;
		temp|=(u32)dfu[0];	  
		dfu+=4;//偏移4个字节	
		iapbuf[i++]=temp;	    
		if(i==512)
		{
			i=0; 
			STMFLASH_Write(fwaddr,iapbuf,512);
			fwaddr+=2048;//偏移2048  512*4=2048
		}
	} 
	if(i)STMFLASH_Write(fwaddr,iapbuf,i);//将最后的一些内容字节写进去.  
}

//跳转到应用程序段
//appxaddr:用户代码起始地址.
void iap_load_app(u32 appxaddr)
{
	if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)	//检查栈顶地址是否合法.
	{ 
		jump2app=(iapfun)*(vu32*)(appxaddr+4);		//用户代码区第二个字为程序开始地址(复位地址)		
		MSR_MSP(*(vu32*)appxaddr);					//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
		jump2app();									//跳转到APP.
	}
}		 



void systeam_ReStart(void)
{
	u32 time = 65535;
	while(time--)
	{
		__NOP();
	}
	//__set_FAULTMASK(1);	//M4内核貌似用不了这个函数
	INTX_DISABLE();
	NVIC_SystemReset();
}


void IAP_U_Disk_Init(void)
{
/*************************************U盘IAP*******************************************///	
	my_mem_init(SRAMIN);	//初始化内部内存池											//
 	exfuns_init();			//为fatfs相关变量申请内存 									//																					//
	#if USB_DISK_allow                                                                  //
	f_mount(fs[0],"0:",1); 	//挂载U盘                                                   //
	#endif                                                                              //
	#if EX_FLASH_allow                                                                  //
	f_mount(fs[1],"1:",1); 	//挂载SPI Flash												//
	#endif																				//
	#if SD_CARD_allow																	//
	f_mount(fs[2],"2:",1); 	//挂载SD卡  												//
	#endif																				//																				//
//	printf("设备连接中...\r\n");														//
	//初始化USB主机																		//
  	USBH_Init(&USB_OTG_Core,USB_OTG_FS_CORE_ID,&USB_Host,&USBH_MSC_cb,&USR_Callbacks);  // 
/*************************************U盘IAP*******************************************///	
}

//用户回调主程序
//返回值:0,正常
//       1,有问题
u8 USH_User_App(void)
{ 
	u32 total,free;
	u8 res=0;
	u8 file_res=0;
	u32 fil_addr=0;
	u32 bin_size=0;
	u16 read_len=0;
	u8  temp_return=0;


//	printf("设备连接成功!.!!!\r\n");
	BEEP = 0;
	//获取磁盘0:的剩余容量
	res=exf_getfree("0:",&total,&free);			
	if(!res) //0:成功
	{
//		printf("FATFS OK!\r\n");
//		printf("U Disk Total Size:%d     MB\r\n",total>>10);
//		printf("U Disk  Free Size:%d     MB\r\n",free>>10) ;
		
		//检测U盘内是否有bin文件--0:ok
		file_res = mf_open("0:/IAP_BIN/IAP_APP.bin",FA_READ);
		if(!file_res)
		{
			//读取bin文件成功
			temp_return	= 0;	
			
			//更新缓存,类似于点保存
			mf_sync();	
			//获取bin文件大小		
			bin_size = mf_size();
			//设置文件指针回零		
			mf_lseek(0);							
			//开始接收bin文件并存入flash
			while(fil_addr < bin_size)
			{
				if((bin_size - fil_addr) < CAN_RX_BUFF_LEN)
				{
					read_len = bin_size - fil_addr;
					if(!mf_read(read_len))				//不足2K数据,读取剩余部分
					{
						fil_addr = mf_tell();
						if(fil_addr%CAN_RX_BUFF_LEN == read_len)
						{	
							iap_write_appbin(FLASH_APP1_ADDR+fil_addr-read_len,g_iap.RX_BUFF,read_len);			//存入flash	
						}						
					}
					else
					{	
						break;
					}			
				}
				else
				{
					if(!mf_read(CAN_RX_BUFF_LEN))		//足2K,读2K数据
					{
						fil_addr = mf_tell();
						if(fil_addr%CAN_RX_BUFF_LEN == 0)
						{
							iap_write_appbin(FLASH_APP1_ADDR+fil_addr-CAN_RX_BUFF_LEN,g_iap.RX_BUFF,CAN_RX_BUFF_LEN);			//存入flash	
						}						
					}
					else
					{	
						break;
					}			
				}
			
			}
			//关闭文件			
			mf_close();
			//判断flash内是否有正确的bin文件并执行
			//判断是否为0X08XXXXXX.
			if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)
			{	 
							
				BEEP = 1;
				delay_ms(500);	
				BEEP = 0;
				//关闭中断
				INTX_DISABLE();	 
				//执行FLASH APP代码	
				delay_ms(100);	
				iap_load_app(FLASH_APP1_ADDR);
			}
			#if 0 //置0,屏蔽掉以下函数
			else //非FLASH应用程序,无法执行
			{
				//软复位
				systeam_ReStart();					
			}	
			#endif	
	
		}
		else
		{
			//读取bin文件失败
			temp_return	= 1;
		}	
	} 
	else
	{
		//失败
		temp_return	= 1;
	}
	//设备连接成功,等待设备拔出
	//磁盘读取失败或者非Flash程序,在这里等待设备拔出
	while(HCD_IsDeviceConnected(&USB_OTG_Core))
	{	
		BEEP = !BEEP;
		delay_ms(50);						
	}												

	return res;										
}



void IAP_U_Disk_UpData(void)
{
	u16 time=0;
	u16 temp_t=0;
	
	while(1)
	{
		USBH_Process(&USB_OTG_Core, &USB_Host);
		delay_ms(1);
		time++;
		if(time == 200)
		{
			BEEP=1;
			delay_ms(20);
			BEEP=0;
			temp_t++;
			time=0;
		}
		//2秒开始执行APP程序
		if(temp_t >= 3)				
		{
			BEEP = 0;
			temp_t=0;
			//判断是否为0X08XXXXXX.
			if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)
			{	 	
				delay_ms(1000);
				BEEP = 1;
				delay_ms(500);	
				BEEP = 0;
				delay_ms(100);	
				//执行FLASH APP代码			
				iap_load_app(FLASH_APP1_ADDR);	
			}
			else //非FLASH应用程序,无法执行
			{
				//软复位
				systeam_ReStart();					
			}			
		}	
	}

}







