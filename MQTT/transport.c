#include "stm32f4xx.h"
#include "LED.h"
#include "delay.h"
#include "usart.h"
#include "stdio.h"
#include "transport.h"
#include "string.h"

                                                   

void WiFi_SendString(uint8_t* str,uint8_t counter)//���ڷ��ͺ���
{
	for(int i = 0;i<counter;i++)
	{
		USART_SendData(USART2,*str);
		while(USART_GetFlagStatus(USART2,USART_FLAG_TXE) == RESET);//�жϷ������ݻ������Ƿ�Ϊ��
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
    
    // �����ٽ籣����������ȫ���жϣ�
    // ��ֹ�ڲ���������ʱ���жϴ�ϣ���֤����һ����
    __disable_irq();
    
    // �����ջ��������Ƿ��д���ȡ����
    if (UartFlagStC.UART2Counter > 0) {
        // ����ʵ�ʿɶ�ȡ���ֽ�����ȡ������ȡ����ʵ�ʻ������Ľ�Сֵ
        rc = (count < UartFlagStC.UART2Counter) ? count : UartFlagStC.UART2Counter;
        
        // �����ݴӻ��λ��������Ƶ�Ӧ�û�����
        memcpy(buf, UartFlagStC.UART2String, rc);
        
        // ��������ʣ�����ݣ�
        // ��δ��ȡ��ʱ����ʣ�������ƶ���������ͷ����ʵ�ֻ��λ�����Ч����
        if (UartFlagStC.UART2Counter > rc) {
            memmove(UartFlagStC.UART2String,      // Ŀ���ַ����������ʼλ��
                    UartFlagStC.UART2String + rc, // Դ��ַ��δ��ȡ���ݵ���ʼλ��
                    UartFlagStC.UART2Counter - rc // �ƶ��ֽ�����ʣ��δ��������
            );
        }
        
        // ���»�����������������Ѷ�ȡ���򣨰�ȫ��ʩ��
        UartFlagStC.UART2Counter -= rc;
        // ���Ѷ�ȡ���ֵĻ��������㣬����������ݱ��ظ���ȡ
        memset(UartFlagStC.UART2String + UartFlagStC.UART2Counter, 0, rc);
    }
    
    // �˳��ٽ籣�������ָ�ȫ���жϣ�
    __enable_irq();
    
    return rc;  // ����ʵ�ʶ�ȡ���ֽ���
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
