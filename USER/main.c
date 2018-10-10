#include "my_include.h"



USBH_HOST  USB_Host;
USB_OTG_CORE_HANDLE  USB_OTG_Core;




//�û�����������
//����ֵ:0,����
//       1,������
u8 USH_User_App(void)
{ 
	u32 total,free;
	u8 res=0;
	u8 time_num=0;
	u32 fil_addr=0;
	u32 bin_size=0;
	u16 read_len=0;


	printf("�豸���ӳɹ�!.!!!\r\n");

	res=exf_getfree("0:",&total,&free);			//�õ�����0:��ʣ������
	if(res==0)
	{
		printf("FATFS OK!\r\n");
		printf("U Disk Total Size:%d     MB\r\n",total>>10);
		printf("U Disk  Free Size:%d     MB\r\n",free>>10);

		if(mf_open("0:/IAP_BIN/IAP_APP.bin",FA_READ))
		{
			return 1; //�����˳�
		}
		
		mf_sync();								//���»���
		bin_size = mf_size();
		mf_lseek(0);							//�����ļ�ָ��
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
		f_close(file);
		
		//
		//���������,��ת��ȥ������
		//
		
		if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//�ж��Ƿ�Ϊ0X08XXXXXX.
		{	 	
			BEEP = 1;
			delay_ms(1000);	
			delay_ms(1000);	
			
			BEEP = 0;
			INTX_DISABLE();	 //U�������һ����־�ļ�,1:���㸴λ����ˢ�̼�
			
			iap_load_app(FLASH_APP1_ADDR);//ִ��FLASH APP����	
		}
		else //��FLASHӦ�ó���,�޷�ִ��
		{
			//��λ
			systeam_ReStart();					
		}		
		
	} 
 
	while(HCD_IsDeviceConnected(&USB_OTG_Core))//�豸���ӳɹ�
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

	printf("�豸������...\r\n");
	
	return res;										
} 


int main(void)
{        
	u16 t;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init(168);  		//��ʼ����ʱ����
	USART3_Configuration(115200);		//��ʼ�����ڲ�����Ϊ115200
	beep_init();
	//LED_Init();				//��ʼ����LED���ӵ�Ӳ���ӿ�
	//W25QXX_Init();			//SPI FLASH��ʼ��
	 
	my_mem_init(SRAMIN);	//��ʼ���ڲ��ڴ��	
 	exfuns_init();			//Ϊfatfs��ر��������ڴ� 

	#if USB_DISK_allow
	f_mount(fs[0],"0:",1); 	//����U��
	#endif
	#if EX_FLASH_allow
	f_mount(fs[1],"1:",1); 	//����SPI Flash
	#endif
	#if SD_CARD_allow	
	f_mount(fs[2],"2:",1); 	//����SD��  
	#endif
	
	printf("�豸������...\r\n");
	//��ʼ��USB����
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
			if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//�ж��Ƿ�Ϊ0X08XXXXXX.
			{	 	
				delay_ms(100);			
				iap_load_app(FLASH_APP1_ADDR);//ִ��FLASH APP����	
			}
			else //��FLASHӦ�ó���,�޷�ִ��
			{
				//��λ
				systeam_ReStart();					
			}			
		}		

	}	
}










