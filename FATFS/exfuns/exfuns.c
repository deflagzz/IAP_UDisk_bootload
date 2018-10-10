#include "exfuns.h"
	
#define FILE_MAX_TYPE_NUM		7	//���FILE_MAX_TYPE_NUM������
#define FILE_MAX_SUBT_NUM		4	//���FILE_MAX_SUBT_NUM��С��

 //�ļ������б�
u8*const FILE_TYPE_TBL[FILE_MAX_TYPE_NUM][FILE_MAX_SUBT_NUM]=
{
{"BIN"},			//BIN�ļ�
{"LRC"},			//LRC�ļ�
{"NES"},			//NES�ļ�
{"TXT","C","H"},	//�ı��ļ�
{"WAV","MP3","APE","FLAC"},//֧�ֵ������ļ�
{"BMP","JPG","JPEG","GIF"},//ͼƬ�ļ�
{"AVI"},			//��Ƶ�ļ�
};
///////////////////////////////�����ļ���,ʹ��malloc��ʱ��////////////////////////////////////////////
FATFS *fs[_VOLUMES];//�߼����̹�����. //��ǰֻʹ��U��_VOLUMESֵΪ1,��ʹ��������������޸Ĵ�ֵ	 
FIL *file;	  		//�ļ�1
FIL *ftemp;	  		//�ļ�2.
UINT br,bw;			//��д����
FILINFO fileinfo;	//�ļ���Ϣ
DIR dir;  			//Ŀ¼

u8 g_file_BUFF[560];
u8 g_ftemp_BUFF[560];
u8 g_fatfs_BUFF[512];
u8 *fatfs_buff;			//U��/SD�� ���ݻ�����512�ֽ�
///////////////////////////////////////////////////////////////////////////////////////
//Ϊexfuns�����ڴ�
//����ֵ:0,�ɹ�
//1,ʧ��
u8 exfuns_init(void)
{


//	file  = (FIL *)g_file_BUFF;  //552�ֽ�
//	ftemp = (FIL *)g_ftemp_BUFF; //552�ֽ�
//	fatfs_buff = g_fatfs_BUFF;
//	return 1;
	
	u8 i;
	for(i=0;i<_VOLUMES;i++)
	{
		fs[i]=(FATFS*)mymalloc(SRAMIN,sizeof(FATFS));	//Ϊ����i�����������ڴ�	
		if(!fs[i])
			break;	//δ���뵽�ڴ�
	}
	file=(FIL*)mymalloc(SRAMIN,sizeof(FIL));		//Ϊfile�����ڴ�
	ftemp=(FIL*)mymalloc(SRAMIN,sizeof(FIL));		//Ϊftemp�����ڴ�
	fatfs_buff=(u8*)mymalloc(SRAMIN,512);			//Ϊfatfs_buff�����ڴ�
	if(i==_VOLUMES&&file&&ftemp&&fatfs_buff)
		return 0;  //������һ��ʧ��,��ʧ��.
	else 
		return 1;	
}

//��Сд��ĸתΪ��д��ĸ,���������,�򱣳ֲ���.
u8 char_upper(u8 c)
{
	if(c<'A')return c;//����,���ֲ���.
	if(c>='a')return c-0x20;//��Ϊ��д.
	else return c;//��д,���ֲ���
}	      
//�����ļ�������
//fname:�ļ���
//����ֵ:0XFF,��ʾ�޷�ʶ����ļ����ͱ��.
//		 ����,����λ��ʾ��������,����λ��ʾ����С��.
u8 f_typetell(u8 *fname)
{
	u8 tbuf[5];
	u8 *attr='\0';//��׺��
	u8 i=0,j;
	while(i<250)
	{
		i++;
		if(*fname=='\0')break;//ƫ�Ƶ��������.
		fname++;
	}
	if(i==250)return 0XFF;//������ַ���.
 	for(i=0;i<5;i++)//�õ���׺��
	{
		fname--;
		if(*fname=='.')
		{
			fname++;
			attr=fname;
			break;
		}
  	}
	strcpy((char *)tbuf,(const char*)attr);//copy
 	for(i=0;i<4;i++)tbuf[i]=char_upper(tbuf[i]);//ȫ����Ϊ��д 
	for(i=0;i<FILE_MAX_TYPE_NUM;i++)	//����Ա�
	{
		for(j=0;j<FILE_MAX_SUBT_NUM;j++)//����Ա�
		{
			if(*FILE_TYPE_TBL[i][j]==0)break;//�����Ѿ�û�пɶԱȵĳ�Ա��.
			if(strcmp((const char *)FILE_TYPE_TBL[i][j],(const char *)tbuf)==0)//�ҵ���
			{
				return (i<<4)|j;
			}
		}
	}
	return 0XFF;//û�ҵ�		 			   
}	 

//�õ�����ʣ������
//drv:���̱��("0:"/"1:")
//total:������	 ����λKB��
//free:ʣ������	 ����λKB��
//����ֵ:0,����.����,�������
u8 exf_getfree(u8 *drv,u32 *total,u32 *free)
{
	FATFS *fs1;
	u8 res;
    u32 fre_clust=0, fre_sect=0, tot_sect=0;
    //�õ�������Ϣ�����д�����
    res =(u32)f_getfree((const TCHAR*)drv, (DWORD*)&fre_clust, &fs1);
    if(res==0)
	{											   
	    tot_sect=(fs1->n_fatent-2)*fs1->csize;	//�õ���������
	    fre_sect=fre_clust*fs1->csize;			//�õ�����������	   
#if _MAX_SS!=512				  				//������С����512�ֽ�,��ת��Ϊ512�ֽ�
		tot_sect*=fs1->ssize/512;
		fre_sect*=fs1->ssize/512;
#endif	  
		*total=tot_sect>>1;	//��λΪKB
		*free=fre_sect>>1;	//��λΪKB 
 	}
	return res;
}	


//f_getfree();��������ķ���ֵΪ��������,������
//static const char * FR_Table[]= 
//{
//    "FR_OK���ɹ�",                                      	 /* (0) Succeeded */
//    "FR_DISK_ERR���ײ�Ӳ������",                      		 /* (1) A hard error occurred in the low level disk I/O layer */
//    "FR_INT_ERR������ʧ��",                             	 /* (2) Assertion failed */
//    "FR_NOT_READY����������û�й���",                  		 /* (3) The physical drive cannot work */
//    "FR_NO_FILE���ļ�������",                          		 /* (4) Could not find the file */
//    "FR_NO_PATH��·��������",                         		 /* (5) Could not find the path */
//    "FR_INVALID_NAME����Ч�ļ���",                     		 /* (6) The path name format is invalid */
//    "FR_DENIED�����ڽ�ֹ���ʻ���Ŀ¼�������ʱ��ܾ�",  		 /* (7) Access denied due to prohibited access or directory full */
//    "FR_EXIST�����ڷ��ʱ���ֹ���ʱ��ܾ�",              		 /* (8) Access denied due to prohibited access */
//    "FR_INVALID_OBJECT���ļ�����Ŀ¼������Ч",        		 /* (9) The file/directory object is invalid */
//    "FR_WRITE_PROTECTED������������д����",            	 	 /* (10) The physical drive is write protected */
//    "FR_INVALID_DRIVE���߼���������Ч",                 	 /* (11) The logical drive number is invalid */
//    "FR_NOT_ENABLED�������޹�����",                      	 /* (12) The volume has no work area */
//    "FR_NO_FILESYSTEM��û����Ч��FAT��",              		 /* (13) There is no valid FAT volume */
//    "FR_MKFS_ABORTED�����ڲ�������f_mkfs()����ֹ",           /* (14) The f_mkfs() aborted due to any parameter error */
//    "FR_TIMEOUT���ڹ涨��ʱ�����޷���÷��ʾ�����",        /* (15) Could not get a grant to access the volume within defined period */
//    "FR_LOCKED�������ļ�������Բ������ܾ�",                 /* (16) The operation is rejected according to the file sharing policy */
//    "FR_NOT_ENOUGH_CORE���޷����䳤�ļ���������",            /* (17) LFN working buffer could not be allocated */
//    "FR_TOO_MANY_OPEN_FILES����ǰ�򿪵��ļ�������_FS_SHARE", /* (18) Number of open files > _FS_SHARE */
//    "FR_INVALID_PARAMETER��������Ч"                         /* (19) Given parameter is invalid */
//};







