#include "my_include.h"



int main(void)
{        
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//����ϵͳ�ж����ȼ�����2
	delay_init(168);  								//��ʼ����ʱ����
	beep_init();		
	
	IAP_U_Disk_Init();								//��ʼ��USB
	
	while(1)
	{
		IAP_U_Disk_UpData();						//�ȴ�U�̲����������
	}	
}










