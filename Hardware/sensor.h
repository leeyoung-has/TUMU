#ifndef __SENSOR_H
#define __SENSOR_H
#include "stm32f4xx.h"
#include "usart.h"

#define MAX_RETRY 3    // 最大重试次数
#define TIMEOUT_MS 500  // 单次等待超时时间(ms)
#define POLL_INTERVAL 20
#define ANGLE_POINT_COUNT 8 //一对多测点数
#define F0 1460.2f //一对一初始频率值
#define K 7200e-5f //灵敏度系数




/*一对一数据结构体*/
typedef struct {
    uint8_t channel;     // 通道数
    float data;          // 数据(IEEE 754)
    char unit[9];        // 单位字符串
} ChannelData;

/*一对多数据结构体*/
// 角度数据结构
typedef struct {
    float roll;     // X
    float pitch;    // Y
    float yaw;      // Z
} AngleData;

//数据处理状态
typedef enum {
    // 通用状态码
    SENSOR_OK = 0,      // 操作成功
    SENSOR_ERR,         // 操作错误
    
    // 通信层错误
    SENSOR_ERR_TIMEOUT, // 接收超时
    SENSOR_ERR_CRC,     // CRC校验失败
    
    // 协议层错误
    SENSOR_ERR_HEADER,  // 协议头错误
    SENSOR_ERR_CMD,     // 指令码不匹配
    SENSOR_ERR_DATALEN, // 数据长度异常
    
    // 数据层错误
    SENSOR_ERR_NODATA   // 无有效数据
} SensorStatus;


//函数声明
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

