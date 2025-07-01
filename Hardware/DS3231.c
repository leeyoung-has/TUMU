#include "softwareIIC.h"
#include "DS3231.h"
#include "stdio.h"

void DS3231_Init(void){
	IIC_Init();
}

uint8_t IIC_DS3231_ByteWrite(uint8_t WriteAddr , uint8_t date)
{
	IIC_Start();
	IIC_WriteByte(DS3231_ADDRESS_Write);
	if(IIC_Wait_Ack())
		return 1;
	IIC_WriteByte(WriteAddr);
	if(IIC_Wait_Ack())
		return 2;
	IIC_WriteByte(date);
	if(IIC_Wait_Ack())
		return 3;
	IIC_Stop();
	return 0;
}

uint8_t IIC_DS3231_ByteRead(uint8_t ReadAddr,uint8_t* Receive)
{
	uint8_t data = 0;
	
	IIC_Start();													//������ʼλ
	IIC_WriteByte(DS3231_ADDRESS_Write); 	//���ʹӻ���ַ(дģʽ)
	if(IIC_Wait_Ack())										//�ȴ���Ӧ
		return 1;
	IIC_WriteByte(ReadAddr);							//���ͼĴ�����ַ
	if(IIC_Wait_Ack())										//�ȴ���Ӧ
		return 2;
	IIC_Start();													//�ظ���ʼλ
	IIC_WriteByte(DS3231_ADDRESS_Read);		//���ʹӻ���ַ(��ģʽ)
	if(IIC_Wait_Ack())										//�ȴ���Ӧ
		return 3;
	data = IIC_Read_Byte(0);							//��ȡ����,������Ϊ0 --- NACK
	*Receive = data;											//�������ֵ������λ
	IIC_Stop();
	return 0;
}

uint8_t DS3231_setDate(uint8_t year,uint8_t mon,uint8_t day)
{
	uint8_t temp_H , temp_L;
	temp_L = year%10;
	temp_H = year/10;
	year = (temp_H << 4) + temp_L;
	if(IIC_DS3231_ByteWrite(DS3231_YEAR_REG,year)) //set year
	{
			printf("set year error\r\n");
			return 1;
	}	
	temp_L = mon%10;
	temp_H = mon/10;
	mon = (temp_H << 4) + temp_L;	
	if(IIC_DS3231_ByteWrite(DS3231_MONTH_REG,mon)) //set mon
	{
		printf("set month error\r\n");
		return 2;
	}
	temp_L = day%10;
	temp_H = day/10;
	day = (temp_H << 4) + temp_L;		
	if(IIC_DS3231_ByteWrite(DS3231_MDAY_REG,day)) //set day
	{
		printf("set day error\r\n");
		return 3;
	}
	return 0;
}


uint8_t DS3231_setTime(uint8_t hour , uint8_t min , uint8_t sec)
{
	uint8_t temp_H , temp_L;
	temp_L = hour%10;
	temp_H = hour/10;
	hour = (temp_H << 4) + temp_L;
	if(IIC_DS3231_ByteWrite(DS3231_HOUR_REG,hour)) //set hour
		return 1;
	temp_L = min%10;
	temp_H = min/10;
	min = (temp_H << 4) + temp_L;
	if(IIC_DS3231_ByteWrite(DS3231_MIN_REG,min)) //SET min
		return 2;	
	temp_L = sec%10;
	temp_H = sec/10;
	sec = (temp_H << 4) + temp_L;	
	if(IIC_DS3231_ByteWrite(DS3231_SEC_REG,sec))		//SET sec
		return 3;
	return 0;
}

static uint8_t bcdToDec(uint8_t byte)
{
	uint8_t temp_H , temp_L;
	temp_L = byte & 0x0f;
	temp_H = (byte & 0xf0) >> 4;
	return ( temp_H * 10 )+ temp_L;
}

 uint8_t DEC_to_BCD(uint8_t dec) {
  return ((dec / 10) << 4) | (dec % 10);
}


uint8_t DS3231_gettime(DateTime* ans)
{
	uint8_t receive = 0;
	if(IIC_DS3231_ByteRead(DS3231_HOUR_REG,&receive))
		return 1;
	ans->hours = bcdToDec(receive);
	if(IIC_DS3231_ByteRead(DS3231_MIN_REG,&receive))
		return 2;
	ans->minutes = bcdToDec(receive);
	if(IIC_DS3231_ByteRead(DS3231_SEC_REG,&receive))
		return 3;
	ans->seconds = bcdToDec(receive);
	return 0;
}

uint8_t DS3231_getdate(DateTime* ans)
{
	uint8_t receive = 0;
	if(IIC_DS3231_ByteRead(DS3231_YEAR_REG,&receive))
		return 1;
	ans->year = bcdToDec(receive) + 2000;
	if(IIC_DS3231_ByteRead(DS3231_MONTH_REG,&receive))
		return 2;
	ans->month = bcdToDec(receive);
	if(IIC_DS3231_ByteRead(DS3231_MDAY_REG,&receive))
		return 3;
	ans->day= bcdToDec(receive);
	return 0;
}

void EXTI_Config(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // ʹ��GPIOE��AFIOʱ��
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    // ����PE6Ϊ����ģʽ
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; 	// ��������
    GPIO_Init(GPIOE, &GPIO_InitStruct);

    // ��PE6ӳ��EXTI Line6
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource6);

    // ����EXTI Line6Ϊ�½��ش���
    EXTI_InitStruct.EXTI_Line = EXTI_Line6;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    // ����NVIC
    NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;  // PE6����EXTI9_5�ж���
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void DS3231_SetAlarm1(DateTime alarm_time) {
    /* Alarm1�Ĵ�����ַ:0x07(��),0x08(��),0x09(ʱ),0x0A(����)*/
    
    // ��Ĵ���(A1M1=0��ʾ����ƥ��)
    IIC_DS3231_ByteWrite(0x07, DEC_to_BCD(alarm_time.seconds)& 0x7F); 
    
    // �ּĴ���(A1M2=0��ʾ����ƥ��)
    IIC_DS3231_ByteWrite(0x08, DEC_to_BCD(alarm_time.minutes) & 0x7F); 
    
    // ʱ�Ĵ���(A1M3=0,24Сʱ��)
    IIC_DS3231_ByteWrite(0x09, DEC_to_BCD(alarm_time.hours) & 0x7F); 
    
	
    /* ���ڼĴ�������   
       - bit7: DY/DT=0 ��ʾ����ƥ��(������)
       - bit6-0: ����ֵ(BCD��ʽ)
       - A1M4=0 ��ʾ����ƥ�� */
    IIC_DS3231_ByteWrite(0x0A, DEC_to_BCD(alarm_time.day) & 0x7F); 
	  uint8_t ctrl_reg;
    IIC_DS3231_ByteRead(0x0E, &ctrl_reg);
    IIC_DS3231_ByteWrite(0x0E, ctrl_reg | 0x05);
}

/* Alarm2���ú��� */
void DS3231_SetAlarm2(DateTime alarm_time) {
    /* Alarm2�Ĵ�����ַ:0x0B(��)?0x0C(ʱ)?0x0D(����/����)*/
    
    // �ּĴ���(A2M2=0��ʾ����ƥ��)
    IIC_DS3231_ByteWrite(0x0B, DEC_to_BCD(alarm_time.minutes) & 0x7F); 
    
    // ʱ�Ĵ���(A2M3=0,24Сʱ��)
    IIC_DS3231_ByteWrite(0x0C, DEC_to_BCD(alarm_time.hours) & 0x7F); 
    
    /* ���ڼĴ�������
       - bit7: DY/DT=0 ��ʾ����ƥ��(������)
       - bit6-0: ����ֵ(BCD��ʽ)
       - A2M4=1 �������� */
    IIC_DS3231_ByteWrite(0x0D, 0x80); 
}

