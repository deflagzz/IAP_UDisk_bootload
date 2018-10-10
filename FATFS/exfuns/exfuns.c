#include "exfuns.h"
	
#define FILE_MAX_TYPE_NUM		7	//最多FILE_MAX_TYPE_NUM个大类
#define FILE_MAX_SUBT_NUM		4	//最多FILE_MAX_SUBT_NUM个小类

 //文件类型列表
u8*const FILE_TYPE_TBL[FILE_MAX_TYPE_NUM][FILE_MAX_SUBT_NUM]=
{
{"BIN"},			//BIN文件
{"LRC"},			//LRC文件
{"NES"},			//NES文件
{"TXT","C","H"},	//文本文件
{"WAV","MP3","APE","FLAC"},//支持的音乐文件
{"BMP","JPG","JPEG","GIF"},//图片文件
{"AVI"},			//视频文件
};
///////////////////////////////公共文件区,使用malloc的时候////////////////////////////////////////////
FATFS *fs[_VOLUMES];//逻辑磁盘工作区. //当前只使用U盘_VOLUMES值为1,如使用其他多个盘请修改此值	 
FIL *file;	  		//文件1
FIL *ftemp;	  		//文件2.
UINT br,bw;			//读写变量
FILINFO fileinfo;	//文件信息
DIR dir;  			//目录

u8 g_file_BUFF[560];
u8 g_ftemp_BUFF[560];
u8 g_fatfs_BUFF[512];
u8 *fatfs_buff;			//U盘/SD卡 数据缓存区512字节
///////////////////////////////////////////////////////////////////////////////////////
//为exfuns申请内存
//返回值:0,成功
//1,失败
u8 exfuns_init(void)
{


//	file  = (FIL *)g_file_BUFF;  //552字节
//	ftemp = (FIL *)g_ftemp_BUFF; //552字节
//	fatfs_buff = g_fatfs_BUFF;
//	return 1;
	
	u8 i;
	for(i=0;i<_VOLUMES;i++)
	{
		fs[i]=(FATFS*)mymalloc(SRAMIN,sizeof(FATFS));	//为磁盘i工作区申请内存	
		if(!fs[i])
			break;	//未申请到内存
	}
	file=(FIL*)mymalloc(SRAMIN,sizeof(FIL));		//为file申请内存
	ftemp=(FIL*)mymalloc(SRAMIN,sizeof(FIL));		//为ftemp申请内存
	fatfs_buff=(u8*)mymalloc(SRAMIN,512);			//为fatfs_buff申请内存
	if(i==_VOLUMES&&file&&ftemp&&fatfs_buff)
		return 0;  //申请有一个失败,即失败.
	else 
		return 1;	
}

//将小写字母转为大写字母,如果是数字,则保持不变.
u8 char_upper(u8 c)
{
	if(c<'A')return c;//数字,保持不变.
	if(c>='a')return c-0x20;//变为大写.
	else return c;//大写,保持不变
}	      
//报告文件的类型
//fname:文件名
//返回值:0XFF,表示无法识别的文件类型编号.
//		 其他,高四位表示所属大类,低四位表示所属小类.
u8 f_typetell(u8 *fname)
{
	u8 tbuf[5];
	u8 *attr='\0';//后缀名
	u8 i=0,j;
	while(i<250)
	{
		i++;
		if(*fname=='\0')break;//偏移到了最后了.
		fname++;
	}
	if(i==250)return 0XFF;//错误的字符串.
 	for(i=0;i<5;i++)//得到后缀名
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
 	for(i=0;i<4;i++)tbuf[i]=char_upper(tbuf[i]);//全部变为大写 
	for(i=0;i<FILE_MAX_TYPE_NUM;i++)	//大类对比
	{
		for(j=0;j<FILE_MAX_SUBT_NUM;j++)//子类对比
		{
			if(*FILE_TYPE_TBL[i][j]==0)break;//此组已经没有可对比的成员了.
			if(strcmp((const char *)FILE_TYPE_TBL[i][j],(const char *)tbuf)==0)//找到了
			{
				return (i<<4)|j;
			}
		}
	}
	return 0XFF;//没找到		 			   
}	 

//得到磁盘剩余容量
//drv:磁盘编号("0:"/"1:")
//total:总容量	 （单位KB）
//free:剩余容量	 （单位KB）
//返回值:0,正常.其他,错误代码
u8 exf_getfree(u8 *drv,u32 *total,u32 *free)
{
	FATFS *fs1;
	u8 res;
    u32 fre_clust=0, fre_sect=0, tot_sect=0;
    //得到磁盘信息及空闲簇数量
    res =(u32)f_getfree((const TCHAR*)drv, (DWORD*)&fre_clust, &fs1);
    if(res==0)
	{											   
	    tot_sect=(fs1->n_fatent-2)*fs1->csize;	//得到总扇区数
	    fre_sect=fre_clust*fs1->csize;			//得到空闲扇区数	   
#if _MAX_SS!=512				  				//扇区大小不是512字节,则转换为512字节
		tot_sect*=fs1->ssize/512;
		fre_sect*=fs1->ssize/512;
#endif	  
		*total=tot_sect>>1;	//单位为KB
		*free=fre_sect>>1;	//单位为KB 
 	}
	return res;
}	


//f_getfree();这个函数的返回值为以下内容,调试用
//static const char * FR_Table[]= 
//{
//    "FR_OK：成功",                                      	 /* (0) Succeeded */
//    "FR_DISK_ERR：底层硬件错误",                      		 /* (1) A hard error occurred in the low level disk I/O layer */
//    "FR_INT_ERR：断言失败",                             	 /* (2) Assertion failed */
//    "FR_NOT_READY：物理驱动没有工作",                  		 /* (3) The physical drive cannot work */
//    "FR_NO_FILE：文件不存在",                          		 /* (4) Could not find the file */
//    "FR_NO_PATH：路径不存在",                         		 /* (5) Could not find the path */
//    "FR_INVALID_NAME：无效文件名",                     		 /* (6) The path name format is invalid */
//    "FR_DENIED：由于禁止访问或者目录已满访问被拒绝",  		 /* (7) Access denied due to prohibited access or directory full */
//    "FR_EXIST：由于访问被禁止访问被拒绝",              		 /* (8) Access denied due to prohibited access */
//    "FR_INVALID_OBJECT：文件或者目录对象无效",        		 /* (9) The file/directory object is invalid */
//    "FR_WRITE_PROTECTED：物理驱动被写保护",            	 	 /* (10) The physical drive is write protected */
//    "FR_INVALID_DRIVE：逻辑驱动号无效",                 	 /* (11) The logical drive number is invalid */
//    "FR_NOT_ENABLED：卷中无工作区",                      	 /* (12) The volume has no work area */
//    "FR_NO_FILESYSTEM：没有有效的FAT卷",              		 /* (13) There is no valid FAT volume */
//    "FR_MKFS_ABORTED：由于参数错误f_mkfs()被终止",           /* (14) The f_mkfs() aborted due to any parameter error */
//    "FR_TIMEOUT：在规定的时间内无法获得访问卷的许可",        /* (15) Could not get a grant to access the volume within defined period */
//    "FR_LOCKED：由于文件共享策略操作被拒绝",                 /* (16) The operation is rejected according to the file sharing policy */
//    "FR_NOT_ENOUGH_CORE：无法分配长文件名工作区",            /* (17) LFN working buffer could not be allocated */
//    "FR_TOO_MANY_OPEN_FILES：当前打开的文件数大于_FS_SHARE", /* (18) Number of open files > _FS_SHARE */
//    "FR_INVALID_PARAMETER：参数无效"                         /* (19) Given parameter is invalid */
//};







