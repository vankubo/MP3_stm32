#include "mp3player.h"
#include "vs10xx.h"	 
#include "delay.h"
#include "led.h"	 
#include "malloc.h"
#include "string.h"
#include "exfuns.h"  
#include "ff.h"   
#include "flac.h"	
#include "usart.h"	
#include "dma.h"
#include "system.h"
 #include "hmi.h"
#include "decode.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板V3
//MP3播放驱动 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/1/20
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved								  						    								  
//////////////////////////////////////////////////////////////////////////////////
//============================
//系统全局资源
extern  u8 *Databuf;//音频缓冲池@main.c
extern  FIL* fmp3;//全局文件描述符@main.c
extern uint16_t count;//内存池字节偏移量@main.c
extern u8 res;//文件读取结果@main.c
extern uint16_t brt;//实际读取字节@main.c
extern uint8_t reload;//内存池刷新标志@main.c
extern uint8_t *filename;//文件名@main.c
extern uint8_t state;//系统全局标志,offset=0x00@main.c
extern char key_pressed;//按键标志@bsp_exti.c
//================
u16 f_kbps=0;//歌曲文件位率	

//显示当前音量
void mp3_vol_show(u8 vol)
{			    
	//LCD_ShowString(30+110,230,200,16,16,"VOL:");	  	  
	//LCD_ShowxNum(30+142,230,vol,2,16,0X80); 	//显示音量	 
}

//显示播放时间,比特率 信息 
//lenth:歌曲总长度
void mp3_msg_show(u32 lenth)
{	
	static u16 playtime=0;//播放时间标记	     
 	u16 time=0;// 时间变量
	u16 temp=0;	  
	if(f_kbps==0xffff)//未更新过
	{
		playtime=0;
		f_kbps=VS_Get_HeadInfo();	   //获得比特率
	}	 	 
	time=VS_Get_DecodeTime(); //得到解码时间
	if(playtime==0)playtime=time;
	else if((time!=playtime)&&(time!=0))//1s时间到,更新显示数据
	{
		playtime=time;//更新时间 	 				    
		temp=VS_Get_HeadInfo(); //获得比特率	   				 
		if(temp!=f_kbps)
		{
			f_kbps=temp;//更新KBPS	  				     
		}			 
		//显示播放时间			 
		//LCD_ShowxNum(30,210,time/60,2,16,0X80);		//分钟
	//	LCD_ShowChar(30+16,210,':',16,0);
		//LCD_ShowxNum(30+24,210,time%60,2,16,0X80);	//秒钟		
 	//	LCD_ShowChar(30+40,210,'/',16,0); 	    	 
		//显示总时间
		if(f_kbps)time=(lenth/f_kbps)/125;//得到秒钟数   (文件长度(字节)/(1000/8)/比特率=持续秒钟数    	  
		else time=0;//非法位率	  
 	//	LCD_ShowxNum(30+48,210,time/60,2,16,0X80);	//分钟
		//LCD_ShowChar(30+64,210,':',16,0);
		//LCD_ShowxNum(30+72,210,time%60,2,16,0X80);	//秒钟	  		    
		//显示位率			   
   	//	LCD_ShowxNum(30+110,210,f_kbps,3,16,0X80); 	//显示位率	 
	//	LCD_ShowString(30+134,210,200,16,16,"Kbps");	  	  
		LED0=!LED0;		//DS0翻转
	}   		 
}			  



//播放指定歌曲
u8 play_a_song(u8 *filename)
{
		FIL* fmp3;
    u16 br;
	u8 res,rval;	  
	u8 *databuf;	   		   
	u16 i=0; 
	u8 key;  
	
	//###################################
	VS_HD_Reset();//vs1053硬件复位
	VS_Soft_Reset();
	vsset.mvol=160;						//默认设置音量为200.
	mp3_vol_show((vsset.mvol-100)/5);	//音量限制在:100~250,显示的时候,按照公式(vol-100)/5,显示,也就是0~30 
  //########################################
	
		    
	fmp3=(FIL*)mymalloc(SRAMIN,sizeof(FIL));//申请文件描述符内存
	databuf=(u8*)mymalloc(SRAMIN,4096);		//开辟4096字节的内存区域
	if(databuf==NULL||fmp3==NULL)rval=0XFF ;//内存申请失败.
	
	rval=0;
	if(rval==0)
	{	  
	  	VS_Restart_Play();  					//重启播放 
		VS_Set_All(0);        					//设置音量等信息 			 
		VS_Reset_DecodeTime();					//复位解码时间 
	  
		res=f_typetell(filename);	 	 			//得到文件后缀	 			  	 						 
		if(res==0x4c)//如果是flac,加载patch
		{	
			VS_Load_Patch((u16*)vs1053b_patch,VS1053B_PATCHLEN);
		}  				 		   		
		
		res=f_open(fmp3,(const TCHAR*)filename,FA_READ);//打开文件	 
 		if(res==0)//打开成功.
		{ 
			VS_SPI_SpeedHigh();	//高速						   
			while(rval==0)
			{
				res=f_read(fmp3,databuf,4096,(UINT*)&br);//读出4096个字节 
				if(res!=FR_OK)//读取文件出错
				{
					rval=0;
					break;
				}
				i=0;
				do//主播放循环VS_Send_MusicData(databuf+i)
			    {  	
					if(VS_Send_MusicData(databuf+i)==0)//给VS10XX发送音频数据
					{
						i+=32;
					}
				}while(i<br);//循环发送br个字节 
					
				
				if(br!=4096)//读取字节数不到4096字节，表示文件已读完
				{
					rval=0;
					break;//读完了.		  
				} 		
			}
			f_close(fmp3);
			
		}else rval=0XFF;//出现错误	   	  
	}						
	
	myfree(SRAMIN,databuf);	  	 		  	    
	myfree(SRAMIN,fmp3);
	return rval;	  	 		  	    
}


/*uint8_t dma_play_a_song(uint8_t *mp3name)
{
	
		FIL* fmp3;
   
	u8 res,rval;	  
		   		   
	u16 i=0; 
	u8 key;  
	
	u8 start=1;
	//###################################
	VS_HD_Reset();//vs1053硬件复位
	VS_Soft_Reset();
	vsset.mvol=220;						//默认设置音量为200.
	//mp3_vol_show((vsset.mvol-100)/5);	//音量限制在:100~250,显示的时候,按照公式(vol-100)/5,显示,也就是0~30 
  //########################################
	
		    
	fmp3=(FIL*)mymalloc(SRAMIN,sizeof(FIL));//申请文件描述符内存
	
	if(Databuf==NULL||fmp3==NULL)rval=0XFF ;//内存申请失败.
	
	rval=0;
	if(rval==0)
	{	  
	  	VS_Restart_Play();  					//重启播放 
		VS_Set_All(1);        					//设置音量等信息 			 
		VS_Reset_DecodeTime();					//复位解码时间 
	  
		res=f_typetell(mp3name);	 	 			//得到文件后缀	 			  	 						 
		if(res==0x4c)//如果是flac,加载patch
		{	
			VS_Load_Patch((u16*)vs1053b_patch,VS1053B_PATCHLEN);
		}  				 		   		
		
		res=f_open(fmp3,(const TCHAR*)mp3name,FA_READ);//打开文件	 
 		if(res==0)//打开成功.
		{ 
			printf("open file success\n");
			VS_SPI_SpeedHigh();	//高速						   
			//==============
			
			
			
			reload=1;
			start=1;
			do{
				if(reload==1)
				{
					printf("reload\n");
					res=f_read(fmp3,Databuf,4096,(UINT*)&brt);//读出4096个字节 
					reload=0;
				}
				if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13)==1)
				{
					//start=0;
					//printf("send%d\n",i);
					 //MYDMA_Config(DMA1_Channel3,(u32)&SPI1->DR,(u32)Datdbuf,32);
	 //DMA1_NVICcfg(DMA1_Channel3_IRQn,1,1);//dma中断分组
	 //先进行一次无效传输，使dma进入工作状态？
					MYDMA_Config(DMA1_Channel3,(u32)&SPI1->DR,(u32)(Databuf+i),DATA_LEN);
					VS_XDCS=0;
	 SPI_I2S_DMACmd(SPI1,SPI_I2S_DMAReq_Tx, ENABLE);
		MYDMA_Enable(DMA1_Channel3);//开始一次DMA传输
					//EXTIX_Init();
					
					i+=DATA_LEN;
			while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);//等待spi外设数据发送完成，避免XDCS过早拉高造成数据丢失
			//delay_ms(5);
			VS_XDCS=1;
				}
				if(i>(4096-DATA_LEN))
				{
					reload=1;
					i=0;
				}
				
			}while(res==FR_OK&&brt==4096);
				
			//=============
			f_close(fmp3);
			
		}else rval=0XFF;//出现错误	   	  
	}						
	  	 		  	    
	myfree(SRAMIN,fmp3);
	return rval;	  	 		  	    
}

*/

//得到path路径下,目标文件的总个数
//path:路径		    
//返回值:总有效文件数
u16 mp3_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//临时目录
	FILINFO tfileinfo;	//临时文件信息		
	u8 *fn; 			 			   			     
    res=f_opendir(&tdir,(const TCHAR*)path); //打开目录
  	tfileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
	tfileinfo.lfname=mymalloc(SRAMIN,tfileinfo.lfsize);	//为长文件缓存区分配内存
	if(res==FR_OK&&tfileinfo.lfname!=NULL)
	{
		while(1)//查询总的有效文件数
		{
	        res=f_readdir(&tdir,&tfileinfo);       		//读取目录下的一个文件
	        if(res!=FR_OK||tfileinfo.fname[0]==0)break;	//错误了/到末尾了,退出		  
     		fn=(u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X40)//取高四位,看看是不是音乐文件	
			{
				rval++;//有效文件数增加1
			}	    
		}  
	} 
	myfree(SRAMIN,tfileinfo.lfname);
	return rval;
}


void mp3_get_flist(u16 totmp3num,_MP3_INFO *_info)//获取MP3文件索引
{
	
	u8 res;
 	DIR mp3dir;	 		//目录
	FILINFO mp3fileinfo;//文件信息
	u8 *fn;   			//长文件名
	u8 *pname;			//带路径的文件名
	u16 curindex;		//图片当前索引
 	u16 temp;
	u16 *mp3indextbl;	//音乐索引表 
	//_MP3_INFO *filelist;
	
	_info->filenum=totmp3num;
					   
  mp3fileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
	mp3fileinfo.lfname=mymalloc(SRAMIN,mp3fileinfo.lfsize);	//为长文件缓存区分配内存
 	pname=mymalloc(SRAMIN,mp3fileinfo.lfsize);				//为带路径的文件名分配内存
 	mp3indextbl=mymalloc(SRAMIN,2*totmp3num);				//申请2*totmp3num个字节的内存,用于存放音乐文件索引
	//记录索引
    res=f_opendir(&mp3dir,"0:/MUSIC"); //打开目录
	if(res==FR_OK)
	{
		curindex=0;//当前索引为0
		while(1)//全部查询一遍
		{
			temp=mp3dir.index;								//记录当前index
	        res=f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
	        if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//错误了/到末尾了,退出		  
     		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);				
			res=f_typetell(fn);	
			if((res&0XF0)==0X40)//取高四位,看看是不是音乐文件	
			{
				mp3indextbl[curindex]=temp;//记录索引
				curindex++;
			}	    
		} 
	}   
	_info->filelist=mp3indextbl;
	
	/*
   	curindex=0;											//从0开始显示
   	res=f_opendir(&mp3dir,(const TCHAR*)"0:/MUSIC"); 	//打开目录
	while(res==FR_OK)//打开成功
	{	
		dir_sdi(&mp3dir,mp3indextbl[curindex]);			//改变当前目录索引
		LCD_ShowNum(30,500,mp3indextbl[curindex],5,16);		
        res=f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
        if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//错误了/到末尾了,退出
     	fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);			 
	
		strcpy((char*)pname,"0:/MUSIC/");				//复制路径(目录)
		strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
		//申请空间
 	LCD_Fill(30,50,240,50+16,WHITE);				//清除之前的显示
	Show_Str(30,50,240-30,16,fn,16,0);				//显示歌曲名字 
			
		delay_ms(800);	
		curindex++;
	}
	*/
	myfree(SRAMIN,mp3fileinfo.lfname);	//释放内存			    
	myfree(SRAMIN,pname);				//释放内存			   
	//myfree(SRAMIN,mp3indextbl);			//释放内存	 
	f_closedir(&mp3dir);
}



//接收文件索引号，返回文件名，调用函数在堆栈段分配的空间退出后会丢失，要求fename已经预先分配好内存
u8 mp3_getFilename(u16 index,u8 *fename)
{
	DIR mp3dir;	 		//目录
	u8 res;
	FILINFO mp3fileinfo;//文件信息
	uint8_t *pname;
	u16 curindex=index;
	
	mp3fileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
	mp3fileinfo.lfname=mymalloc(SRAMIN,mp3fileinfo.lfsize);	//为长文件缓存区分配内存
	 res=f_opendir(&mp3dir,(const TCHAR*)"0:/MUSIC"); //打开目录
	if(res==FR_OK)
	{
		dir_sdi(&mp3dir,index);			//改变当前目录索引	 
		res=f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
		
        if(res!=FR_OK||mp3fileinfo.fname[0]==0)	//错误了/到末尾了,退出
				{
					f_closedir(&mp3dir);
					fename=0;
						return -1;
				}
				else
				{
					f_closedir(&mp3dir);
					pname=(u8 *)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);
					strcpy((char*)fename,"0:/MUSIC/");				//复制路径(目录)
					strcat((char*)fename,(const char*)pname);  			//将文件名接在后面
					myfree(SRAMIN,mp3fileinfo.lfname);	//释放内存
					return 0;
				}
		
	}
	
	
		f_closedir(&mp3dir);
	
	myfree(SRAMIN,mp3fileinfo.lfname);	//释放内存		
return -1;	
}
//_LabelFrameHead label;
//接收文件名，返回名称，作者等信息
void mp3_getSongInfo(uint16_t *filename,char *frameLabel,char *detail)
{
	FIL* fmp3;
	_LabelHead	buffer;//标签头
	_LabelFrameHead label;//标签帧
	uint16_t res;
		long  Size;//lable size
	long lsize;
	char *typ1=NULL;
	int i;
	
	fmp3=(FIL*)mymalloc(SRAMIN,sizeof(FIL));//申请文件描述符内存
	
	res=f_open(fmp3,(const TCHAR*)filename,FA_READ);//打开文件
		if(res!=FR_OK)
			return;
		/*移动到文件开头*/ 
    res = f_lseek(fmp3,0);
		res=f_read(fmp3,&buffer,sizeof(_LabelHead),(UINT*)&br);//读取MP3标签头
		//计算标签头尺寸
		Size=(buffer.Size[0]&0x7F)*0x200000+(buffer.Size[1]&0x7F)*0x400+(buffer.Size[2]&0x7F)*0x80+(buffer.Size[3]&0x7F);
	
		
		/*读取标签帧，提取信息*/
	while((f_tell(fmp3)+10 ) <= Size)
{
	res=f_read(fmp3,&label,sizeof(_LabelFrameHead),(UINT*)&br);
	 lsize = label.Size[0]*0x100000000 + label.Size[1]*0x10000+ label.Size[2]*0x100 +label.Size[3];
	
	 //printf("ID->%s\n",label.ID);
	 if((strncmp(label.ID, frameLabel, 4) == 0))//寻找帧标识
	{
			//detail = (char*)mymalloc(SRAMIN,lsize);//申请标识对应内容空间
		res=f_read(fmp3,detail,lsize,(UINT*)&br);//读取帧标识内容
		break;
	}
	else
	{
		res=f_lseek(fmp3,f_tell(fmp3)+lsize);//定位到下一个标签帧
	}
}



		if((strncmp( frameLabel, "TIT2", 4) == 0))//仅在tit2时进行编码转换
		{
	//此时detail已近指向读取到的标签帧将进行转码操作
	shiftCode((uint8_t *)detail,br,3,0);//将字符串左移3位，去除编码标志和前缀
				//debug ok
			
	UnicodeToGBK((WCHAR *)detail,br-3);//unicode转gbk
		}
		
		
		
	
		//myfree(SRAMIN,typ1);				//释放内存	
		myfree(SRAMIN,fmp3);				//释放内存			
}



/*
int *lognfo;
loginfo=mymalloc(SRAMIN,(info.filenum)*(sizeof(int)));//为系统log文件字节定位数组申请空间
*/

/*将歌曲名列表写入系统文件，并设置字节地址索引*/
void createSongLog(uint8_t *logName,_MP3_INFO *_info,int *loginfo)
{
	//---------------------------------------------------------------------
	int g;
	//
	uint8_t i,num;
	uint16_t t;
	uint8_t *fname;
	FIL* flog;
	uint16_t res;
	uint16_t bytesum=0,tbyte=0;
	char *pdetail,*tname;
	
	flog=(FIL*)mymalloc(SRAMIN,sizeof(FIL));//申请文件描述符内存
	fname=mymalloc(SRAMIN,_MAX_LFN*2+1);	//为长文件缓存区分配内存
	pdetail=(char*)mymalloc(SRAMIN,50);//申请MP3标签内存
	tname=(char*)mymalloc(SRAMIN,50);//申请MP3标签内存
	
	num=_info->filenum;
	delay_ms(200);
	res=f_open(flog,(const TCHAR*)logName,FA_OPEN_ALWAYS|FA_WRITE);//创建系统文件
	if(res!=FR_OK)
	{
		printf("sys>>create logfile err\n");
		return;
	}
	for(i=0;i<num;i++)
	{
		mp3_getFilename(_info->filelist[i],fname);//获取文件名
		mp3_getSongInfo((uint16_t *)fname,"TIT2",pdetail);//读取标签帧内容
		delay_ms(20);
		//*将每一行的起始字符字节地址记录下来，便于快速定位
		loginfo[i]=bytesum;
		printf("f>>%s\n",fname);
		printf(">>>%s\n",pdetail);//调试输出读到的文件名
		//
		//
		delay_ms(100);
		tbyte=f_printf(flog,"%s",pdetail);//写入文件
		f_printf(flog,"%c",'\n');//写入换行符
		
		delay_ms(100);
		//printf("=>%d\n",tbyte);
		bytesum+=tbyte;
		//清空数组
		delay_ms(100);
		for(t=0;t<50;t++)
		{
			pdetail[t]=0x00;
		}
		
	}
	
	f_printf(flog,"%c",0x80);//写入终止符
	f_close(flog);
	//归还空间
	myfree(SRAMIN,fname);
	myfree(SRAMIN,flog);
	myfree(SRAMIN,pdetail);
	myfree(SRAMIN,tname);
	
}


void printIndex(_MP3_INFO *_info)//打印文件索引号
{
	uint8_t i,num;
	num=_info->filenum;
	printf("filenum:%d\n",num);
	delay_ms(200);
	for(i=0;i<num;i++)
	{
		printf("index%d:%d\n",i,_info->filelist[i]);
		delay_ms(200);
	}
}

void printFilList(_MP3_INFO *_info)//打印文件名列表
{
	uint8_t i,num;
	uint8_t *fname;
	fname=mymalloc(SRAMIN,_MAX_LFN*2+1);	//为长文件缓存区分配内存
	num=_info->filenum;
	delay_ms(200);
	for(i=0;i<num;i++)
	{
		mp3_getFilename(_info->filelist[i],fname);
		printf("%s\n",fname);
		delay_ms(200);
		
	}
	myfree(SRAMIN,fname);
}


void mp3_pause(uint8_t *pstate)//暂停播放
{
	(*pstate)^=PLAY_ON;
}

void mp3_continue(uint8_t *pstate)//继续播放
{
	(*pstate)|=PLAY_ON;
}

void mp3_openNew(uint8_t *pname,FIL* pfmp3,uint8_t *pstate)//打开下一曲
{
	uint8_t res;
	count=0;//系统状态复位
	reload=1;
	f_close(pfmp3);//关闭正在播放的文件
	res=f_open(pfmp3,(const TCHAR*)pname,FA_READ);//打开新文件	 
	while(res!=FR_OK)
	{
		printf("open file error\n");
		(*pstate)|=FILEOPEN_ERR;//置位标志位
	}
	printf("opem file ok\n");
	(*pstate)|=PLAY_ON;
}


int DrawchPic(char *fname)
{
	FIL *fp;//全局文件描述
	int res,brt;
	unsigned char  ch;
	fp=(FIL*)mymalloc(SRAMIN,sizeof(FIL));//申请文件描述符内存
		res=f_open(fp,(const TCHAR*)fname,FA_READ);//打开新文件
	if(res!=FR_OK)
	{
		printf("ferr\n");
		myfree(SRAMIN,fp);
		return -1;
	}
	while((	res=f_read(fp,&ch,1,(UINT*)&brt))==FR_OK&&ch!=0x80)//0x80为终止标志，避免读取超出范围
	{
		delay_ms(10);
		printf("%c",ch);
	}
	f_close(fp);
	myfree(SRAMIN,fp);
	return 0;
}

