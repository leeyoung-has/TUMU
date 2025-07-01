#include "stm32f4xx.h"
#include "LED.h"
#include "delay.h"
#include "usart.h"
#include "stdio.h"
#include "transport.h"
#include "string.h"

                                                   

void WiFi_SendString(uint8_t* str,uint8_t counter)//串口发送函数
{
	for(int i = 0;i<counter;i++)
	{
		USART_SendData(USART2,*str);
		while(USART_GetFlagStatus(USART2,USART_FLAG_TXE) == RESET);//判断发送数据缓存区是否为空
		str++;
	}
}

int transport_sendPacketBuffer( unsigned char* buf, int buflen)
{
	WiFi_SendString(buf,buflen);
	return buflen;
}

int transport_getdata(unsigned char* buf, int count) {
    int rc = 0;
    
    // 进入临界保护区（禁用全局中断）
    // 防止在操作缓冲区时被中断打断，保证数据一致性
    __disable_irq();
    
    // 检查接收缓冲区中是否有待读取数据
    if (UartFlagStC.UART2Counter > 0) {
        // 计算实际可读取的字节数：取期望读取量和实际缓存量的较小值
        rc = (count < UartFlagStC.UART2Counter) ? count : UartFlagStC.UART2Counter;
        
        // 将数据从环形缓冲区复制到应用缓冲区
        memcpy(buf, UartFlagStC.UART2String, rc);
        
        // 处理缓冲区剩余数据：
        // 当未读取完时，将剩余数据移动到缓冲区头部（实现环形缓冲区效果）
        if (UartFlagStC.UART2Counter > rc) {
            memmove(UartFlagStC.UART2String,      // 目标地址：缓冲区起始位置
                    UartFlagStC.UART2String + rc, // 源地址：未读取数据的起始位置
                    UartFlagStC.UART2Counter - rc // 移动字节数：剩余未读数据量
            );
        }
        
        // 更新缓冲区计数器并清空已读取区域（安全措施）
        UartFlagStC.UART2Counter -= rc;
        // 将已读取部分的缓冲区清零，避免残留数据被重复读取
        memset(UartFlagStC.UART2String + UartFlagStC.UART2Counter, 0, rc);
    }
    
    // 退出临界保护区（恢复全局中断）
    __enable_irq();
    
    return rc;  // 返回实际读取的字节数
}



int transport_getdatanb(void *sck, unsigned char* buf, int count)
{
	return 0;
}

int transport_open(char* addr, int port)
{
	return 0;
}

int transport_close(int sock)
{
	return 0;
}
