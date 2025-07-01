#ifndef __SENSOR_H
#define __SENSOR_H
#include "stm32f4xx.h"
#include "usart.h"

#define MAX_RETRY 3    // ������Դ���
#define TIMEOUT_MS 500  // ���εȴ���ʱʱ��(ms)
#define POLL_INTERVAL 20
#define ANGLE_POINT_COUNT 8 //һ�Զ�����
#define F0 1460.2f //һ��һ��ʼƵ��ֵ
#define K 7200e-5f //������ϵ��




/*һ��һ���ݽṹ��*/
typedef struct {
    uint8_t channel;     // ͨ����
    float data;          // ����(IEEE 754)
    char unit[9];        // ��λ�ַ���
} ChannelData;

/*һ�Զ����ݽṹ��*/
// �Ƕ����ݽṹ
typedef struct {
    float roll;     // X
    float pitch;    // Y
    float yaw;      // Z
} AngleData;

//���ݴ���״̬
typedef enum {
    // ͨ��״̬��
    SENSOR_OK = 0,      // �����ɹ�
    SENSOR_ERR,         // ��������
    
    // ͨ�Ų����
    SENSOR_ERR_TIMEOUT, // ���ճ�ʱ
    SENSOR_ERR_CRC,     // CRCУ��ʧ��
    
    // Э������
    SENSOR_ERR_HEADER,  // Э��ͷ����
    SENSOR_ERR_CMD,     // ָ���벻ƥ��
    SENSOR_ERR_DATALEN, // ���ݳ����쳣
    
    // ���ݲ����
    SENSOR_ERR_NODATA   // ����Ч����
} SensorStatus;


//��������
void Send_Predefined_CMD(const uint8_t *cmd_ptr, uint16_t cmd_len);
void USART3_get_Data(uint8_t *temp_buf, uint16_t *data_len);
ChannelData* parse_sensor_data(uint8_t *buffer, uint16_t data_len, uint8_t *out_channel_count);
float Get_Channel2_Data(void);
SensorStatus ParseSensorData(const uint8_t *raw_data, uint16_t data_len,
                           AngleData *angles, uint8_t *valid_points);
                    
SensorStatus GetAngleData(AngleData *angles, uint8_t *valid_points);
SensorStatus ParseModbusFrame(const uint8_t *data, uint16_t len, uint8_t *soc);
SensorStatus GetBatterySOC(uint8_t *soc);	
#endif

