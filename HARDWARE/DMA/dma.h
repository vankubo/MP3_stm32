#ifndef __DMA_H
#define	__DMA_H	   
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//DMA ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/8
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////
							    					    

void MYDMA_Config(DMA_Channel_TypeDef*DMA_CHx,u32 cpar,u32 cmar,u16 cndtr);//����DMA1_CHx

void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx);//ʹ��DMA1_CHx������һ�δ���
void DMA1_NVICcfg(IRQn_Type DMA1_irq,uint8_t PreemptionPriority,uint8_t SubPriority);
/*
MYDMA_Config(DMA1_Channel4,(u32)&USART1->DR,(u32)SendBuff,24);
	 	USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE); //ʹ�ܴ���1��DMA����
		SPI_I2S_DMACmd(SPI1,SPI_I2S_DMAReq_Tx, ENABLE);
		MYDMA_Enable(DMA1_Channel4);//��ʼһ��DMA���䣡
	if(DMA_GetFlagStatus(DMA1_FLAG_TC4)!=RESET)//�ж�ͨ��4�������*��֪��ʲôԭ��Ŀǰֻ�������ַ�ʽ
				DMA_ClearFlag(DMA1_FLAG_TC4);//���ͨ��4������ɱ�־
*/
#endif




