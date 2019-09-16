#ifndef __MP3PLAYER_H
#define __MP3PLAYER_H
#include "sys.h"
#include "ff.h"
#include "mp3FileDef.h"
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
 #define DATA_LEN 32
//structs
//MP3文件详细信息
typedef struct{
	u8 *filename;
	u8 *auther;
	u8 *name;
	u8 *album;
} _MP3_DETAIL;


//文件夹下MP3文件的信息
typedef struct 
{
	int filenum;//文件总数
	u16 *filelist;//二维数组，文件索引表
		
} _MP3_INFO;

void mp3_play(void);
u16 mp3_get_tnum(u8 *path);
u8 mp3_play_song(u8 *pname);
void mp3_index_show(u16 index,u16 total);
void mp3_msg_show(u32 lenth);

//=================

//functions

u16 mp3_get_tnum(u8 *path);//返回目录下有效（mp3）文件的个数
void mp3_get_flist(u16 totmp3num,_MP3_INFO *_info);//获取MP3文件索引
void mp3_getSongInfo(uint16_t *filename,char *frameLabel,char *detail);//获取文件的作者，专辑等详细信息
//接收文件索引号，返回文件名，要求fename已经预先分配好内存
u8 mp3_getFilename(u16 index,u8 *fename);
void printIndex(_MP3_INFO *_info);//打印文件索引号
void printFilList(_MP3_INFO *_info);//打印文件名列表
void mp3_pause(uint8_t *pstate);//暂停播放
void mp3_continue(uint8_t *pstate);//继续播放
void mp3_openNew(uint8_t *pname,FIL* pfmp3,uint8_t *pstate);//打开下一曲
/*将歌曲名列表写入系统文件，并设置字节地址索引*/
void createSongLog(uint8_t *logName,_MP3_INFO *_info,int *loginfo);
int DrawchPic(char *fname);


u8 play_a_song(u8 *filename);
uint8_t dma_play_a_song(uint8_t *mp3name);
uint8_t dma_playsongWithIT(uint8_t *mp3name);
#endif












