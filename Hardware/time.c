#include "time.h"
#include "usart.h"
#include "delay.h"
#include "stdio.h"
#include "stm32f4xx_rtc.h"
#include "stm32f4xx_pwr.h"
#include "stm32f4xx_rcc.h"
#include "Frq.h"
#define RTC_BKP_FLAG    RTC_BKP_DR0


/**
 * @brief 将DateTime结构体格式化为ISO8601标准字符串
 * @param dt 指向DateTime结构体的指针，包含日期时间信息
 * @param buf 输出缓冲区（至少20字节空间）
 */
void DateTime_ToString(const DateTime* dt, char* buf) {
    // 使用snprintf进行安全格式化
    snprintf(buf, 20, "%04d-%02d-%02d %02d:%02d:%02d",
             2000 + dt->year,  // RTC年份0-99对应2000-2099
             dt->month,        // 月份范围1-12
             dt->day,          // 日期范围1-31
             dt->hours,        // 24小时制的小时数
             dt->minutes,      // 分钟数
             dt->seconds);     // 秒数
    
    // 确保字符串终止符
    buf[19] = '\0';
}




void TIM3_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 1. 使能TIM3的APB1总线时钟（RCC_APB1Periph_TIM3）
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);

    // 2. 配置时基单元参数
    TIM_TimeBaseInitStructure.TIM_Period = 10-1;         // 自动重装载值，10个计数周期后溢出
    TIM_TimeBaseInitStructure.TIM_Prescaler = 7200-1;    // 预分频系数，72MHz/7200=10kHz计数频率
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;  // 向上计数模式
    TIM_TimeBaseInitStructure.TIM_ClockDivision = 0;     // 时钟分频因子（与输入捕获相关）
    TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
    
    // 3. 配置从模式触发设置
    TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Reset);     // 触发时计数器复位
    TIM_SelectInputTrigger(TIM3, TIM_TS_TI1FP1);        // 使用TI1FP1作为触发源
    
    // 4. 中断配置
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);          // 使能更新事件中断
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;      // 指定TIM3中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F; // 抢占优先级15（最低）
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;        // 子优先级15（最低）
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 5. 启动定时器
    TIM_Cmd(TIM3,ENABLE);
}


void TIM2_Init(void) {
    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // 1. 使能TIM2的APB1总线时钟（26.88MHz）
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // 2. 配置时基单元参数（300秒定时周期）
    TIM_InitStruct.TIM_Prescaler = 26879;       // 预分频值26879+1=26880
                                                // 26.88MHz / 26880 = 1kHz计数频率
    TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数模式
    TIM_InitStruct.TIM_Period = 59999;         // 自动重装载值299999+1=300000
                                                // 1kHz下300000计数 = 300秒周期
    TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1; // 时钟分频（与输入捕获无关）
    TIM_TimeBaseInit(TIM2, &TIM_InitStruct);
    
    // 清除更新事件标志位（避免首次立即触发中断）
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    
    // 启动定时器基础计数
    TIM_Cmd(TIM2, ENABLE);

    // 3. 使能定时器更新中断
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    // 4. 配置NVIC中断控制器
    NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn; // TIM2全局中断
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x0F; // 抢占优先级15（最低）
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x01;        // 子优先级1
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}


void TIM5_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    
    // 1. 使能TIM5的APB1总线时钟（26.88MHz）
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    
    // 2. 配置时基单元参数（60秒定时周期）
    TIM_InitStruct.TIM_Prescaler = 26879;          // 预分频值26879+1=26880
    TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数模式
    TIM_InitStruct.TIM_Period = 29999;            // 自动重装载值59999+1=60000
                                                  // 1kHz下60000计数 = 60秒周期
    
    // 被注释的备选配置：600秒周期（600000计数）
    //    TIM_InitStruct.TIM_Prescaler = 26879;          
    //    TIM_InitStruct.TIM_Period = 599999;  
    
    TIM_TimeBaseInit(TIM5, &TIM_InitStruct);
    
    // 清除更新事件标志位（初始化后立即清除）
    TIM_ClearFlag(TIM5, TIM_FLAG_Update);
    // 使能定时器更新中断
    TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
    // 启动定时器
    TIM_Cmd(TIM5, ENABLE);
    
    // 配置NVIC中断控制器
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TIM5_IRQn;  // TIM5全局中断
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;  // 抢占优先级1
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;         // 子优先级0
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}


void TIM3_IRQHandler(void) 
{
    // 检测TIM3更新事件中断标志
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) 
    { 
        // 清除中断挂起位（必须操作）
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        
        // 当UART2接收计数器有数据时，触发MQTT接收标志
        if(UartFlagStC.UART2Counter>0){
            UartFlagStC.MQTTReceiveFlag =1;  // 置位MQTT数据接收标志
        }
    }
}
