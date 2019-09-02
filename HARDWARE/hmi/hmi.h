#ifndef __HMI_H
#define __HMI_H
#include "stm32f10x.h"
#include "bsp_usart.h"
void CMD(char *cmd,int value,int enable);
void _CMD(char *cmd,int data,int en);
#endif


