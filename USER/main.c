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
 ALIENTEKս��STM32������ʵ��43
 ���ֲ����� ʵ��
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/
//ϵͳȫ����Դ
u8 *Databuf;//��Ƶ�����
FIL* fmp3;//ȫ���ļ�������
uint16_t count;//�ڴ���ֽ�ƫ����
u8 res;//�ļ���ȡ���
uint16_t brt;//ʵ�ʶ�ȡ�ֽ�
uint8_t reload;//�ڴ��ˢ�±�־
uint8_t *filename;//�ļ���
uint8_t state=0x00;//ϵͳȫ�ֱ�־,offset=0x00
_MP3_INFO info;//ϵͳ�ļ�������
extern char key_pressed;//������־@bsp_exti.c
//================
//ϵͳ����
void play();//����
void doOperate(uint8_t *pindex,uint8_t *pstate,u16 pfnum);//ִ�а�������
//====================
 int main(void)
 {	 
	
	 uint8_t testBuff[32];
	 uint8_t c='A';//������ֵ
	 uint8_t cindex=0;//�ļ��б�������
	 //================
	  u32 SD_total,SD_free;//sd������	   
	u16 fnum=0;//�ļ�����
	 DIR mp3dir;//Ŀ¼
	 //================
	delay_init();	    	 //��ʱ������ʼ��	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200
	USART_Config();//���������ڳ�ʼ��
 	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
	//LCD_Init();			   		//��ʼ��LCD   
	 IIC_Init();
	  
	W25QXX_Init();				//��ʼ��W25Q128
 	VS_Init();	  				//��ʼ��VS1053 
 	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ��
	mpr121QuickConfig();//mpr121��ʼ��
	exfuns_init();	
	//=======================================
	//hmi
	delay_ms(500);	
	CMD("page 1",0,0);
	
	//=====================================
	while(SD_Init())//��ⲻ��SD��
	{
		printf("SD Card Error!\n");
		delay_ms(500);					
		delay_ms(500);
		LED0=!LED0;//DS0��˸
		state|=SD_ERR;//��λ��־λ
	}
	
	exfuns_init();				//Ϊfatfs��ر��������ڴ�  
	 
	  //********************************
	// DMA1_NVICcfg(DMA1_Channel3_IRQn,1,1);//dma�жϷ���
	 Databuf=(u8*)mymalloc(SRAMIN,4096);		//����4096�ֽڵ��ڴ�����
	  fmp3=(FIL*)mymalloc(SRAMIN,sizeof(FIL));//�����ļ��������ڴ�
	filename=mymalloc(SRAMIN,_MAX_LFN*2+1);	//Ϊϵͳ���ļ������������ڴ�
	 while(Databuf==NULL||fmp3==NULL)
	 {
		 state|=MALLOC_ERR;//��λ��־λ
		 LED0=0;
			delay_ms(500);
		 LED0=!LED0;
		 printf("Databuf/fmp3 error\n");
			delay_ms(500);
	 }
	 //********************************
	 while(font_init()) 				//����ֿ�
	{	    
		printf("Font Error!\n");
		delay_ms(200);				  
  
	}
	//**********************************
 	f_mount(fs[0],"0:",1); 		//����SD�� 
 	f_mount(fs[1],"1:",1); 		//����FLASH.    
	 
	while(exf_getfree("0",&SD_total,&SD_free))	//�õ�SD������������ʣ������
	{
		printf("SD Card Fatfs Error!\n");
		delay_ms(200);			  
		delay_ms(200);
		LED0=!LED0;//DS0��˸
	}	
	printf("total=>%dMB\tfree=>%dMB\n",SD_total>>10,SD_free>>10);
	//====================================
	printf("�洢������...");
		printf("Ram Test:0X%04X\r\n",VS_Ram_Test());//��ӡvs1053 RAM���Խ��	0X83FF=ok
	VS_Sine_Test();	 //���Ҳ�����
	//====================================
	
	fnum=mp3_get_tnum("0:/MUSIC");//��ȡ�ļ�����
	mp3_get_flist(fnum,&info);//�õ��ļ������б�
	
	/*
	filename=mp3_getFilename(info.filelist[t]);
	myfree(SRAMIN,filename);
	*/
	printIndex(&info);//��ӡ�ļ�������
	delay_ms(200);
	printFilList(&info);//��ӡ�ļ����б�
	/*
	//====================================
	res=f_typetell("0:/MUSIC/cry.mp3");//�õ��ļ���׺	
  printf("==>%X\n",res);	 			  	 						 
		if(res==0x4c)//�����flac,����patch
		{	
			//VS_Load_Patch((u16*)vs1053b_patch,VS1053B_PATCHLEN);
		}  			
	res=f_open(fmp3,(const TCHAR*)("0:/MUSIC/cry.mp3"),FA_READ);//���ļ�	 
	while(res!=FR_OK)
	{
		printf("open file error\n");
		state|=FILEOPEN_ERR;//��λ��־λ
	}
	//=================================
	*/
	cindex=0;
	mp3_getFilename(info.filelist[cindex],filename);//�õ��ļ���
				printf("play->%s\n",filename);
				delay_ms(200);
				mp3_openNew(filename,fmp3,&state);//����һ��
	//==========================================
	VS_HD_Reset();//vs1053Ӳ����λ
	VS_Soft_Reset();
	vsset.mvol=180;						//Ĭ����������Ϊ200.
	//mp3_vol_show((vsset.mvol-100)/5);	//����������:100~250,��ʾ��ʱ��,���չ�ʽ(vol-100)/5,��ʾ,Ҳ����0~30 
	
		VS_Restart_Play();  					//�������� 
		VS_Set_All(1);        					//������������Ϣ 			 
		VS_Reset_DecodeTime();					//��λ����ʱ�� 
	//=========================================================
	VS_SPI_SpeedHigh();	//����
	//��������ʼ��
	
	reload=1;
	count=0;
	key_pressed=0;
	state=SYSTEM_OFFSET;//��λ��־λ
	
	while(1)
	{
 	 	//delay_ms(200);
		//printf("main run@%X\n",state);
		LED1=1;
		//==================================================
		//���Ų���
			play();//����
		
		//===============================
		//��ʾ����
		LED1=0;
		doOperate(&cindex,&state,fnum);//ִ�а�������
	}
	//�ͷ�ϵͳ�ڴ�
	myfree(SRAMIN,Databuf);	
	myfree(SRAMIN,fmp3);
	myfree(SRAMIN,filename);
}


//==================================
//ϵͳ����

void play()
{
	if(state&PLAY_ON)
	{
		if(reload==1)
			{
				reload=0;
				count=0;
					//printf("reload\n");
					res=f_read(fmp3,Databuf,4096,(UINT*)&brt);//����4096���ֽ�
			}
			
		if(res==FR_OK)//�ļ���ȡ�ɹ�
		{
			if(DREQ==1)
			{
				//printf("t start\n");
				VS_Send_MusicData(Databuf+count);
				count+=DATA_LEN;
			}
			if(count>(4096-DATA_LEN))//����ƫ����
			{
					count=0;
					reload=1;
			}
			
		}
	
		if(brt<4096)//�����ļ�ĩβ�������ļ�ָ��
		{
			res = f_lseek(fmp3,0);
		}
		
	}
}

void doOperate(uint8_t *pindex,uint8_t *pstate,u16 pfnum)//ִ�а�������
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
			case '5'://���ţ���ͣ
			{
				c='A';
				if((*pstate)&PLAY_ON)
				{
					mp3_pause(pstate);//��ͣ����
				}
				else
				{
					mp3_continue(pstate);//��������
				}		
			}break;
			case '4'://��һ��
			{
				c='A';
				(*pindex)++;//�����ļ�������
				if((*pindex)>=pfnum)//�����ŷ�Χ���
					(*pindex)--;
				mp3_getFilename(info.filelist[(*pindex)],filename);//�õ��ļ���
				printf("play->%s\n",filename);
				delay_ms(200);
				mp3_openNew(filename,fmp3,pstate);//����һ��
			}break;
			case '6'://��һ��
			{
				c='A';
				(*pindex)--;//�����ļ�������
				if((*pindex)==0)//�����ŷ�Χ���
					(*pindex)++;
				mp3_getFilename(info.filelist[(*pindex)],filename);//�õ��ļ���
				printf("play->%s\n",filename);
				delay_ms(200);
				mp3_openNew(filename,fmp3,pstate);//����һ��
			}break;
		}
	}
}













