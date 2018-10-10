#include "my_include.h"



USBH_HOST  USB_Host;
USB_OTG_CORE_HANDLE  USB_OTG_Core;




//用户测试主程序
//返回值:0,正常
//       1,有问题
u8 USH_User_App(void)
{ 
	u32 total,free;
	u8 res=0;
	u8 time_num=0;
	u32 fil_addr=0;
	u32 bin_size=0;
	u16 read_len=0;


	printf("设备连接成功!.!!!\r\n");

	res=exf_getfree("0:",&total,&free);			//得到磁盘0:的剩余容量
	if(res==0)
	{
		printf("FATFS OK!\r\n");
		printf("U Disk Total Size:%d     MB\r\n",total>>10);
		printf("U Disk  Free Size:%d     MB\r\n",free>>10);

		if(mf_open("0:/IAP_BIN/IAP_APP.bin",FA_READ))
		{
			return 1; //出错退出
		}
		
		mf_sync();								//更新缓存
		bin_size = mf_size();
		mf_lseek(0);							//设置文件指针
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
		f_close(file);
		
		//
		//这块有问题,跳转过去后死机
		//
		
		if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
		{	 	
			BEEP = 1;
			delay_ms(1000);	
			delay_ms(1000);	
			
			BEEP = 0;
			INTX_DISABLE();	 //U盘内添加一个标志文件,1:清零复位重启刷固件
			
			iap_load_app(FLASH_APP1_ADDR);//执行FLASH APP代码	
		}
		else //非FLASH应用程序,无法执行
		{
			//软复位
			systeam_ReStart();					
		}		
		
	} 
 
	while(HCD_IsDeviceConnected(&USB_OTG_Core))//设备连接成功
	{	
//		time_num++;
//		if(time_num == 95)
//		{
//			BEEP = 1;
//		}
//		else if(time_num > 100)
//		{
//			time_num = 0;
//			BEEP = 0;
//		}
		delay_ms(10);						
	}												

	printf("设备连接中...\r\n");
	
	return res;										
} 


int main(void)
{        
	u16 t;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);  		//初始化延时函数
	USART3_Configuration(115200);		//初始化串口波特率为115200
	beep_init();
	//LED_Init();				//初始化与LED连接的硬件接口
	//W25QXX_Init();			//SPI FLASH初始化
	 
	my_mem_init(SRAMIN);	//初始化内部内存池	
 	exfuns_init();			//为fatfs相关变量申请内存 

	#if USB_DISK_allow
	f_mount(fs[0],"0:",1); 	//挂载U盘
	#endif
	#if EX_FLASH_allow
	f_mount(fs[1],"1:",1); 	//挂载SPI Flash
	#endif
	#if SD_CARD_allow	
	f_mount(fs[2],"2:",1); 	//挂载SD卡  
	#endif
	
	printf("设备连接中...\r\n");
	//初始化USB主机
  	USBH_Init(&USB_OTG_Core,USB_OTG_FS_CORE_ID,&USB_Host,&USBH_MSC_cb,&USR_Callbacks);  
	while(1)
	{
		USBH_Process(&USB_OTG_Core, &USB_Host);
		delay_ms(1);
		t++;
		if(t==200)
		{
			//BEEP=!BEEP;
//			t=0;
		}
		
		if(t==2000)
		{
			//BEEP=!BEEP;
			t=0;
			if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
			{	 	
				delay_ms(100);			
				iap_load_app(FLASH_APP1_ADDR);//执行FLASH APP代码	
			}
			else //非FLASH应用程序,无法执行
			{
				//软复位
				systeam_ReStart();					
			}			
		}		

	}	
}










