#ifndef __MP3PLAYER_H
#define __MP3PLAYER_H
#include "sys.h"
#include "ff.h"
#include "mp3FileDef.h"
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
 #define DATA_LEN 32
//structs
//MP3�ļ���ϸ��Ϣ
typedef struct{
	u8 *filename;
	u8 *auther;
	u8 *name;
	u8 *album;
} _MP3_DETAIL;


//�ļ�����MP3�ļ�����Ϣ
typedef struct 
{
	int filenum;//�ļ�����
	u16 *filelist;//��ά���飬�ļ�������
		
} _MP3_INFO;

void mp3_play(void);
u16 mp3_get_tnum(u8 *path);
u8 mp3_play_song(u8 *pname);
void mp3_index_show(u16 index,u16 total);
void mp3_msg_show(u32 lenth);

//=================

//functions

u16 mp3_get_tnum(u8 *path);//����Ŀ¼����Ч��mp3���ļ��ĸ���
void mp3_get_flist(u16 totmp3num,_MP3_INFO *_info);//��ȡMP3�ļ�����
void mp3_getSongInfo(uint16_t *filename,char *frameLabel,char *detail);//��ȡ�ļ������ߣ�ר������ϸ��Ϣ
//�����ļ������ţ������ļ�����Ҫ��fename�Ѿ�Ԥ�ȷ�����ڴ�
u8 mp3_getFilename(u16 index,u8 *fename);
void printIndex(_MP3_INFO *_info);//��ӡ�ļ�������
void printFilList(_MP3_INFO *_info);//��ӡ�ļ����б�
void mp3_pause(uint8_t *pstate);//��ͣ����
void mp3_continue(uint8_t *pstate);//��������
void mp3_openNew(uint8_t *pname,FIL* pfmp3,uint8_t *pstate);//����һ��
/*���������б�д��ϵͳ�ļ����������ֽڵ�ַ����*/
void createSongLog(uint8_t *logName,_MP3_INFO *_info,int *loginfo);
int DrawchPic(char *fname);


u8 play_a_song(u8 *filename);
uint8_t dma_play_a_song(uint8_t *mp3name);
uint8_t dma_playsongWithIT(uint8_t *mp3name);
#endif












