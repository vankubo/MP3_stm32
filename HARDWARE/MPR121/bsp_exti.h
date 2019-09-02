 #ifndef __EXTI_H
#define	__EXTI_H


#include "stm32f10x.h"


//引脚定义
#define KEY1_INT_GPIO_PORT         GPIOB
#define KEY1_INT_GPIO_CLK          (RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO)
#define KEY1_INT_GPIO_PIN          GPIO_Pin_1//EXTI用中断口
#define KEY1_INT_EXTI_PORTSOURCE   GPIO_PortSourceGPIOB//EXTI信号源
#define KEY1_INT_EXTI_PINSOURCE    GPIO_PinSource1//EXTI信号源
#define KEY1_INT_EXTI_LINE         EXTI_Line1//EXTI中断线
#define KEY1_INT_EXTI_IRQ          EXTI1_IRQn//NVIC中断源

#define KEY1_IRQHandler            EXTI1_IRQHandler

void EXTI_Key_Config(void);


#endif /* __EXTI_H */
