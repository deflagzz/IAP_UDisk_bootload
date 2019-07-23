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
u32 iapbuf[512]; 	//2K�ֽڻ���  
//appxaddr:Ӧ�ó������ʼ��ַ
//appbuf:Ӧ�ó���CODE.
//appsize:Ӧ�ó����С(�ֽ�).
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize)
{
	u32 t;
	u16 i=0;
	u32 temp;
	u32 fwaddr=appxaddr;//��ǰд��ĵ�ַ
	u8 *dfu=appbuf;
	for(t=0;t<appsize;t+=4)
	{						   
		temp=(u32)dfu[3]<<24;   
		temp|=(u32)dfu[2]<<16;    
		temp|=(u32)dfu[1]<<8;
		temp|=(u32)dfu[0];	  
		dfu+=4;//ƫ��4���ֽ�	
		iapbuf[i++]=temp;	    
		if(i==512)
		{
			i=0; 
			STMFLASH_Write(fwaddr,iapbuf,512);
			fwaddr+=2048;//ƫ��2048  512*4=2048
		}
	} 
	if(i)STMFLASH_Write(fwaddr,iapbuf,i);//������һЩ�����ֽ�д��ȥ.  
}

//��ת��Ӧ�ó����
//appxaddr:�û�������ʼ��ַ.
void iap_load_app(u32 appxaddr)
{
	if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)	//���ջ����ַ�Ƿ�Ϸ�.
	{ 
		jump2app=(iapfun)*(vu32*)(appxaddr+4);		//�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)		
		MSR_MSP(*(vu32*)appxaddr);					//��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
		jump2app();									//��ת��APP.
	}
}		 



void systeam_ReStart(void)
{
	u32 time = 65535;
	while(time--)
	{
		__NOP();
	}
	//__set_FAULTMASK(1);	//M4�ں�ò���ò����������
	INTX_DISABLE();
	NVIC_SystemReset();
}


void IAP_U_Disk_Init(void)
{
/*************************************U��IAP*******************************************///	
	my_mem_init(SRAMIN);	//��ʼ���ڲ��ڴ��											//
 	exfuns_init();			//Ϊfatfs��ر��������ڴ� 									//																					//
	#if USB_DISK_allow                                                                  //
	f_mount(fs[0],"0:",1); 	//����U��                                                   //
	#endif                                                                              //
	#if EX_FLASH_allow                                                                  //
	f_mount(fs[1],"1:",1); 	//����SPI Flash												//
	#endif																				//
	#if SD_CARD_allow																	//
	f_mount(fs[2],"2:",1); 	//����SD��  												//
	#endif																				//																				//
//	printf("�豸������...\r\n");														//
	//��ʼ��USB����																		//
  	USBH_Init(&USB_OTG_Core,USB_OTG_FS_CORE_ID,&USB_Host,&USBH_MSC_cb,&USR_Callbacks);  // 
/*************************************U��IAP*******************************************///	
}

//�û��ص�������
//����ֵ:0,����
//       1,������
u8 USH_User_App(void)
{ 
	u32 total,free;
	u8 res=0;
	u8 file_res=0;
	u32 fil_addr=0;
	u32 bin_size=0;
	u16 read_len=0;
	u8  temp_return=0;


//	printf("�豸���ӳɹ�!.!!!\r\n");
	BEEP = 0;
	//��ȡ����0:��ʣ������
	res=exf_getfree("0:",&total,&free);			
	if(!res) //0:�ɹ�
	{
//		printf("FATFS OK!\r\n");
//		printf("U Disk Total Size:%d     MB\r\n",total>>10);
//		printf("U Disk  Free Size:%d     MB\r\n",free>>10) ;
		
		//���U�����Ƿ���bin�ļ�--0:ok
		file_res = mf_open("0:/IAP_BIN/IAP_APP.bin",FA_READ);
		if(!file_res)
		{
			//��ȡbin�ļ��ɹ�
			temp_return	= 0;	
			
			//���»���,�����ڵ㱣��
			mf_sync();	
			//��ȡbin�ļ���С		
			bin_size = mf_size();
			//�����ļ�ָ�����		
			mf_lseek(0);							
			//��ʼ����bin�ļ�������flash
			while(fil_addr < bin_size)
			{
				if((bin_size - fil_addr) < CAN_RX_BUFF_LEN)
				{
					read_len = bin_size - fil_addr;
					if(!mf_read(read_len))				//����2K����,��ȡʣ�ಿ��
					{
						fil_addr = mf_tell();
						if(fil_addr%CAN_RX_BUFF_LEN == read_len)
						{	
							iap_write_appbin(FLASH_APP1_ADDR+fil_addr-read_len,g_iap.RX_BUFF,read_len);			//����flash	
						}						
					}
					else
					{	
						break;
					}			
				}
				else
				{
					if(!mf_read(CAN_RX_BUFF_LEN))		//��2K,��2K����
					{
						fil_addr = mf_tell();
						if(fil_addr%CAN_RX_BUFF_LEN == 0)
						{
							iap_write_appbin(FLASH_APP1_ADDR+fil_addr-CAN_RX_BUFF_LEN,g_iap.RX_BUFF,CAN_RX_BUFF_LEN);			//����flash	
						}						
					}
					else
					{	
						break;
					}			
				}
			
			}
			//�ر��ļ�			
			mf_close();
			//�ж�flash���Ƿ�����ȷ��bin�ļ���ִ��
			//�ж��Ƿ�Ϊ0X08XXXXXX.
			if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)
			{	 
							
				BEEP = 1;
				delay_ms(500);	
				BEEP = 0;
				//�ر��ж�
				INTX_DISABLE();	 
				//ִ��FLASH APP����	
				delay_ms(100);	
				iap_load_app(FLASH_APP1_ADDR);
			}
			#if 0 //��0,���ε����º���
			else //��FLASHӦ�ó���,�޷�ִ��
			{
				//��λ
				systeam_ReStart();					
			}	
			#endif	
	
		}
		else
		{
			//��ȡbin�ļ�ʧ��
			temp_return	= 1;
		}	
	} 
	else
	{
		//ʧ��
		temp_return	= 1;
	}
	//�豸���ӳɹ�,�ȴ��豸�γ�
	//���̶�ȡʧ�ܻ��߷�Flash����,������ȴ��豸�γ�
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
		//2�뿪ʼִ��APP����
		if(temp_t >= 3)				
		{
			BEEP = 0;
			temp_t=0;
			//�ж��Ƿ�Ϊ0X08XXXXXX.
			if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)
			{	 	
				delay_ms(1000);
				BEEP = 1;
				delay_ms(500);	
				BEEP = 0;
				delay_ms(100);	
				//ִ��FLASH APP����			
				iap_load_app(FLASH_APP1_ADDR);	
			}
			else //��FLASHӦ�ó���,�޷�ִ��
			{
				//��λ
				systeam_ReStart();					
			}			
		}	
	}

}







