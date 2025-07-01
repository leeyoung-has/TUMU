#include "softwareIIC.h"
#include "delay.h"


void IIC_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);  // 使能GPIOE时钟

    GPIO_InitStruct.GPIO_Pin = IIC_SCL_PIN| IIC_SDA_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;       // 初始化为输出模式
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;      // 开漏输出
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;        // 上拉电阻
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;   // 高速模式
    GPIO_Init(IIC_GPIO_PORT, &GPIO_InitStruct);
	
	 //初始状态：SCL和SDA均为高电平
	  IIC_SCL_HIGH();
    IIC_SDA_HIGH(); 
}

//产生IIC起始信号
void IIC_Start(void){
	 IIC_SDA_HIGH();
    IIC_SCL_HIGH();
    delay_us(5);    // 
    IIC_SDA_LOW();  // SDA由高变低时启动
    delay_us(5);
    IIC_SCL_LOW();  // 钳住总线,准备发送数据
}

void IIC_Stop(void) {
    IIC_SDA_LOW();
    IIC_SCL_LOW();
    delay_us(5);
    IIC_SCL_HIGH();
    delay_us(5);
    IIC_SDA_HIGH();  // SDA由低变高时停止
    delay_us(5);
}

// 发送应答信号
void IIC_Ack(void) {
    IIC_SCL_LOW();
    IIC_SDA_LOW();  // SDA为低电平表示应答
    delay_us(2);
    IIC_SCL_HIGH();
    delay_us(2);
    IIC_SCL_LOW();
}

// 发送非应该信号
void IIC_NAck(void) {
    IIC_SCL_LOW();
    IIC_SDA_HIGH();  // SDA为高电平表示非应答
    delay_us(2);
    IIC_SCL_HIGH();
    delay_us(2);
    IIC_SCL_LOW();
}

// 等待从机回应(返回0表示应答成功)
uint8_t IIC_Wait_Ack(void) {
    uint8_t timeout = 0;

    IIC_SCL_LOW();
    IIC_SDA_HIGH();  // 释放SDA线
    delay_us(1);
    IIC_SCL_HIGH();  // 主机拉高SCL

    // 检测SDA是否为低电平(从机应答)
    while (IIC_SDA_READ()) {  // 
        timeout++;
        if (timeout > 300) {
            IIC_Stop();       // 超时则停止通信
            return 1;
        }
        delay_us(1);
    }
    IIC_SCL_LOW();
    return 0;
}

// 发送一个字节
void IIC_WriteByte(uint8_t txd) {
    uint8_t i;

    for (i = 0; i < 8; i++) {
        IIC_SCL_LOW();
        if (txd & 0x80)  // 发送最高位
            IIC_SDA_HIGH();
        else
            IIC_SDA_LOW();
        txd <<= 1;
        delay_us(2);
        IIC_SCL_HIGH();  // 上升沿锁存数据
        delay_us(2);
        IIC_SCL_LOW();
        delay_us(2);
    }
}

// 读取一个字节
uint8_t IIC_Read_Byte(uint8_t ack) {
    uint8_t i, receive = 0;

    IIC_SDA_HIGH();  // 释放SDA线
    for (i = 0; i < 8; i++) {
        receive <<= 1;
        IIC_SCL_LOW();
        delay_us(2);
        IIC_SCL_HIGH();  // 主机拉高SCL,从机发送数据位
        if (IIC_SDA_READ())
            receive |= 0x01;  // 读取SDA状态
        delay_us(2);
        IIC_SCL_LOW();
        delay_us(2);
    }
    if (ack)
        IIC_Ack();  // 发送应答
    else
        IIC_NAck(); // 发送非应答
    return receive;
}

