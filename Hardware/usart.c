#include "stm32f4xx.h"
#include "usart.h"
#include "stdio.h"
#include <string.h>


// UART2接受状态控制结构体

//声明全局变量
UartFlagStruct UartFlagStC = {
    .UART2Counter = 0,  
    .UART2String = {0},
    .MQTTReceiveFlag = 0
};

u8  USART_RX_BUF[USART_REC_LEN];
u16 USART_RX_STA=0;

USART3_Dev usart3_dev = {0};

//printf 函数重定向
int fputc(int ch,FILE *p) 
 
{
 
 USART_SendData(USART1,(u8)ch);
 
 while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
 
 return ch;
 
}

/* 发送单个字符  */
void USART_SendChar(USART_TypeDef* USARTx, uint8_t ch) {
    USART_SendData(USARTx, ch);
    while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
}

/*发送字符串函数  */
void USART_SendString(USART_TypeDef* USARTx, const char* str) {
     while(*str != '\0')
    {
        // 等待上一个字符发送完成
        while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
        
        // 发送当前字符
        USART_SendData(USARTx, (uint16_t)*str);
        str++;
    }
}

void uart1_init(u32 bound) //初始化串口1
{
	GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//使能USART1 
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); //PA9复用为USART1
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); //PA10复用为USART1
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9 与GPIOA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //速度50MHz
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
  GPIO_Init(GPIOA,&GPIO_InitStructure); // 初始化PA9,PA10
	
	//初始化串口
	
	USART_InitStructure.USART_BaudRate = bound;//波特率;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长
  USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
  USART_InitStructure.USART_Parity = USART_Parity_No;//奇偶校验位
  USART_InitStructure.USART_HardwareFlowControl =USART_HardwareFlowControl_None; 
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //收发模式
  USART_Init(USART1, &USART_InitStructure); //初始化串口
	USART_Cmd(USART1, ENABLE);//使能串口
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启中断
	//usart中断配置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;//抢占优先级2
  NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;//字优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//IQR使能通道
	NVIC_Init(&NVIC_InitStructure);
}

void uart3_init(u32 bound) //初始化串口3
{
	GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);	//使能GPIOB
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);  //使能GPIOD
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); //使能串口3
	
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_USART3); //PB10复用为USART3
  GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_USART3); //PB11复用为USART3
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11; //GPIOB10 与GPIOB11
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //速度50MHz
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
  GPIO_Init(GPIOB,&GPIO_InitStructure); // 初始化PB10,PB11
	
	 //配置PD10为RE控制引脚(推挽输出，默认低电平)
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  GPIO_ResetBits(GPIOD, GPIO_Pin_10); // 初始化为接收模式
	
	//初始化串口
	
	USART_InitStructure.USART_BaudRate = bound;//波特率;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长
  USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
  USART_InitStructure.USART_Parity = USART_Parity_No;//奇偶校验位
  USART_InitStructure.USART_HardwareFlowControl =USART_HardwareFlowControl_None; 
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //收发模式
  USART_Init(USART3, &USART_InitStructure); //初始化串口
	USART_Cmd(USART3, ENABLE);//使能串口
	
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启中断
  USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);  // 启用空闲中断

	//usart中断配置
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//抢占优先级2
  NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;//字优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//IQR使能通道
	NVIC_Init(&NVIC_InitStructure);
}

void uart2_init(u32 bound) //初始化串口2
{
	GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//使能USART2
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2); //PA2复用为USART1
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2); //PA3复用为USART1
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2| GPIO_Pin_3; //GPIOA2 与GPIOA3
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //速度50MHz
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
  GPIO_Init(GPIOA,&GPIO_InitStructure); // 初始化PA2,PA3
	
	//初始化串口
	
	USART_InitStructure.USART_BaudRate = bound;//波特率;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长
  USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
  USART_InitStructure.USART_Parity = USART_Parity_No;//奇偶校验位
  USART_InitStructure.USART_HardwareFlowControl =USART_HardwareFlowControl_None; 
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //收发模式
  USART_Init(USART2, &USART_InitStructure); //初始化串口
	USART_Cmd(USART2, ENABLE);//使能串口
	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断
	//usart中断配置
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;//抢占优先级2
  NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;//字优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//IQR使能通道
	NVIC_Init(&NVIC_InitStructure);
}

void USART1_IRQHandler(void)
{
	u8 Res;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)//接受中断
	{
    Res =USART_ReceiveData(USART1);//读取接受到的数据
    if((USART_RX_STA&0x8000)==0)//接受未完成,&按位与保留二进制位置相同的数
      {
       if(USART_RX_STA&0x4000)//接受到0x0d
        {
          if(Res!=0x0a)USART_RX_STA=0;//接受错误，重新接收
          else USART_RX_STA|=0x8000; //接收完成
        }
    else 
      {
      if(Res==0x0d)USART_RX_STA|=0x4000;//如果接收到了0d，次高位置一
      else
      {
        USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;//将数据存入缓存区
        USART_RX_STA++; //计数加一
        if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0; //超出最大

			}

			}

			} 

	}
}


//串口2中断服务函数，用于向mqtt服务器收发
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



//串口3中断服务函数，用于接收传感器数据
void USART3_IRQHandler(void){ 
        // RXNE接收中断
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) {
        uint8_t data = USART_ReceiveData(USART3);
        uint16_t next_head = (usart3_dev.head + 1) % USART3_RX_BUFFER_SIZE;
        
        // 缓冲区未满时写入
        if(next_head != usart3_dev.tail) {
            usart3_dev.buffer[usart3_dev.head] = data;
            usart3_dev.head = next_head;
        }
        USART_ClearITPendingBit(USART3, USART_IT_RXNE); // 清除标志
    }

    // IDLE空闲中断
    if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET) {
        volatile uint32_t temp = USART3->SR;  // 读SR
        temp = USART3->DR;
        USART_ClearITPendingBit(USART3, USART_IT_IDLE);			// 读DR
        usart3_dev.data_ready = 1;            // 触发数据处理
    }
		
  }

	

	