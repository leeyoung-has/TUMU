#include "softwareIIC.h"
#include "delay.h"


void IIC_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);  // ʹ��GPIOEʱ��

    GPIO_InitStruct.GPIO_Pin = IIC_SCL_PIN| IIC_SDA_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;       // ��ʼ��Ϊ���ģʽ
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;      // ��©���
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;        // ��������
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;   // ����ģʽ
    GPIO_Init(IIC_GPIO_PORT, &GPIO_InitStruct);
	
	 //��ʼ״̬��SCL��SDA��Ϊ�ߵ�ƽ
	  IIC_SCL_HIGH();
    IIC_SDA_HIGH(); 
}

//����IIC��ʼ�ź�
void IIC_Start(void){
	 IIC_SDA_HIGH();
    IIC_SCL_HIGH();
    delay_us(5);    // 
    IIC_SDA_LOW();  // SDA�ɸ߱��ʱ����
    delay_us(5);
    IIC_SCL_LOW();  // ǯס����,׼����������
}

void IIC_Stop(void) {
    IIC_SDA_LOW();
    IIC_SCL_LOW();
    delay_us(5);
    IIC_SCL_HIGH();
    delay_us(5);
    IIC_SDA_HIGH();  // SDA�ɵͱ��ʱֹͣ
    delay_us(5);
}

// ����Ӧ���ź�
void IIC_Ack(void) {
    IIC_SCL_LOW();
    IIC_SDA_LOW();  // SDAΪ�͵�ƽ��ʾӦ��
    delay_us(2);
    IIC_SCL_HIGH();
    delay_us(2);
    IIC_SCL_LOW();
}

// ���ͷ�Ӧ���ź�
void IIC_NAck(void) {
    IIC_SCL_LOW();
    IIC_SDA_HIGH();  // SDAΪ�ߵ�ƽ��ʾ��Ӧ��
    delay_us(2);
    IIC_SCL_HIGH();
    delay_us(2);
    IIC_SCL_LOW();
}

// �ȴ��ӻ���Ӧ(����0��ʾӦ��ɹ�)
uint8_t IIC_Wait_Ack(void) {
    uint8_t timeout = 0;

    IIC_SCL_LOW();
    IIC_SDA_HIGH();  // �ͷ�SDA��
    delay_us(1);
    IIC_SCL_HIGH();  // ��������SCL

    // ���SDA�Ƿ�Ϊ�͵�ƽ(�ӻ�Ӧ��)
    while (IIC_SDA_READ()) {  // 
        timeout++;
        if (timeout > 300) {
            IIC_Stop();       // ��ʱ��ֹͣͨ��
            return 1;
        }
        delay_us(1);
    }
    IIC_SCL_LOW();
    return 0;
}

// ����һ���ֽ�
void IIC_WriteByte(uint8_t txd) {
    uint8_t i;

    for (i = 0; i < 8; i++) {
        IIC_SCL_LOW();
        if (txd & 0x80)  // �������λ
            IIC_SDA_HIGH();
        else
            IIC_SDA_LOW();
        txd <<= 1;
        delay_us(2);
        IIC_SCL_HIGH();  // ��������������
        delay_us(2);
        IIC_SCL_LOW();
        delay_us(2);
    }
}

// ��ȡһ���ֽ�
uint8_t IIC_Read_Byte(uint8_t ack) {
    uint8_t i, receive = 0;

    IIC_SDA_HIGH();  // �ͷ�SDA��
    for (i = 0; i < 8; i++) {
        receive <<= 1;
        IIC_SCL_LOW();
        delay_us(2);
        IIC_SCL_HIGH();  // ��������SCL,�ӻ���������λ
        if (IIC_SDA_READ())
            receive |= 0x01;  // ��ȡSDA״̬
        delay_us(2);
        IIC_SCL_LOW();
        delay_us(2);
    }
    if (ack)
        IIC_Ack();  // ����Ӧ��
    else
        IIC_NAck(); // ���ͷ�Ӧ��
    return receive;
}

