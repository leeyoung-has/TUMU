
#ifndef		___SOFTWARE_IIC
#define		___SOFTWARE_IIC

#include  "stm32f4xx.h"

//引脚定义
#define IIC_SCL_PIN    GPIO_Pin_4      // PE4
#define IIC_SDA_PIN    GPIO_Pin_5      // PE5
#define IIC_GPIO_PORT      GPIOE           // 端口E

// 引脚操作宏
#define IIC_SCL_HIGH()  GPIO_SetBits(IIC_GPIO_PORT, IIC_SCL_PIN)
#define IIC_SCL_LOW()   GPIO_ResetBits(IIC_GPIO_PORT, IIC_SCL_PIN)
#define IIC_SDA_HIGH()  GPIO_SetBits(IIC_GPIO_PORT, IIC_SDA_PIN)
#define IIC_SDA_LOW()   GPIO_ResetBits(IIC_GPIO_PORT, IIC_SDA_PIN)
#define IIC_SDA_READ()  GPIO_ReadInputDataBit(IIC_GPIO_PORT, IIC_SDA_PIN)
//函数声明
void IIC_Init(void);
void IIC_Start(void);
void IIC_Stop(void);
void IIC_Ack(void);
void IIC_NAck(void);
uint8_t IIC_Wait_Ack(void);
void IIC_WriteByte(uint8_t txd);
uint8_t IIC_Read_Byte(uint8_t ack);

#endif

