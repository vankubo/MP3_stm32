/*
系统全局标志
*/

#ifndef __SYSTEM_H
#define __SYSTEM_H
#define SYSTEM_OFFSET 0x00//系统复位状态
#define PLAY_ON 0x01//正在播放
#define FILEOPEN_ERR 0x02//文件打开错误
#define SD_ERR 0x04//sd卡错误
#define FS_ERR 0x8//文件系统错误
#define VS_ERR 0x10//vs1053硬件错误
#define MALLOC_ERR 0x20//内存申请错误
#define SYS_RELOAD 0X40//刷新内存池
#endif

