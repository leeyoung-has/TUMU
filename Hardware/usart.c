#include "stm32f4xx.h"
#include "usart.h"
#include "stdio.h"
#include <string.h>


// UART2����״̬���ƽṹ��

//����ȫ�ֱ���
UartFlagStruct UartFlagStC = {
    .UART2Counter = 0,  
    .UART2String = {0},
    .MQTTReceiveFlag = 0
};

u8  USART_RX_BUF[USART_REC_LEN];
u16 USART_RX_STA=0;

USART3_Dev usart3_dev = {0};

//printf �����ض���
int fputc(int ch,FILE *p) 
 
{
 
 USART_SendData(USART1,(u8)ch);
 
 while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
 
 return ch;
 
}

/* ���͵����ַ�  */
void USART_SendChar(USART_TypeDef* USARTx, uint8_t ch) {
    USART_SendData(USARTx, ch);
    while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
}

/*�����ַ�������  */
void USART_SendString(USART_TypeDef* USARTx, const char* str) {
     while(*str != '\0')
    {
        // �ȴ���һ���ַ��������
        while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
        
        // ���͵�ǰ�ַ�
        USART_SendData(USARTx, (uint16_t)*str);
        str++;
    }
}

void uart1_init(u32 bound) //��ʼ������1
{
	GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //ʹ��GPIOA
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//ʹ��USART1 
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); //PA9����ΪUSART1
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); //PA10����ΪUSART1
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9 ��GPIOA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //�ٶ�50MHz
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
  GPIO_Init(GPIOA,&GPIO_InitStructure); // ��ʼ��PA9,PA10
	
	//��ʼ������
	
	USART_InitStructure.USART_BaudRate = bound;//������;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�
  USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
  USART_InitStructure.USART_Parity = USART_Parity_No;//��żУ��λ
  USART_InitStructure.USART_HardwareFlowControl =USART_HardwareFlowControl_None; 
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //�շ�ģʽ
  USART_Init(USART1, &USART_InitStructure); //��ʼ������
	USART_Cmd(USART1, ENABLE);//ʹ�ܴ���
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�����ж�
	//usart�ж�����
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;//��ռ���ȼ�2
  NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;//�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//IQRʹ��ͨ��
	NVIC_Init(&NVIC_InitStructure);
}

void uart3_init(u32 bound) //��ʼ������3
{
	GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);	//ʹ��GPIOB
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);  //ʹ��GPIOD
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); //ʹ�ܴ���3
	
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_USART3); //PB10����ΪUSART3
  GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_USART3); //PB11����ΪUSART3
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11; //GPIOB10 ��GPIOB11
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //�ٶ�50MHz
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
  GPIO_Init(GPIOB,&GPIO_InitStructure); // ��ʼ��PB10,PB11
	
	 //����PD10ΪRE��������(���������Ĭ�ϵ͵�ƽ)
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  GPIO_ResetBits(GPIOD, GPIO_Pin_10); // ��ʼ��Ϊ����ģʽ
	
	//��ʼ������
	
	USART_InitStructure.USART_BaudRate = bound;//������;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�
  USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
  USART_InitStructure.USART_Parity = USART_Parity_No;//��żУ��λ
  USART_InitStructure.USART_HardwareFlowControl =USART_HardwareFlowControl_None; 
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //�շ�ģʽ
  USART_Init(USART3, &USART_InitStructure); //��ʼ������
	USART_Cmd(USART3, ENABLE);//ʹ�ܴ���
	
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//�����ж�
  USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);  // ���ÿ����ж�

	//usart�ж�����
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//��ռ���ȼ�2
  NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;//�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//IQRʹ��ͨ��
	NVIC_Init(&NVIC_InitStructure);
}

void uart2_init(u32 bound) //��ʼ������2
{
	GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //ʹ��GPIOA
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//ʹ��USART2
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2); //PA2����ΪUSART1
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2); //PA3����ΪUSART1
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2| GPIO_Pin_3; //GPIOA2 ��GPIOA3
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //�ٶ�50MHz
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
  GPIO_Init(GPIOA,&GPIO_InitStructure); // ��ʼ��PA2,PA3
	
	//��ʼ������
	
	USART_InitStructure.USART_BaudRate = bound;//������;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�
  USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
  USART_InitStructure.USART_Parity = USART_Parity_No;//��żУ��λ
  USART_InitStructure.USART_HardwareFlowControl =USART_HardwareFlowControl_None; 
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //�շ�ģʽ
  USART_Init(USART2, &USART_InitStructure); //��ʼ������
	USART_Cmd(USART2, ENABLE);//ʹ�ܴ���
	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�����ж�
	//usart�ж�����
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;//��ռ���ȼ�2
  NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;//�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//IQRʹ��ͨ��
	NVIC_Init(&NVIC_InitStructure);
}

void USART1_IRQHandler(void)
{
	u8 Res;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)//�����ж�
	{
    Res =USART_ReceiveData(USART1);//��ȡ���ܵ�������
    if((USART_RX_STA&0x8000)==0)//����δ���,&��λ�뱣��������λ����ͬ����
      {
       if(USART_RX_STA&0x4000)//���ܵ�0x0d
        {
          if(Res!=0x0a)USART_RX_STA=0;//���ܴ������½���
          else USART_RX_STA|=0x8000; //�������
        }
    else 
      {
      if(Res==0x0d)USART_RX_STA|=0x4000;//������յ���0d���θ�λ��һ
      else
      {
        USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;//�����ݴ��뻺����
        USART_RX_STA++; //������һ
        if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0; //�������

			}

			}

			} 

	}
}


//����2�жϷ�������������mqtt�������շ�
void USART2_IRQHandler(void) 
	{
	   if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
	    if(UartFlagStC.UART2Counter<512)
	    {
	      UartFlagStC.UART2String[UartFlagStC.UART2Counter] = USART_ReceiveData(USART2);
	      UartFlagStC.UART2Counter++;
	    }
			else 
		 {
			UartFlagStC.UART2Counter = 0;
			memset(UartFlagStC.UART2String,0,512);
		 }
		TIM3->CNT=0;
		USART_ClearITPendingBit(USART2,USART_IT_RXNE);
	}
    }



//����3�жϷ����������ڽ��մ���������
void USART3_IRQHandler(void){ 
        // RXNE�����ж�
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) {
        uint8_t data = USART_ReceiveData(USART3);
        uint16_t next_head = (usart3_dev.head + 1) % USART3_RX_BUFFER_SIZE;
        
        // ������δ��ʱд��
        if(next_head != usart3_dev.tail) {
            usart3_dev.buffer[usart3_dev.head] = data;
            usart3_dev.head = next_head;
        }
        USART_ClearITPendingBit(USART3, USART_IT_RXNE); // �����־
    }

    // IDLE�����ж�
    if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET) {
        volatile uint32_t temp = USART3->SR;  // ��SR
        temp = USART3->DR;
        USART_ClearITPendingBit(USART3, USART_IT_IDLE);			// ��DR
        usart3_dev.data_ready = 1;            // �������ݴ���
    }
		
  }

	

	