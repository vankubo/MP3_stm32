#include "hmi.h"
char End[]={0xff, 0xff, 0xff};    //命令结束符
void CMD(char *cmd,int value,int enable)
{
	Usart_SendString( DEBUG_USARTx , cmd);//发送命令
	if(enable>0)//发送数据
	{
		Usart_SendByte( DEBUG_USARTx,value);
		
	}
	/* 发送一个字节数据到串口 */
		USART_SendData(DEBUG_USARTx, 0xff);
		
		/* 等待发送完毕 */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);		
	
	/* 发送一个字节数据到串口 */
		USART_SendData(DEBUG_USARTx, 0xff);
		
		/* 等待发送完毕 */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);		
	
	/* 发送一个字节数据到串口 */
		USART_SendData(DEBUG_USARTx, 0xff);
		
		/* 等待发送完毕 */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);		
	
	
}

/*void _CMD(char *cmd,int data,int en)  //向串口屏发送命令和数据,规定en=1为数据有效
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

