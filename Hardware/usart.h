#ifndef __USART_H
#define __USART_H

#define USART_REC_LEN		200 
#define RX_BUF_SIZE 1024
#define USART3_RX_BUFFER_SIZE 256  // ����3��������С
//����ͻ����ֽ���
// UART2����״̬���ƽṹ��
typedef struct {
 volatile    uint16_t UART2Counter;   // ��ǰ�����ֽ���(:0\~512)
    uint8_t UART2String[512]; //�������ݻ�����(���512�ֽ�)
     uint8_t MQTTReceiveFlag; // MQTT���ݽ�����ɱ�־(1=�����ݴ�����)
} UartFlagStruct;
extern UartFlagStruct UartFlagStC;
//����1�������ݻ�����
extern u8  USART_RX_BUF[USART_REC_LEN];
extern u16 USART_RX_STA;
//����3���ݽṹ����

typedef struct {
    uint8_t buffer[USART3_RX_BUFFER_SIZE];  // ���ܻ�����
    volatile uint16_t head;                  // д��λ��
    volatile uint16_t tail;                  // ��ȡλ��
    volatile uint8_t data_ready;             // ���ݾ�����־
} USART3_Dev;
extern USART3_Dev usart3_dev;
//��������
void uart1_init(u32 bound);
void uart2_init(u32 bound);
void uart3_init(u32 bound);
void USART_SendString(USART_TypeDef* USARTx, const char* str);
void USART_SendChar(USART_TypeDef* USARTx, uint8_t ch);
#endif
