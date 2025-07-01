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
 * @brief ��DateTime�ṹ���ʽ��ΪISO8601��׼�ַ���
 * @param dt ָ��DateTime�ṹ���ָ�룬��������ʱ����Ϣ
 * @param buf ���������������20�ֽڿռ䣩
 */
void DateTime_ToString(const DateTime* dt, char* buf) {
    // ʹ��snprintf���а�ȫ��ʽ��
    snprintf(buf, 20, "%04d-%02d-%02d %02d:%02d:%02d",
             2000 + dt->year,  // RTC���0-99��Ӧ2000-2099
             dt->month,        // �·ݷ�Χ1-12
             dt->day,          // ���ڷ�Χ1-31
             dt->hours,        // 24Сʱ�Ƶ�Сʱ��
             dt->minutes,      // ������
             dt->seconds);     // ����
    
    // ȷ���ַ�����ֹ��
    buf[19] = '\0';
}




void TIM3_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 1. ʹ��TIM3��APB1����ʱ�ӣ�RCC_APB1Periph_TIM3��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);

    // 2. ����ʱ����Ԫ����
    TIM_TimeBaseInitStructure.TIM_Period = 10-1;         // �Զ���װ��ֵ��10���������ں����
    TIM_TimeBaseInitStructure.TIM_Prescaler = 7200-1;    // Ԥ��Ƶϵ����72MHz/7200=10kHz����Ƶ��
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;  // ���ϼ���ģʽ
    TIM_TimeBaseInitStructure.TIM_ClockDivision = 0;     // ʱ�ӷ�Ƶ���ӣ������벶����أ�
    TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
    
    // 3. ���ô�ģʽ��������
    TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Reset);     // ����ʱ��������λ
    TIM_SelectInputTrigger(TIM3, TIM_TS_TI1FP1);        // ʹ��TI1FP1��Ϊ����Դ
    
    // 4. �ж�����
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);          // ʹ�ܸ����¼��ж�
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;      // ָ��TIM3�ж�ͨ��
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F; // ��ռ���ȼ�15����ͣ�
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;        // �����ȼ�15����ͣ�
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 5. ������ʱ��
    TIM_Cmd(TIM3,ENABLE);
}


void TIM2_Init(void) {
    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // 1. ʹ��TIM2��APB1����ʱ�ӣ�26.88MHz��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // 2. ����ʱ����Ԫ������300�붨ʱ���ڣ�
    TIM_InitStruct.TIM_Prescaler = 26879;       // Ԥ��Ƶֵ26879+1=26880
                                                // 26.88MHz / 26880 = 1kHz����Ƶ��
    TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up; // ���ϼ���ģʽ
    TIM_InitStruct.TIM_Period = 59999;         // �Զ���װ��ֵ299999+1=300000
                                                // 1kHz��300000���� = 300������
    TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1; // ʱ�ӷ�Ƶ�������벶���޹أ�
    TIM_TimeBaseInit(TIM2, &TIM_InitStruct);
    
    // ��������¼���־λ�������״����������жϣ�
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    
    // ������ʱ����������
    TIM_Cmd(TIM2, ENABLE);

    // 3. ʹ�ܶ�ʱ�������ж�
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    // 4. ����NVIC�жϿ�����
    NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn; // TIM2ȫ���ж�
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x0F; // ��ռ���ȼ�15����ͣ�
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x01;        // �����ȼ�1
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}


void TIM5_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    
    // 1. ʹ��TIM5��APB1����ʱ�ӣ�26.88MHz��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    
    // 2. ����ʱ����Ԫ������60�붨ʱ���ڣ�
    TIM_InitStruct.TIM_Prescaler = 26879;          // Ԥ��Ƶֵ26879+1=26880
    TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up; // ���ϼ���ģʽ
    TIM_InitStruct.TIM_Period = 29999;            // �Զ���װ��ֵ59999+1=60000
                                                  // 1kHz��60000���� = 60������
    
    // ��ע�͵ı�ѡ���ã�600�����ڣ�600000������
    //    TIM_InitStruct.TIM_Prescaler = 26879;          
    //    TIM_InitStruct.TIM_Period = 599999;  
    
    TIM_TimeBaseInit(TIM5, &TIM_InitStruct);
    
    // ��������¼���־λ����ʼ�������������
    TIM_ClearFlag(TIM5, TIM_FLAG_Update);
    // ʹ�ܶ�ʱ�������ж�
    TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
    // ������ʱ��
    TIM_Cmd(TIM5, ENABLE);
    
    // ����NVIC�жϿ�����
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TIM5_IRQn;  // TIM5ȫ���ж�
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;  // ��ռ���ȼ�1
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;         // �����ȼ�0
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}


void TIM3_IRQHandler(void) 
{
    // ���TIM3�����¼��жϱ�־
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) 
    { 
        // ����жϹ���λ�����������
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        
        // ��UART2���ռ�����������ʱ������MQTT���ձ�־
        if(UartFlagStC.UART2Counter>0){
            UartFlagStC.MQTTReceiveFlag =1;  // ��λMQTT���ݽ��ձ�־
        }
    }
}
