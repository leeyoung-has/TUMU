#ifndef __USART_H
#define __USART_H

#define USART_REC_LEN		200 
#define RX_BUF_SIZE 1024
#define USART3_RX_BUFFER_SIZE 256  // 串口3缓存区大小
//最大发送缓存字节数
// UART2接受状态控制结构体
typedef struct {
 volatile    uint16_t UART2Counter;   // 当前接受字节数(:0\~512)
    uint8_t UART2String[512]; //接受数据缓冲区(最大512字节)
     uint8_t MQTTReceiveFlag; // MQTT数据接受完成标志(1=有数据待处理)
} UartFlagStruct;
extern UartFlagStruct UartFlagStC;
//串口1结束数据缓存区
extern u8  USART_RX_BUF[USART_REC_LEN];
extern u16 USART_RX_STA;
//串口3数据结构定义

typedef struct {
    uint8_t buffer[USART3_RX_BUFFER_SIZE];  // 接受缓冲区
    volatile uint16_t head;                  // 写入位置
    volatile uint16_t tail;                  // 读取位置
    volatile uint8_t data_ready;             // 数据就绪标志
} USART3_Dev;
extern USART3_Dev usart3_dev;
//函数声明
void uart1_init(u32 bound);
void uart2_init(u32 bound);
void uart3_init(u32 bound);
void USART_SendString(USART_TypeDef* USARTx, const char* str);
void USART_SendChar(USART_TypeDef* USARTx, uint8_t ch);
#endif
