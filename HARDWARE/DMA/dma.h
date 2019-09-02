#ifndef __DMA_H
#define	__DMA_H	   
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//DMA 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/8
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////
							    					    

void MYDMA_Config(DMA_Channel_TypeDef*DMA_CHx,u32 cpar,u32 cmar,u16 cndtr);//配置DMA1_CHx

void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx);//使能DMA1_CHx来启动一次传输
void DMA1_NVICcfg(IRQn_Type DMA1_irq,uint8_t PreemptionPriority,uint8_t SubPriority);
/*
MYDMA_Config(DMA1_Channel4,(u32)&USART1->DR,(u32)SendBuff,24);
	 	USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE); //使能串口1的DMA发送
		SPI_I2S_DMACmd(SPI1,SPI_I2S_DMAReq_Tx, ENABLE);
		MYDMA_Enable(DMA1_Channel4);//开始一次DMA传输！
	if(DMA_GetFlagStatus(DMA1_FLAG_TC4)!=RESET)//判断通道4传输完成*不知道什么原因，目前只能用这种方式
				DMA_ClearFlag(DMA1_FLAG_TC4);//清除通道4传输完成标志
*/
#endif




