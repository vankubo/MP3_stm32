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
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������V3
//MP3�������� ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2015/1/20
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved								  						    								  
//////////////////////////////////////////////////////////////////////////////////
//============================
//ϵͳȫ����Դ
extern  u8 *Databuf;//��Ƶ�����@main.c
extern  FIL* fmp3;//ȫ���ļ�������@main.c
extern uint16_t count;//�ڴ���ֽ�ƫ����@main.c
extern u8 res;//�ļ���ȡ���@main.c
extern uint16_t brt;//ʵ�ʶ�ȡ�ֽ�@main.c
extern uint8_t reload;//�ڴ��ˢ�±�־@main.c
extern uint8_t *filename;//�ļ���@main.c
extern uint8_t state;//ϵͳȫ�ֱ�־,offset=0x00@main.c
extern char key_pressed;//������־@bsp_exti.c
//================
u16 f_kbps=0;//�����ļ�λ��	

//��ʾ��ǰ����
void mp3_vol_show(u8 vol)
{			    
	//LCD_ShowString(30+110,230,200,16,16,"VOL:");	  	  
	//LCD_ShowxNum(30+142,230,vol,2,16,0X80); 	//��ʾ����	 
}

//��ʾ����ʱ��,������ ��Ϣ 
//lenth:�����ܳ���
void mp3_msg_show(u32 lenth)
{	
	static u16 playtime=0;//����ʱ����	     
 	u16 time=0;// ʱ�����
	u16 temp=0;	  
	if(f_kbps==0xffff)//δ���¹�
	{
		playtime=0;
		f_kbps=VS_Get_HeadInfo();	   //��ñ�����
	}	 	 
	time=VS_Get_DecodeTime(); //�õ�����ʱ��
	if(playtime==0)playtime=time;
	else if((time!=playtime)&&(time!=0))//1sʱ�䵽,������ʾ����
	{
		playtime=time;//����ʱ�� 	 				    
		temp=VS_Get_HeadInfo(); //��ñ�����	   				 
		if(temp!=f_kbps)
		{
			f_kbps=temp;//����KBPS	  				     
		}			 
		//��ʾ����ʱ��			 
		//LCD_ShowxNum(30,210,time/60,2,16,0X80);		//����
	//	LCD_ShowChar(30+16,210,':',16,0);
		//LCD_ShowxNum(30+24,210,time%60,2,16,0X80);	//����		
 	//	LCD_ShowChar(30+40,210,'/',16,0); 	    	 
		//��ʾ��ʱ��
		if(f_kbps)time=(lenth/f_kbps)/125;//�õ�������   (�ļ�����(�ֽ�)/(1000/8)/������=����������    	  
		else time=0;//�Ƿ�λ��	  
 	//	LCD_ShowxNum(30+48,210,time/60,2,16,0X80);	//����
		//LCD_ShowChar(30+64,210,':',16,0);
		//LCD_ShowxNum(30+72,210,time%60,2,16,0X80);	//����	  		    
		//��ʾλ��			   
   	//	LCD_ShowxNum(30+110,210,f_kbps,3,16,0X80); 	//��ʾλ��	 
	//	LCD_ShowString(30+134,210,200,16,16,"Kbps");	  	  
		LED0=!LED0;		//DS0��ת
	}   		 
}			  



//����ָ������
u8 play_a_song(u8 *filename)
{
		FIL* fmp3;
    u16 br;
	u8 res,rval;	  
	u8 *databuf;	   		   
	u16 i=0; 
	u8 key;  
	
	//###################################
	VS_HD_Reset();//vs1053Ӳ����λ
	VS_Soft_Reset();
	vsset.mvol=160;						//Ĭ����������Ϊ200.
	mp3_vol_show((vsset.mvol-100)/5);	//����������:100~250,��ʾ��ʱ��,���չ�ʽ(vol-100)/5,��ʾ,Ҳ����0~30 
  //########################################
	
		    
	fmp3=(FIL*)mymalloc(SRAMIN,sizeof(FIL));//�����ļ��������ڴ�
	databuf=(u8*)mymalloc(SRAMIN,4096);		//����4096�ֽڵ��ڴ�����
	if(databuf==NULL||fmp3==NULL)rval=0XFF ;//�ڴ�����ʧ��.
	
	rval=0;
	if(rval==0)
	{	  
	  	VS_Restart_Play();  					//�������� 
		VS_Set_All(0);        					//������������Ϣ 			 
		VS_Reset_DecodeTime();					//��λ����ʱ�� 
	  
		res=f_typetell(filename);	 	 			//�õ��ļ���׺	 			  	 						 
		if(res==0x4c)//�����flac,����patch
		{	
			VS_Load_Patch((u16*)vs1053b_patch,VS1053B_PATCHLEN);
		}  				 		   		
		
		res=f_open(fmp3,(const TCHAR*)filename,FA_READ);//���ļ�	 
 		if(res==0)//�򿪳ɹ�.
		{ 
			VS_SPI_SpeedHigh();	//����						   
			while(rval==0)
			{
				res=f_read(fmp3,databuf,4096,(UINT*)&br);//����4096���ֽ� 
				if(res!=FR_OK)//��ȡ�ļ�����
				{
					rval=0;
					break;
				}
				i=0;
				do//������ѭ��VS_Send_MusicData(databuf+i)
			    {  	
					if(VS_Send_MusicData(databuf+i)==0)//��VS10XX������Ƶ����
					{
						i+=32;
					}
				}while(i<br);//ѭ������br���ֽ� 
					
				
				if(br!=4096)//��ȡ�ֽ�������4096�ֽڣ���ʾ�ļ��Ѷ���
				{
					rval=0;
					break;//������.		  
				} 		
			}
			f_close(fmp3);
			
		}else rval=0XFF;//���ִ���	   	  
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
	VS_HD_Reset();//vs1053Ӳ����λ
	VS_Soft_Reset();
	vsset.mvol=220;						//Ĭ����������Ϊ200.
	//mp3_vol_show((vsset.mvol-100)/5);	//����������:100~250,��ʾ��ʱ��,���չ�ʽ(vol-100)/5,��ʾ,Ҳ����0~30 
  //########################################
	
		    
	fmp3=(FIL*)mymalloc(SRAMIN,sizeof(FIL));//�����ļ��������ڴ�
	
	if(Databuf==NULL||fmp3==NULL)rval=0XFF ;//�ڴ�����ʧ��.
	
	rval=0;
	if(rval==0)
	{	  
	  	VS_Restart_Play();  					//�������� 
		VS_Set_All(1);        					//������������Ϣ 			 
		VS_Reset_DecodeTime();					//��λ����ʱ�� 
	  
		res=f_typetell(mp3name);	 	 			//�õ��ļ���׺	 			  	 						 
		if(res==0x4c)//�����flac,����patch
		{	
			VS_Load_Patch((u16*)vs1053b_patch,VS1053B_PATCHLEN);
		}  				 		   		
		
		res=f_open(fmp3,(const TCHAR*)mp3name,FA_READ);//���ļ�	 
 		if(res==0)//�򿪳ɹ�.
		{ 
			printf("open file success\n");
			VS_SPI_SpeedHigh();	//����						   
			//==============
			
			
			
			reload=1;
			start=1;
			do{
				if(reload==1)
				{
					printf("reload\n");
					res=f_read(fmp3,Databuf,4096,(UINT*)&brt);//����4096���ֽ� 
					reload=0;
				}
				if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13)==1)
				{
					//start=0;
					//printf("send%d\n",i);
					 //MYDMA_Config(DMA1_Channel3,(u32)&SPI1->DR,(u32)Datdbuf,32);
	 //DMA1_NVICcfg(DMA1_Channel3_IRQn,1,1);//dma�жϷ���
	 //�Ƚ���һ����Ч���䣬ʹdma���빤��״̬��
					MYDMA_Config(DMA1_Channel3,(u32)&SPI1->DR,(u32)(Databuf+i),DATA_LEN);
					VS_XDCS=0;
	 SPI_I2S_DMACmd(SPI1,SPI_I2S_DMAReq_Tx, ENABLE);
		MYDMA_Enable(DMA1_Channel3);//��ʼһ��DMA����
					//EXTIX_Init();
					
					i+=DATA_LEN;
			while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);//�ȴ�spi�������ݷ�����ɣ�����XDCS��������������ݶ�ʧ
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
			
		}else rval=0XFF;//���ִ���	   	  
	}						
	  	 		  	    
	myfree(SRAMIN,fmp3);
	return rval;	  	 		  	    
}

*/

//�õ�path·����,Ŀ���ļ����ܸ���
//path:·��		    
//����ֵ:����Ч�ļ���
u16 mp3_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//��ʱĿ¼
	FILINFO tfileinfo;	//��ʱ�ļ���Ϣ		
	u8 *fn; 			 			   			     
    res=f_opendir(&tdir,(const TCHAR*)path); //��Ŀ¼
  	tfileinfo.lfsize=_MAX_LFN*2+1;						//���ļ�����󳤶�
	tfileinfo.lfname=mymalloc(SRAMIN,tfileinfo.lfsize);	//Ϊ���ļ������������ڴ�
	if(res==FR_OK&&tfileinfo.lfname!=NULL)
	{
		while(1)//��ѯ�ܵ���Ч�ļ���
		{
	        res=f_readdir(&tdir,&tfileinfo);       		//��ȡĿ¼�µ�һ���ļ�
	        if(res!=FR_OK||tfileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�		  
     		fn=(u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X40)//ȡ����λ,�����ǲ��������ļ�	
			{
				rval++;//��Ч�ļ�������1
			}	    
		}  
	} 
	myfree(SRAMIN,tfileinfo.lfname);
	return rval;
}


void mp3_get_flist(u16 totmp3num,_MP3_INFO *_info)//��ȡMP3�ļ�����
{
	
	u8 res;
 	DIR mp3dir;	 		//Ŀ¼
	FILINFO mp3fileinfo;//�ļ���Ϣ
	u8 *fn;   			//���ļ���
	u8 *pname;			//��·�����ļ���
	u16 curindex;		//ͼƬ��ǰ����
 	u16 temp;
	u16 *mp3indextbl;	//���������� 
	//_MP3_INFO *filelist;
	
	_info->filenum=totmp3num;
					   
  mp3fileinfo.lfsize=_MAX_LFN*2+1;						//���ļ�����󳤶�
	mp3fileinfo.lfname=mymalloc(SRAMIN,mp3fileinfo.lfsize);	//Ϊ���ļ������������ڴ�
 	pname=mymalloc(SRAMIN,mp3fileinfo.lfsize);				//Ϊ��·�����ļ��������ڴ�
 	mp3indextbl=mymalloc(SRAMIN,2*totmp3num);				//����2*totmp3num���ֽڵ��ڴ�,���ڴ�������ļ�����
	//��¼����
    res=f_opendir(&mp3dir,"0:/MUSIC"); //��Ŀ¼
	if(res==FR_OK)
	{
		curindex=0;//��ǰ����Ϊ0
		while(1)//ȫ����ѯһ��
		{
			temp=mp3dir.index;								//��¼��ǰindex
	        res=f_readdir(&mp3dir,&mp3fileinfo);       		//��ȡĿ¼�µ�һ���ļ�
	        if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�		  
     		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);				
			res=f_typetell(fn);	
			if((res&0XF0)==0X40)//ȡ����λ,�����ǲ��������ļ�	
			{
				mp3indextbl[curindex]=temp;//��¼����
				curindex++;
			}	    
		} 
	}   
	_info->filelist=mp3indextbl;
	
	/*
   	curindex=0;											//��0��ʼ��ʾ
   	res=f_opendir(&mp3dir,(const TCHAR*)"0:/MUSIC"); 	//��Ŀ¼
	while(res==FR_OK)//�򿪳ɹ�
	{	
		dir_sdi(&mp3dir,mp3indextbl[curindex]);			//�ı䵱ǰĿ¼����
		LCD_ShowNum(30,500,mp3indextbl[curindex],5,16);		
        res=f_readdir(&mp3dir,&mp3fileinfo);       		//��ȡĿ¼�µ�һ���ļ�
        if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�
     	fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);			 
	
		strcpy((char*)pname,"0:/MUSIC/");				//����·��(Ŀ¼)
		strcat((char*)pname,(const char*)fn);  			//���ļ������ں���
		//����ռ�
 	LCD_Fill(30,50,240,50+16,WHITE);				//���֮ǰ����ʾ
	Show_Str(30,50,240-30,16,fn,16,0);				//��ʾ�������� 
			
		delay_ms(800);	
		curindex++;
	}
	*/
	myfree(SRAMIN,mp3fileinfo.lfname);	//�ͷ��ڴ�			    
	myfree(SRAMIN,pname);				//�ͷ��ڴ�			   
	//myfree(SRAMIN,mp3indextbl);			//�ͷ��ڴ�	 
	f_closedir(&mp3dir);
}



//�����ļ������ţ������ļ��������ú����ڶ�ջ�η���Ŀռ��˳���ᶪʧ��Ҫ��fename�Ѿ�Ԥ�ȷ�����ڴ�
u8 mp3_getFilename(u16 index,u8 *fename)
{
	DIR mp3dir;	 		//Ŀ¼
	u8 res;
	FILINFO mp3fileinfo;//�ļ���Ϣ
	uint8_t *pname;
	u16 curindex=index;
	
	mp3fileinfo.lfsize=_MAX_LFN*2+1;						//���ļ�����󳤶�
	mp3fileinfo.lfname=mymalloc(SRAMIN,mp3fileinfo.lfsize);	//Ϊ���ļ������������ڴ�
	 res=f_opendir(&mp3dir,(const TCHAR*)"0:/MUSIC"); //��Ŀ¼
	if(res==FR_OK)
	{
		dir_sdi(&mp3dir,index);			//�ı䵱ǰĿ¼����	 
		res=f_readdir(&mp3dir,&mp3fileinfo);       		//��ȡĿ¼�µ�һ���ļ�
		
        if(res!=FR_OK||mp3fileinfo.fname[0]==0)	//������/��ĩβ��,�˳�
				{
					f_closedir(&mp3dir);
					fename=0;
						return -1;
				}
				else
				{
					f_closedir(&mp3dir);
					pname=(u8 *)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);
					strcpy((char*)fename,"0:/MUSIC/");				//����·��(Ŀ¼)
					strcat((char*)fename,(const char*)pname);  			//���ļ������ں���
					myfree(SRAMIN,mp3fileinfo.lfname);	//�ͷ��ڴ�
					return 0;
				}
		
	}
	
	
		f_closedir(&mp3dir);
	
	myfree(SRAMIN,mp3fileinfo.lfname);	//�ͷ��ڴ�		
return -1;	
}
//_LabelFrameHead label;
//�����ļ������������ƣ����ߵ���Ϣ
void mp3_getSongInfo(uint16_t *filename,char *frameLabel,char *detail)
{
	FIL* fmp3;
	_LabelHead	buffer;//��ǩͷ
	_LabelFrameHead label;//��ǩ֡
	uint16_t res;
		long  Size;//lable size
	long lsize;
	char *typ1=NULL;
	int i;
	
	fmp3=(FIL*)mymalloc(SRAMIN,sizeof(FIL));//�����ļ��������ڴ�
	
	res=f_open(fmp3,(const TCHAR*)filename,FA_READ);//���ļ�
		if(res!=FR_OK)
			return;
		/*�ƶ����ļ���ͷ*/ 
    res = f_lseek(fmp3,0);
		res=f_read(fmp3,&buffer,sizeof(_LabelHead),(UINT*)&br);//��ȡMP3��ǩͷ
		//�����ǩͷ�ߴ�
		Size=(buffer.Size[0]&0x7F)*0x200000+(buffer.Size[1]&0x7F)*0x400+(buffer.Size[2]&0x7F)*0x80+(buffer.Size[3]&0x7F);
	
		
		/*��ȡ��ǩ֡����ȡ��Ϣ*/
	while((f_tell(fmp3)+10 ) <= Size)
{
	res=f_read(fmp3,&label,sizeof(_LabelFrameHead),(UINT*)&br);
	 lsize = label.Size[0]*0x100000000 + label.Size[1]*0x10000+ label.Size[2]*0x100 +label.Size[3];
	
	 //printf("ID->%s\n",label.ID);
	 if((strncmp(label.ID, frameLabel, 4) == 0))//Ѱ��֡��ʶ
	{
			//detail = (char*)mymalloc(SRAMIN,lsize);//�����ʶ��Ӧ���ݿռ�
		res=f_read(fmp3,detail,lsize,(UINT*)&br);//��ȡ֡��ʶ����
		break;
	}
	else
	{
		res=f_lseek(fmp3,f_tell(fmp3)+lsize);//��λ����һ����ǩ֡
	}
}



		if((strncmp( frameLabel, "TIT2", 4) == 0))//����tit2ʱ���б���ת��
		{
	//��ʱdetail�ѽ�ָ���ȡ���ı�ǩ֡������ת�����
	shiftCode((uint8_t *)detail,br,3,0);//���ַ�������3λ��ȥ�������־��ǰ׺
				//debug ok
			
	UnicodeToGBK((WCHAR *)detail,br-3);//unicodeתgbk
		}
		
		
		
	
		//myfree(SRAMIN,typ1);				//�ͷ��ڴ�	
		myfree(SRAMIN,fmp3);				//�ͷ��ڴ�			
}



/*
int *lognfo;
loginfo=mymalloc(SRAMIN,(info.filenum)*(sizeof(int)));//Ϊϵͳlog�ļ��ֽڶ�λ��������ռ�
*/

/*���������б�д��ϵͳ�ļ����������ֽڵ�ַ����*/
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
	
	flog=(FIL*)mymalloc(SRAMIN,sizeof(FIL));//�����ļ��������ڴ�
	fname=mymalloc(SRAMIN,_MAX_LFN*2+1);	//Ϊ���ļ������������ڴ�
	pdetail=(char*)mymalloc(SRAMIN,50);//����MP3��ǩ�ڴ�
	tname=(char*)mymalloc(SRAMIN,50);//����MP3��ǩ�ڴ�
	
	num=_info->filenum;
	delay_ms(200);
	res=f_open(flog,(const TCHAR*)logName,FA_OPEN_ALWAYS|FA_WRITE);//����ϵͳ�ļ�
	if(res!=FR_OK)
	{
		printf("sys>>create logfile err\n");
		return;
	}
	for(i=0;i<num;i++)
	{
		mp3_getFilename(_info->filelist[i],fname);//��ȡ�ļ���
		mp3_getSongInfo((uint16_t *)fname,"TIT2",pdetail);//��ȡ��ǩ֡����
		delay_ms(20);
		//*��ÿһ�е���ʼ�ַ��ֽڵ�ַ��¼���������ڿ��ٶ�λ
		loginfo[i]=bytesum;
		printf("f>>%s\n",fname);
		printf(">>>%s\n",pdetail);//��������������ļ���
		//
		//
		delay_ms(100);
		tbyte=f_printf(flog,"%s",pdetail);//д���ļ�
		f_printf(flog,"%c",'\n');//д�뻻�з�
		
		delay_ms(100);
		//printf("=>%d\n",tbyte);
		bytesum+=tbyte;
		//�������
		delay_ms(100);
		for(t=0;t<50;t++)
		{
			pdetail[t]=0x00;
		}
		
	}
	
	f_printf(flog,"%c",0x80);//д����ֹ��
	f_close(flog);
	//�黹�ռ�
	myfree(SRAMIN,fname);
	myfree(SRAMIN,flog);
	myfree(SRAMIN,pdetail);
	myfree(SRAMIN,tname);
	
}


void printIndex(_MP3_INFO *_info)//��ӡ�ļ�������
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

void printFilList(_MP3_INFO *_info)//��ӡ�ļ����б�
{
	uint8_t i,num;
	uint8_t *fname;
	fname=mymalloc(SRAMIN,_MAX_LFN*2+1);	//Ϊ���ļ������������ڴ�
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


void mp3_pause(uint8_t *pstate)//��ͣ����
{
	(*pstate)^=PLAY_ON;
}

void mp3_continue(uint8_t *pstate)//��������
{
	(*pstate)|=PLAY_ON;
}

void mp3_openNew(uint8_t *pname,FIL* pfmp3,uint8_t *pstate)//����һ��
{
	uint8_t res;
	count=0;//ϵͳ״̬��λ
	reload=1;
	f_close(pfmp3);//�ر����ڲ��ŵ��ļ�
	res=f_open(pfmp3,(const TCHAR*)pname,FA_READ);//�����ļ�	 
	while(res!=FR_OK)
	{
		printf("open file error\n");
		(*pstate)|=FILEOPEN_ERR;//��λ��־λ
	}
	printf("opem file ok\n");
	(*pstate)|=PLAY_ON;
}


int DrawchPic(char *fname)
{
	FIL *fp;//ȫ���ļ�����
	int res,brt;
	unsigned char  ch;
	fp=(FIL*)mymalloc(SRAMIN,sizeof(FIL));//�����ļ��������ڴ�
		res=f_open(fp,(const TCHAR*)fname,FA_READ);//�����ļ�
	if(res!=FR_OK)
	{
		printf("ferr\n");
		myfree(SRAMIN,fp);
		return -1;
	}
	while((	res=f_read(fp,&ch,1,(UINT*)&brt))==FR_OK&&ch!=0x80)//0x80Ϊ��ֹ��־�������ȡ������Χ
	{
		delay_ms(10);
		printf("%c",ch);
	}
	f_close(fp);
	myfree(SRAMIN,fp);
	return 0;
}

