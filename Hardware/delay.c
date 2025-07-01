
#include "delay.h"





/**
   @brief  初始化TIM4定时器,配置为1us计数精度
  */
void TIM4_Delay_Init(void) {
    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    
    // 使能TIM4时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    
    // 配置TIM4时钟
    TIM_InitStruct.TIM_Prescaler = 26;        
    TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_InitStruct.TIM_Period = 0xFFFF;   
    TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM4, &TIM_InitStruct);
    
    // 启动TIM4计数器
    TIM_Cmd(TIM4, ENABLE);
}

/**
   @brief  微秒式延时
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
