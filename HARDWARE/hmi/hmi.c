#include "hmi.h"
char End[]={0xff, 0xff, 0xff};    //���������
void CMD(char *cmd,int value,int enable)
{
	Usart_SendString( DEBUG_USARTx , cmd);//��������
	if(enable>0)//��������
	{
		Usart_SendByte( DEBUG_USARTx,value);
		
	}
	/* ����һ���ֽ����ݵ����� */
		USART_SendData(DEBUG_USARTx, 0xff);
		
		/* �ȴ�������� */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);		
	
	/* ����һ���ֽ����ݵ����� */
		USART_SendData(DEBUG_USARTx, 0xff);
		
		/* �ȴ�������� */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);		
	
	/* ����һ���ֽ����ݵ����� */
		USART_SendData(DEBUG_USARTx, 0xff);
		
		/* �ȴ�������� */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);		
	
	
}

/*void _CMD(char *cmd,int data,int en)  //�򴮿����������������,�涨en=1Ϊ������Ч
{
  int i;
 
   printf("%s",cmd);
  if(en)
    printf("%d",data);
    
  i=0;
  while(i<=2)
  {
    printf("%c",End[i]);
    i++;
  }
  
  }*/

