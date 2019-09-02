#include "dma.h"
#include "spi.h"
#include "usart.h"
#include "delay.h"
#include "vs10xx.h"
#include "mp3player.h"
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

DMA_InitTypeDef DMA_InitStructure;

u16 DMA1_MEM_LEN;//����DMAÿ�����ݴ��͵ĳ��� 	
//DMA1�ĸ�ͨ������
//����Ĵ�����ʽ�ǹ̶���,���Ҫ���ݲ�ͬ��������޸�
//�Ӵ洢��->����ģʽ/8λ���ݿ��/�洢������ģʽ
//DMA_CHx:DMAͨ��CHx
//cpar:�����ַ
//cmar:�洢����ַ
//cndtr:���ݴ����� 
void MYDMA_Config(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar,u16 cndtr)
{
		
	
 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//ʹ��DMA����
	
  DMA_DeInit(DMA_CHx);   //��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ

	DMA1_MEM_LEN=cndtr;
	DMA_InitStructure.DMA_PeripheralBaseAddr = cpar;  //DMA�������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA�ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  //���ݴ��䷽�򣬴��ڴ��ȡ���͵�����
	DMA_InitStructure.DMA_BufferSize = cndtr;  //DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //����������ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMAͨ�� xӵ�������ȼ� 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(DMA_CHx, &DMA_InitStructure);  //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��USART1_Tx_DMA_Channel����ʶ�ļĴ���
	 
	//������ɽ����ж�
	DMA_ITConfig(DMA_CHx, DMA_IT_TC, ENABLE);

} 

void DMA1_NVICcfg(IRQn_Type DMA1_irq,uint8_t PreemptionPriority,uint8_t SubPriority)
{
	NVIC_InitTypeDef NVIC_InitStructure;
		
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_irq;			//ʹ���ж�ͨ��DMA1_ChannelX_IRQn
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PreemptionPriority;	//��ռ���ȼ��� 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = SubPriority;					//�����ȼ�
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
  	NVIC_Init(&NVIC_InitStructure); 
}

//����һ��DMA����
void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx)
{ 
	DMA_Cmd(DMA_CHx, DISABLE );  //�ر�DMA_CHx DMA1 ��ָʾ��ͨ��      
 	DMA_SetCurrDataCounter(DMA_CHx,DMA1_MEM_LEN);//DMAͨ����DMA����Ĵ�С
 	DMA_Cmd(DMA_CHx, ENABLE);  //ʹ��DMA_CHx DMA1 ��ָʾ��ͨ�� 
}	  

 

void DMA1_Channel3_IRQHandler(void)
{
	printf("d");
	if(DMA_GetITStatus(DMA1_IT_TC3))
    {
							DMA_ClearITPendingBit(DMA1_IT_TC3);
						DMA_ClearITPendingBit(DMA1_IT_HT3);
			if(DMA_GetFlagStatus(DMA1_FLAG_TC3)!=RESET)//�ж�ͨ��4�������*��֪��ʲôԭ��Ŀǰֻ�������ַ�ʽ
			{
				DMA_ClearFlag(DMA1_FLAG_TC3);//���ͨ��4������ɱ�־
			}
					while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);//�ȴ�spi�������ݷ�����ɣ�����XDCS��������������ݶ�ʧ
			//delay_ms(5);
					VS_XDCS=1;
		}
}























