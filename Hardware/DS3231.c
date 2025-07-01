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
	
	IIC_Start();													//产生起始位
	IIC_WriteByte(DS3231_ADDRESS_Write); 	//发送从机地址(写模式)
	if(IIC_Wait_Ack())										//等待响应
		return 1;
	IIC_WriteByte(ReadAddr);							//发送寄存器地址
	if(IIC_Wait_Ack())										//等待响应
		return 2;
	IIC_Start();													//重复起始位
	IIC_WriteByte(DS3231_ADDRESS_Read);		//发送从机地址(读模式)
	if(IIC_Wait_Ack())										//等待响应
		return 3;
	data = IIC_Read_Byte(0);							//读取数据,参数设为0 --- NACK
	*Receive = data;											//将结果赋值给接收位
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

    // 使能GPIOE和AFIO时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    // 配置PE6为输入模式
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; 	// 上拉电阻
    GPIO_Init(GPIOE, &GPIO_InitStruct);

    // 将PE6映射EXTI Line6
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource6);

    // 配置EXTI Line6为下降沿触发
    EXTI_InitStruct.EXTI_Line = EXTI_Line6;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    // 配置NVIC
    NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;  // PE6属于EXTI9_5中断组
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void DS3231_SetAlarm1(DateTime alarm_time) {
    /* Alarm1寄存器地址:0x07(秒),0x08(分),0x09(时),0x0A(日期)*/
    
    // 秒寄存器(A1M1=0表示参与匹配)
    IIC_DS3231_ByteWrite(0x07, DEC_to_BCD(alarm_time.seconds)& 0x7F); 
    
    // 分寄存器(A1M2=0表示参与匹配)
    IIC_DS3231_ByteWrite(0x08, DEC_to_BCD(alarm_time.minutes) & 0x7F); 
    
    // 时寄存器(A1M3=0,24小时制)
    IIC_DS3231_ByteWrite(0x09, DEC_to_BCD(alarm_time.hours) & 0x7F); 
    
	
    /* 日期寄存器配置   
       - bit7: DY/DT=0 表示日期匹配(非星期)
       - bit6-0: 日期值(BCD格式)
       - A1M4=0 表示参与匹配 */
    IIC_DS3231_ByteWrite(0x0A, DEC_to_BCD(alarm_time.day) & 0x7F); 
	  uint8_t ctrl_reg;
    IIC_DS3231_ByteRead(0x0E, &ctrl_reg);
    IIC_DS3231_ByteWrite(0x0E, ctrl_reg | 0x05);
}

/* Alarm2配置函数 */
void DS3231_SetAlarm2(DateTime alarm_time) {
    /* Alarm2寄存器地址:0x0B(分)?0x0C(时)?0x0D(日期/星期)*/
    
    // 分寄存器(A2M2=0表示参与匹配)
    IIC_DS3231_ByteWrite(0x0B, DEC_to_BCD(alarm_time.minutes) & 0x7F); 
    
    // 时寄存器(A2M3=0,24小时制)
    IIC_DS3231_ByteWrite(0x0C, DEC_to_BCD(alarm_time.hours) & 0x7F); 
    
    /* 日期寄存器配置
       - bit7: DY/DT=0 表示日期匹配(非星期)
       - bit6-0: 日期值(BCD格式)
       - A2M4=1 忽略日期 */
    IIC_DS3231_ByteWrite(0x0D, 0x80); 
}

