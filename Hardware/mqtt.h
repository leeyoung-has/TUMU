#ifndef __MQTT_H_
#define	__MQTT_H_

#include "stm32f4xx.h"
#include <stdbool.h>
//*******MQTT服务器参数*******
//MQTT三元组
#define Client_ID "test"
#define UserName "admin"
#define Password "Hust@bim415"

//测试主题订阅与发布
#define Pub_Topic "$oc/devices/67b841362ff1872637dd9106_stm32/sys/properties/report"
#define Sub_Topic "$oc/devices/67b841362ff1872637dd9106_stm32/sys/properties/down"

//********主题订阅与发布******
#define Pub_Topic1 "up/point/auto" //自动监测数据(一对一) 采集上报
#define Pub_Topic2 "up/point/sensor" //自动采集数据一对多采集上报
#define Pub_Topic3 "up/point/semi" //半自动监测数据采集上报
#define Pub_Topic4 "up/device/status" //设备状态数据上报

#define Sub_Topic1 "down/point/auto" //自动监测数据预警下发
#define Sub_Topic2 "down/point/semi" //半自动监测数据采集下发
#define Sub_Topic3 "down/point/imt" //自动监测数据加测下发
#define Sub_Topic4 "down/point/frq" //自动监测数据频率变更下发

//错误码设计
#define MQTT_SUCCESS            0x00  // 成功
#define MQTT_ERR_INVALID_PARAM  0xE0  // 参数非法
#define MQTT_ERR_SERIALIZE_FAIL 0xE1  // 序列化失败
#define MQTT_ERR_NETWORK_FAIL   0xE2  // 网络发送失败
#define MQTT_ERR_DESERIALIZE_FAIL 0xE3 // 反序列化失败
#define MQTT_ERR_PACKET_INVALID 0xE4  // 报文格式错误
#define MQTT_ERR_TIMEOUT        0xE5  // 响应超时
#define MQTT_ERR_CONNACK_BASE   0xF0  // CONNACK错误基址

#define MAX_RECONNECT_ATTEMPTS 5  // 最大重连尝试次数
#define RECONNECT_BASE_DELAY_MS 4000  // 基础重连延迟

//服务器IP
#define Sever_IP "117.78.5.125"
#define Sever_Port 1883



#define MQTT_MAX_BUFF 512

//服务器质量等级
enum REQ_QoS
{
	REQ_QoS_0=0,
	REQ_QoS_1,
	REQ_QoS_2,
	
};

extern u8  mqtt_buf[MQTT_MAX_BUFF];
extern u16 rx_cnt;
extern volatile uint32_t offline_flag;//服务器下线标志


u8 MQTT_Connect(char *client_id,char *user_name,char *password,u16 keep_alive);//MQTT服务器连接
void MQTT_Disconnect(void);//断开MQTT服务器连接
u8 MQTT_Subscribe_Topic(char *topic,int req_qos,int msgid);//订阅主题
u8 MQTT_Unsubscribe_Topic(char *topic,int msgid); //取消订阅
u8 MQTT_Publish(char *pub_topic, unsigned char* payload);//主题发布
u8 MQTT_Heart_Send(void);//心跳包发布
uint8_t test(void);//检测连接测试函数
void MQTT_Client_Task(void);//服务器连接函数

#endif



