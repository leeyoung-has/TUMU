#ifndef __MQTT_H_
#define	__MQTT_H_

#include "stm32f4xx.h"
#include <stdbool.h>
//*******MQTT����������*******
//MQTT��Ԫ��
#define Client_ID "test"
#define UserName "admin"
#define Password "Hust@bim415"

//�������ⶩ���뷢��
#define Pub_Topic "$oc/devices/67b841362ff1872637dd9106_stm32/sys/properties/report"
#define Sub_Topic "$oc/devices/67b841362ff1872637dd9106_stm32/sys/properties/down"

//********���ⶩ���뷢��******
#define Pub_Topic1 "up/point/auto" //�Զ��������(һ��һ) �ɼ��ϱ�
#define Pub_Topic2 "up/point/sensor" //�Զ��ɼ�����һ�Զ�ɼ��ϱ�
#define Pub_Topic3 "up/point/semi" //���Զ�������ݲɼ��ϱ�
#define Pub_Topic4 "up/device/status" //�豸״̬�����ϱ�

#define Sub_Topic1 "down/point/auto" //�Զ��������Ԥ���·�
#define Sub_Topic2 "down/point/semi" //���Զ�������ݲɼ��·�
#define Sub_Topic3 "down/point/imt" //�Զ�������ݼӲ��·�
#define Sub_Topic4 "down/point/frq" //�Զ��������Ƶ�ʱ���·�

//���������
#define MQTT_SUCCESS            0x00  // �ɹ�
#define MQTT_ERR_INVALID_PARAM  0xE0  // �����Ƿ�
#define MQTT_ERR_SERIALIZE_FAIL 0xE1  // ���л�ʧ��
#define MQTT_ERR_NETWORK_FAIL   0xE2  // ���緢��ʧ��
#define MQTT_ERR_DESERIALIZE_FAIL 0xE3 // �����л�ʧ��
#define MQTT_ERR_PACKET_INVALID 0xE4  // ���ĸ�ʽ����
#define MQTT_ERR_TIMEOUT        0xE5  // ��Ӧ��ʱ
#define MQTT_ERR_CONNACK_BASE   0xF0  // CONNACK�����ַ

#define MAX_RECONNECT_ATTEMPTS 5  // ����������Դ���
#define RECONNECT_BASE_DELAY_MS 4000  // ���������ӳ�

//������IP
#define Sever_IP "117.78.5.125"
#define Sever_Port 1883



#define MQTT_MAX_BUFF 512

//�����������ȼ�
enum REQ_QoS
{
	REQ_QoS_0=0,
	REQ_QoS_1,
	REQ_QoS_2,
	
};

extern u8  mqtt_buf[MQTT_MAX_BUFF];
extern u16 rx_cnt;
extern volatile uint32_t offline_flag;//���������߱�־


u8 MQTT_Connect(char *client_id,char *user_name,char *password,u16 keep_alive);//MQTT����������
void MQTT_Disconnect(void);//�Ͽ�MQTT����������
u8 MQTT_Subscribe_Topic(char *topic,int req_qos,int msgid);//��������
u8 MQTT_Unsubscribe_Topic(char *topic,int msgid); //ȡ������
u8 MQTT_Publish(char *pub_topic, unsigned char* payload);//���ⷢ��
u8 MQTT_Heart_Send(void);//����������
uint8_t test(void);//������Ӳ��Ժ���
void MQTT_Client_Task(void);//���������Ӻ���

#endif



