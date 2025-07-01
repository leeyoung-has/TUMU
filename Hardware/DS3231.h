#ifndef DS3231_H
#define DS3231_H

#include  "stm32f4xx.h"
#include  "time.h"

#define DS3231_ADDRESS	      0x68 //I2C Slave address
#define	DS3231_ADDRESS_Write	0xD0
#define	DS3231_ADDRESS_Read		0xD1

/* DS3231 Registers. Refer Sec 8.2 of application manual */
#define DS3231_SEC_REG        0x00    // √Î
#define DS3231_MIN_REG        0x01    //
#define DS3231_HOUR_REG       0x02
#define DS3231_WDAY_REG       0x03
#define DS3231_MDAY_REG       0x04
#define DS3231_MONTH_REG      0x05
#define DS3231_YEAR_REG       0x06

#define DS3231_AL1SEC_REG     0x07
#define DS3231_AL1MIN_REG     0x08
#define DS3231_AL1HOUR_REG    0x09
#define DS3231_AL1WDAY_REG    0x0A

#define DS3231_AL2MIN_REG     0x0B
#define DS3231_AL2HOUR_REG    0x0C
#define DS3231_AL2WDAY_REG    0x0D

#define DS3231_CONTROL_REG          0x0E
#define DS3231_STATUS_REG           0x0F
#define DS3231_AGING_OFFSET_REG     0x0F
#define DS3231_TMP_UP_REG           0x11
#define DS3231_TMP_LOW_REG          0x12

#define EverySecond     0x01
#define EveryMinute     0x02
#define EveryHour       0x03

void DS3231_Init(void);
uint8_t IIC_DS3231_ByteWrite(uint8_t WriteAddr , uint8_t date);
uint8_t IIC_DS3231_ByteRead(uint8_t ReadAddr,uint8_t* Receive);
uint8_t DS3231_setDate(uint8_t year,uint8_t mon,uint8_t day);
uint8_t DS3231_setTime(uint8_t hour , uint8_t min , uint8_t sec);
static uint8_t bcdToDec(uint8_t byte);
uint8_t DEC_to_BCD(uint8_t dec);
uint8_t DS3231_gettime(DateTime* ans);
uint8_t DS3231_getdate(DateTime* ans);
void DS3231_SetAlarm1(DateTime alarm_time);
void DS3231_SetAlarm2(DateTime alarm_time);
void EXTI_Config(void);
#endif

