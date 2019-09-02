#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "led.h" 		 	       
#include "usmart.h" 
#include "malloc.h"
#include "sdio_sdcard.h"  
#include "w25qxx.h"    
#include "ff.h"  
#include "exfuns.h"   
#include "vs10xx.h"
#include "mp3player.h"	
 #include "dma.h"
 #include "bsp_exti.h"
 #include "mpr121.h"
 #include "myiic.h"
 #include "system.h"
 #include "fontupd.h"
 #include "bsp_usart.h"
 #include "hmi.h"
/************************************************
 ALIENTEK战舰STM32开发板实验43
 音乐播放器 实验
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/
//系统全局资源
u8 *Databuf;//音频缓冲池
FIL* fmp3;//全局文件描述符
uint16_t count;//内存池字节偏移量
u8 res;//文件读取结果
uint16_t brt;//实际读取字节
uint8_t reload;//内存池刷新标志
uint8_t *filename;//文件名
uint8_t state=0x00;//系统全局标志,offset=0x00
_MP3_INFO info;//系统文件索引表
extern char key_pressed;//按键标志@bsp_exti.c
//================
//系统函数
void play();//播放
void doOperate(uint8_t *pindex,uint8_t *pstate,u16 pfnum);//执行按键操作
//====================
 int main(void)
 {	 
	
	 uint8_t testBuff[32];
	 uint8_t c='A';//按键键值
	 uint8_t cindex=0;//文件列表索引号
	 //================
	  u32 SD_total,SD_free;//sd卡容量	   
	u16 fnum=0;//文件数量
	 DIR mp3dir;//目录
	 //================
	delay_init();	    	 //延时函数初始化	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);	 	//串口初始化为115200
	USART_Config();//串口屏串口初始化
 	LED_Init();		  			//初始化与LED连接的硬件接口
	//LCD_Init();			   		//初始化LCD   
	 IIC_Init();
	  
	W25QXX_Init();				//初始化W25Q128
 	VS_Init();	  				//初始化VS1053 
 	my_mem_init(SRAMIN);		//初始化内部内存池
	mpr121QuickConfig();//mpr121初始化
	exfuns_init();	
	//=======================================
	//hmi
	delay_ms(500);	
	CMD("page 1",0,0);
	
	//=====================================
	while(SD_Init())//检测不到SD卡
	{
		printf("SD Card Error!\n");
		delay_ms(500);					
		delay_ms(500);
		LED0=!LED0;//DS0闪烁
		state|=SD_ERR;//置位标志位
	}
	
	exfuns_init();				//为fatfs相关变量申请内存  
	 
	  //********************************
	// DMA1_NVICcfg(DMA1_Channel3_IRQn,1,1);//dma中断分组
	 Databuf=(u8*)mymalloc(SRAMIN,4096);		//开辟4096字节的内存区域
	  fmp3=(FIL*)mymalloc(SRAMIN,sizeof(FIL));//申请文件描述符内存
	filename=mymalloc(SRAMIN,_MAX_LFN*2+1);	//为系统长文件缓存区分配内存
	 while(Databuf==NULL||fmp3==NULL)
	 {
		 state|=MALLOC_ERR;//置位标志位
		 LED0=0;
			delay_ms(500);
		 LED0=!LED0;
		 printf("Databuf/fmp3 error\n");
			delay_ms(500);
	 }
	 //********************************
	 while(font_init()) 				//检查字库
	{	    
		printf("Font Error!\n");
		delay_ms(200);				  
  
	}
	//**********************************
 	f_mount(fs[0],"0:",1); 		//挂载SD卡 
 	f_mount(fs[1],"1:",1); 		//挂载FLASH.    
	 
	while(exf_getfree("0",&SD_total,&SD_free))	//得到SD卡的总容量和剩余容量
	{
		printf("SD Card Fatfs Error!\n");
		delay_ms(200);			  
		delay_ms(200);
		LED0=!LED0;//DS0闪烁
	}	
	printf("total=>%dMB\tfree=>%dMB\n",SD_total>>10,SD_free>>10);
	//====================================
	printf("存储器测试...");
		printf("Ram Test:0X%04X\r\n",VS_Ram_Test());//打印vs1053 RAM测试结果	0X83FF=ok
	VS_Sine_Test();	 //正弦波测试
	//====================================
	
	fnum=mp3_get_tnum("0:/MUSIC");//获取文件总数
	mp3_get_flist(fnum,&info);//得到文件索引列表
	
	/*
	filename=mp3_getFilename(info.filelist[t]);
	myfree(SRAMIN,filename);
	*/
	printIndex(&info);//打印文件索引号
	delay_ms(200);
	printFilList(&info);//打印文件名列表
	/*
	//====================================
	res=f_typetell("0:/MUSIC/cry.mp3");//得到文件后缀	
  printf("==>%X\n",res);	 			  	 						 
		if(res==0x4c)//如果是flac,加载patch
		{	
			//VS_Load_Patch((u16*)vs1053b_patch,VS1053B_PATCHLEN);
		}  			
	res=f_open(fmp3,(const TCHAR*)("0:/MUSIC/cry.mp3"),FA_READ);//打开文件	 
	while(res!=FR_OK)
	{
		printf("open file error\n");
		state|=FILEOPEN_ERR;//置位标志位
	}
	//=================================
	*/
	cindex=0;
	mp3_getFilename(info.filelist[cindex],filename);//得到文件名
				printf("play->%s\n",filename);
				delay_ms(200);
				mp3_openNew(filename,fmp3,&state);//打开下一曲
	//==========================================
	VS_HD_Reset();//vs1053硬件复位
	VS_Soft_Reset();
	vsset.mvol=180;						//默认设置音量为200.
	//mp3_vol_show((vsset.mvol-100)/5);	//音量限制在:100~250,显示的时候,按照公式(vol-100)/5,显示,也就是0~30 
	
		VS_Restart_Play();  					//重启播放 
		VS_Set_All(1);        					//设置音量等信息 			 
		VS_Reset_DecodeTime();					//复位解码时间 
	//=========================================================
	VS_SPI_SpeedHigh();	//高速
	//开关量初始化
	
	reload=1;
	count=0;
	key_pressed=0;
	state=SYSTEM_OFFSET;//置位标志位
	
	while(1)
	{
 	 	//delay_ms(200);
		//printf("main run@%X\n",state);
		LED1=1;
		//==================================================
		//播放部分
			play();//播放
		
		//===============================
		//显示部分
		LED1=0;
		doOperate(&cindex,&state,fnum);//执行按键操作
	}
	//释放系统内存
	myfree(SRAMIN,Databuf);	
	myfree(SRAMIN,fmp3);
	myfree(SRAMIN,filename);
}


//==================================
//系统函数

void play()
{
	if(state&PLAY_ON)
	{
		if(reload==1)
			{
				reload=0;
				count=0;
					//printf("reload\n");
					res=f_read(fmp3,Databuf,4096,(UINT*)&brt);//读出4096个字节
			}
			
		if(res==FR_OK)//文件读取成功
		{
			if(DREQ==1)
			{
				//printf("t start\n");
				VS_Send_MusicData(Databuf+count);
				count+=DATA_LEN;
			}
			if(count>(4096-DATA_LEN))//重置偏移量
			{
					count=0;
					reload=1;
			}
			
		}
	
		if(brt<4096)//读到文件末尾，重置文件指针
		{
			res = f_lseek(fmp3,0);
		}
		
	}
}

void doOperate(uint8_t *pindex,uint8_t *pstate,u16 pfnum)//执行按键操作
{
	char c='A';
	if(key_pressed==1)
		{
			
			c=getPhoneNumber();
			printf("%d@key=%c\n",key_pressed,c);
			key_pressed=0;
		}
	if(c!='A')
	{
		switch(c)
		{
			case '5'://播放，暂停
			{
				c='A';
				if((*pstate)&PLAY_ON)
				{
					mp3_pause(pstate);//暂停播放
				}
				else
				{
					mp3_continue(pstate);//继续播放
				}		
			}break;
			case '4'://下一曲
			{
				c='A';
				(*pindex)++;//更改文件索引号
				if((*pindex)>=pfnum)//索引号范围检查
					(*pindex)--;
				mp3_getFilename(info.filelist[(*pindex)],filename);//得到文件名
				printf("play->%s\n",filename);
				delay_ms(200);
				mp3_openNew(filename,fmp3,pstate);//打开下一曲
			}break;
			case '6'://上一曲
			{
				c='A';
				(*pindex)--;//更改文件索引号
				if((*pindex)==0)//索引号范围检查
					(*pindex)++;
				mp3_getFilename(info.filelist[(*pindex)],filename);//得到文件名
				printf("play->%s\n",filename);
				delay_ms(200);
				mp3_openNew(filename,fmp3,pstate);//打开下一曲
			}break;
		}
	}
}













