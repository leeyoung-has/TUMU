#include "mqtt.h"
#include "delay.h"
#include "usart.h"
#include "stdio.h"
#include "string.h"
#include "MQTTPacket.h"  
#include "transport.h"

u8  mqtt_buf[MQTT_MAX_BUFF];
u16 rx_cnt=0;
volatile uint32_t offline_flag=0;

u8 MQTT_Connect(char *client_id, char *user_name, char *password, u16 keep_alive) 
{
    /* �����Ϸ��Լ�� */
    if (client_id == NULL || user_name == NULL || password == NULL) {
        printf("����: �������ڿ�ָ��\n");
        return MQTT_ERR_INVALID_PARAM; // �Զ��������: 0xE0
    }

    /* ��ʼ��MQTT���ӽṹ�� */
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.clientID.cstring = client_id;
    data.keepAliveInterval = keep_alive;
    data.cleansession =1;
    data.username.cstring = user_name;
    data.password.cstring = password;
    data.MQTTVersion = 3;
    /* ���л�CONNECT���� */
    u16 send_len = MQTTSerialize_connect(mqtt_buf, MQTT_MAX_BUFF, &data);
    if (send_len == 0) {
        printf("����: CONNECT�������л�ʧ��\n");
        return MQTT_ERR_SERIALIZE_FAIL; // 0xE1
    }

    /* �������ݵ����� */
    if (transport_sendPacketBuffer(mqtt_buf, send_len) != send_len) {
        printf("����: ���緢��ʧ��\n");
        return MQTT_ERR_NETWORK_FAIL; // 0xE2
    }

    memset(mqtt_buf, 0, MQTT_MAX_BUFF); // ��ջ�����
    delay_ms(2000);
    /* �ȴ�CONNACK��Ӧ������ʱ���ƣ� */
    int timeout = 3000; // ��ʱʱ��3��
    unsigned char sessionPresent, connack_rc = 0xFF;
		printf("\r\n");
    while (timeout > 0) {
      int packet_type = MQTTPacket_read(mqtt_buf, MQTT_MAX_BUFF, transport_getdata);
			for(int i=0;i<64;i++){
			printf("%d ",mqtt_buf[i]);
		}
		    printf("\r\n");
        if (packet_type == CONNACK) {
            /* �����л�CONNACK���� */
            if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, mqtt_buf, MQTT_MAX_BUFF) != 1) {
                printf("����: CONNACK�����л�ʧ��\n");
                return MQTT_ERR_DESERIALIZE_FAIL; // 0xE3
            }
            /* ������ӷ����� */
            if (connack_rc != 0x00) { 
                printf("���ӱ��ܾ���Э�������: 0x%02X\n", connack_rc);
                return MQTT_ERR_CONNACK_BASE + connack_rc; // 0xF0\~0xF5
            }

            /* ���ӳɹ����� */
            printf("MQTT���ӳɹ�!\n");
            UartFlagStC.MQTTReceiveFlag = 0;
            return MQTT_SUCCESS; // 0x00
        }
        else if (packet_type == -1) {
            printf("����: ���Ľ����쳣\n");
            return MQTT_ERR_PACKET_INVALID; // 0xE4
        }
        else if (packet_type == 0) {
            /* �����ݵ������CPUռ�� */
            delay_ms(50);
            timeout -= 50;
        }
        else {
            printf("����: �յ�δ���������� 0x%02X\n", packet_type);
            // ���ڴ˴���QoS 0��PUBLISH����������
        }
    }

    /* ��ʱδ�յ�CONNACK */
    printf("����: �ȴ�CONNACK��ʱ\n");
    return MQTT_ERR_TIMEOUT; // 0xE5
}


void MQTT_Disconnect(void)
{
	u16 send_len=MQTTSerialize_disconnect(mqtt_buf, MQTT_MAX_BUFF);
	transport_sendPacketBuffer(mqtt_buf,send_len);//����������ͶϿ��������ݱ�
	memset(mqtt_buf,0,MQTT_MAX_BUFF);
}

u8 MQTT_Subscribe_Topic(char *sub_topic,int req_qos,int msgid)
{
	 // ����У��
    if (req_qos < 0 || req_qos > 2){
     printf("��������");
			return 1;
		}
	
    MQTTString topicString = MQTTString_initializer;
	  topicString.cstring = sub_topic;
		
    u16 send_len = MQTTSerialize_subscribe(mqtt_buf, MQTT_MAX_BUFF, 0, msgid, 1, &topicString, &req_qos);
    if (send_len == 0){
			printf("���л�ʧ��");
			return 1; // ���л�ʧ��
		}
    if (transport_sendPacketBuffer(mqtt_buf, send_len) != send_len){
			printf("����ʧ��");
			return 1; // ����ʧ��
		}
    memset(mqtt_buf, 0, MQTT_MAX_BUFF);
		delay_ms(1000);
    // ���� SUBACK(����ʱ)
    int timeout = 1000; 
        while (timeout > 0) {
            int packet_type = MQTTPacket_read(mqtt_buf, MQTT_MAX_BUFF, transport_getdata);
					
            if (packet_type == SUBACK) {
							
           
                unsigned short submsgid;
                int subcount, granted_qos;
                u8 rc = MQTTDeserialize_suback(&submsgid, 1, &subcount, &granted_qos, mqtt_buf, MQTT_MAX_BUFF);
							

       
               if (subcount != 1) {
               printf("����������ƥ��!����: %d,ʵ��: %d\n", 1, subcount);
               return 1;
							 }
                if (rc != 1) {
                    printf("�����л�ʧ��!������: %d\n", rc);
                    break;
                }
                if (submsgid != msgid) {
                    printf("��ϢID��ƥ��!����: %d,ʵ��: %d\n", msgid, submsgid);
                    return 1;
                }
                if (granted_qos == 0x80) {
                    printf("�������ⱻ�ܾ�: %s\n", sub_topic);
                    return 1;
                }
								UartFlagStC.MQTTReceiveFlag=0;
                return 0;
            } else if (packet_type == PUBLISH) {
                
                printf("�յ�δԤ�ڵ�PUBLISH����\n");
                timeout -= 100; 
                delay_ms(100);
            } else {
                timeout -= 100;
                delay_ms(100);
            }
        }
    printf("�ȴ�SUBACK��ʱ!\n");
    return 1;
}

u8 MQTT_Unsubscribe_Topic(char *unsub_topic,int msgid)
{
	MQTTString topicString = MQTTString_initializer;
	topicString.cstring = unsub_topic;
	u16 send_len = MQTTSerialize_unsubscribe(mqtt_buf,MQTT_MAX_BUFF, 0, msgid,1, &topicString);//���л�ȡ���������ݱ�
	transport_sendPacketBuffer(mqtt_buf,send_len);//�����������ȡ���������ݱ�
	memset(mqtt_buf,0,MQTT_MAX_BUFF);
	if(MQTTPacket_read(mqtt_buf,MQTT_MAX_BUFF, transport_getdata) == UNSUBACK)//�ж��Ƿ���UNSUBACK����
	{
		unsigned short unsubmsgid;
		u8 rc=MQTTDeserialize_unsuback(&unsubmsgid,mqtt_buf,MQTT_MAX_BUFF);//�����л�UNSUBACK����
		if(!rc || unsubmsgid!=msgid )	return 1;//���ܱ���������ȷ����ȡ������ʧ��
	}
	else return 1;//���ܱ������ʹ���
	return 0;
}

u8 MQTT_Publish(char *pub_topic, unsigned char* payload)
{
	MQTTString topicString = MQTTString_initializer;
	topicString.cstring = pub_topic;
	u16 send_len = MQTTSerialize_publish(mqtt_buf,MQTT_MAX_BUFF,0,0,0,0,topicString, payload, strlen((const char*)payload));//���л��������ݱ�
	transport_sendPacketBuffer(mqtt_buf,send_len);//��������������ݱ�
	memset(mqtt_buf,0,MQTT_MAX_BUFF);
	return 0;
}

u8 MQTT_Heart_Send(void)
{
	u16 send_len=MQTTSerialize_pingreq(mqtt_buf,MQTT_MAX_BUFF);
	transport_sendPacketBuffer(mqtt_buf,send_len);//�����������PING���ݱ�
	memset(mqtt_buf,0,MQTT_MAX_BUFF);
	if(MQTTPacket_read(mqtt_buf,MQTT_MAX_BUFF, transport_getdata) != PINGRESP)//�ж��Ƿ���PINGRESP����
	{
		return 1;//���ܱ������ʹ���
	}

	return 0;
}

//MQTT���Ժ���
uint8_t test(void)
{
		uint32_t len;
		len = MQTTSerialize_pingreq(mqtt_buf, MQTT_MAX_BUFF);
		transport_sendPacketBuffer( mqtt_buf, len);
		  printf("Ping...\r\n");
		if (MQTTPacket_read(mqtt_buf, len, transport_getdata) == PINGRESP){ //����Ƿ�ΪPINGRESP����
			printf("Pong\r\n");
			return 0;
		}
		else {
			printf("OOPS\r\n");
			return 1;
		}
 
}

void MQTT_Client_Task(void) {
    uint8_t reconnectAttempts = 0;
    int8_t connectResult;
    
    while(1) {
        // ��������MQTT
        connectResult = MQTT_Connect(Client_ID, UserName, Password, 300);
        
        if(connectResult == MQTT_SUCCESS) {
            printf("MQTT���ӳɹ�����������...\n");
            
            // ���Ĺؼ�����
            bool sub3_ok = (MQTT_Subscribe_Topic(Sub_Topic3, 0, 1) == 0);
            bool sub4_ok = (MQTT_Subscribe_Topic(Sub_Topic4, 0, 2) == 0);
            
            // ���ĳɹ�����
            if(sub3_ok && sub4_ok) {
                printf("�������ⶩ�ĳɹ�\n");
                reconnectAttempts = 0;  // ���ü�����
                offline_flag = 0;      // ������߱�־
                break;                  // �˳�ѭ������ҵ���߼�
            }
            
            // ����ʧ�ܴ���
            printf("���ⶩ��ʧ�ܣ�Sub3:%d Sub4:%d\n", sub3_ok, sub4_ok);
            MQTT_Disconnect();  // �����Ͽ�����
        }

        // ����/����ʧ�ܴ���
        if(++reconnectAttempts >= MAX_RECONNECT_ATTEMPTS) {
            printf("������������������������߱�־\n");
            printf("����ѭ���ȴ�����ģʽ...\n");
            offline_flag = 1;  // ����ȫ�����߱�־
            break;					// �˳�ѭ��
					printf("offline_flag=%d...\n",offline_flag);
        }
        
        // ָ���˱��ӳ٣�����ͬһ��������
        uint32_t delayMs = RECONNECT_BASE_DELAY_MS * (1 << reconnectAttempts);
        delayMs = (delayMs > 60000) ? 60000 : delayMs;
        printf("%lums���%d������...\n", delayMs, reconnectAttempts);
        delay_ms(delayMs);
    }
}
