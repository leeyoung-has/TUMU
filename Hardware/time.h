#ifndef __TIME_H
#define __TIME_H

#include "stm32f4xx.h"
 typedef struct {
    uint8_t year;    // ��� (0-99, 2000\~2099)
    uint8_t month;   // �·� (1-12)
    uint8_t day;     // �� (1-31)
   
    uint8_t hours;   // ʱ (0-23)
    uint8_t minutes; // ��(0-59)
    uint8_t seconds; // �� (0-59)
} DateTime;
 
void TIM2_Init(void);
void TIM3_Init(void);
void RTC_GetDateTime(DateTime *dt);
void MyRTC_Init(void);
void DateTime_ToString( const DateTime* dt, char* buf);
void set_rtc_alarm(const DateTime* time);
void AlarmB_Config(void);
void TIM4_Init(void);
void TIM5_Init(void);

#endif
