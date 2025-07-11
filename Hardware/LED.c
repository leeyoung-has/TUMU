#include "LED.h"
#include "stm32f4xx.h"

void LED_Init(void)//LED0��ʼ��
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_8);
	
}

void LED0_ON(void)//LED0��
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_8);
}

void LED0_OFF(void)
{
  GPIO_SetBits(GPIOB,GPIO_Pin_8);
}


