
#include "delay.h"





/**
   @brief  ��ʼ��TIM4��ʱ��,����Ϊ1us��������
  */
void TIM4_Delay_Init(void) {
    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    
    // ʹ��TIM4ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    
    // ����TIM4ʱ��
    TIM_InitStruct.TIM_Prescaler = 26;        
    TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_InitStruct.TIM_Period = 0xFFFF;   
    TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM4, &TIM_InitStruct);
    
    // ����TIM4������
    TIM_Cmd(TIM4, ENABLE);
}

/**
   @brief  ΢��ʽ��ʱ
  */
void delay_us(uint32_t us) {
    uint32_t start = TIM_GetCounter(TIM4);
    uint32_t elapsed = 0;
    
    while (elapsed < us) {
        uint32_t current = TIM_GetCounter(TIM4);
        if (current >= start) {
            elapsed += (current - start);
        } else {
            elapsed += (0x10000 - start + current);
        }
        start = current;
    }
}


void delay_ms(uint32_t ms) {
    while (ms--) {
        delay_us(1000);     
    }
}
