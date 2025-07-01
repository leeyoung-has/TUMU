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
    /* 参数合法性检查 */
    if (client_id == NULL || user_name == NULL || password == NULL) {
        printf("错误: 参数存在空指针\n");
        return MQTT_ERR_INVALID_PARAM; // 自定义错误码: 0xE0
    }

    /* 初始化MQTT连接结构体 */
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.clientID.cstring = client_id;
    data.keepAliveInterval = keep_alive;
    data.cleansession =1;
    data.username.cstring = user_name;
    data.password.cstring = password;
    data.MQTTVersion = 3;
    /* 序列化CONNECT报文 */
    u16 send_len = MQTTSerialize_connect(mqtt_buf, MQTT_MAX_BUFF, &data);
    if (send_len == 0) {
        printf("错误: CONNECT报文序列化失败\n");
        return MQTT_ERR_SERIALIZE_FAIL; // 0xE1
    }

    /* 发送数据到网络 */
    if (transport_sendPacketBuffer(mqtt_buf, send_len) != send_len) {
        printf("错误: 网络发送失败\n");
        return MQTT_ERR_NETWORK_FAIL; // 0xE2
    }

    memset(mqtt_buf, 0, MQTT_MAX_BUFF); // 清空缓冲区
    delay_ms(2000);
    /* 等待CONNACK响应（带超时机制） */
    int timeout = 3000; // 超时时间3秒
    unsigned char sessionPresent, connack_rc = 0xFF;
		printf("\r\n");
    while (timeout > 0) {
      int packet_type = MQTTPacket_read(mqtt_buf, MQTT_MAX_BUFF, transport_getdata);
			for(int i=0;i<64;i++){
			printf("%d ",mqtt_buf[i]);
		}
		    printf("\r\n");
        if (packet_type == CONNACK) {
            /* 反序列化CONNACK报文 */
            if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, mqtt_buf, MQTT_MAX_BUFF) != 1) {
                printf("错误: CONNACK反序列化失败\n");
                return MQTT_ERR_DESERIALIZE_FAIL; // 0xE3
            }
            /* 检查连接返回码 */
            if (connack_rc != 0x00) { 
                printf("连接被拒绝，协议错误码: 0x%02X\n", connack_rc);
                return MQTT_ERR_CONNACK_BASE + connack_rc; // 0xF0\~0xF5
            }

            /* 连接成功处理 */
            printf("MQTT连接成功!\n");
            UartFlagStC.MQTTReceiveFlag = 0;
            return MQTT_SUCCESS; // 0x00
        }
        else if (packet_type == -1) {
            printf("错误: 报文解析异常\n");
            return MQTT_ERR_PACKET_INVALID; // 0xE4
        }
        else if (packet_type == 0) {
            /* 无数据到达，降低CPU占用 */
            delay_ms(50);
            timeout -= 50;
        }
        else {
            printf("警告: 收到未处理报文类型 0x%02X\n", packet_type);
            // 可在此处理QoS 0的PUBLISH或其他报文
        }
    }

    /* 超时未收到CONNACK */
    printf("错误: 等待CONNACK超时\n");
    return MQTT_ERR_TIMEOUT; // 0xE5
}


void MQTT_Disconnect(void)
{
	u16 send_len=MQTTSerialize_disconnect(mqtt_buf, MQTT_MAX_BUFF);
	transport_sendPacketBuffer(mqtt_buf,send_len);//向服务器发送断开连接数据报
	memset(mqtt_buf,0,MQTT_MAX_BUFF);
}

u8 MQTT_Subscribe_Topic(char *sub_topic,int req_qos,int msgid)
{
	 // 参数校验
    if (req_qos < 0 || req_qos > 2){
     printf("参数错误");
			return 1;
		}
	
    MQTTString topicString = MQTTString_initializer;
	  topicString.cstring = sub_topic;
		
    u16 send_len = MQTTSerialize_subscribe(mqtt_buf, MQTT_MAX_BUFF, 0, msgid, 1, &topicString, &req_qos);
    if (send_len == 0){
			printf("序列化失败");
			return 1; // 序列化失败
		}
    if (transport_sendPacketBuffer(mqtt_buf, send_len) != send_len){
			printf("发送失败");
			return 1; // 发送失败
		}
    memset(mqtt_buf, 0, MQTT_MAX_BUFF);
		delay_ms(1000);
    // 接受 SUBACK(带超时)
    int timeout = 1000; 
        while (timeout > 0) {
            int packet_type = MQTTPacket_read(mqtt_buf, MQTT_MAX_BUFF, transport_getdata);
					
            if (packet_type == SUBACK) {
							
           
                unsigned short submsgid;
                int subcount, granted_qos;
                u8 rc = MQTTDeserialize_suback(&submsgid, 1, &subcount, &granted_qos, mqtt_buf, MQTT_MAX_BUFF);
							

       
               if (subcount != 1) {
               printf("订阅数量不匹配!期望: %d,实际: %d\n", 1, subcount);
               return 1;
							 }
                if (rc != 1) {
                    printf("反序列化失败!错误码: %d\n", rc);
                    break;
                }
                if (submsgid != msgid) {
                    printf("消息ID不匹配!期望: %d,实际: %d\n", msgid, submsgid);
                    return 1;
                }
                if (granted_qos == 0x80) {
                    printf("订阅主题被拒绝: %s\n", sub_topic);
                    return 1;
                }
								UartFlagStC.MQTTReceiveFlag=0;
                return 0;
            } else if (packet_type == PUBLISH) {
                
                printf("收到未预期的PUBLISH报文\n");
                timeout -= 100; 
                delay_ms(100);
            } else {
                timeout -= 100;
                delay_ms(100);
            }
        }
    printf("等待SUBACK超时!\n");
    return 1;
}

u8 MQTT_Unsubscribe_Topic(char *unsub_topic,int msgid)
{
	MQTTString topicString = MQTTString_initializer;
	topicString.cstring = unsub_topic;
	u16 send_len = MQTTSerialize_unsubscribe(mqtt_buf,MQTT_MAX_BUFF, 0, msgid,1, &topicString);//序列化取消订阅数据报
	transport_sendPacketBuffer(mqtt_buf,send_len);//向服务器发送取消订阅数据报
	memset(mqtt_buf,0,MQTT_MAX_BUFF);
	if(MQTTPacket_read(mqtt_buf,MQTT_MAX_BUFF, transport_getdata) == UNSUBACK)//判断是否是UNSUBACK报文
	{
		unsigned short unsubmsgid;
		u8 rc=MQTTDeserialize_unsuback(&unsubmsgid,mqtt_buf,MQTT_MAX_BUFF);//反序列化UNSUBACK报文
		if(!rc || unsubmsgid!=msgid )	return 1;//接受报文类型正确，但取消订阅失败
	}
	else return 1;//接受报文类型错误
	return 0;
}

u8 MQTT_Publish(char *pub_topic, unsigned char* payload)
{
	MQTTString topicString = MQTTString_initializer;
	topicString.cstring = pub_topic;
	u16 send_len = MQTTSerialize_publish(mqtt_buf,MQTT_MAX_BUFF,0,0,0,0,topicString, payload, strlen((const char*)payload));//序列化发布数据报
	transport_sendPacketBuffer(mqtt_buf,send_len);//向服务器发送数据报
	memset(mqtt_buf,0,MQTT_MAX_BUFF);
	return 0;
}

u8 MQTT_Heart_Send(void)
{
	u16 send_len=MQTTSerialize_pingreq(mqtt_buf,MQTT_MAX_BUFF);
	transport_sendPacketBuffer(mqtt_buf,send_len);//向服务器发送PING数据报
	memset(mqtt_buf,0,MQTT_MAX_BUFF);
	if(MQTTPacket_read(mqtt_buf,MQTT_MAX_BUFF, transport_getdata) != PINGRESP)//判断是否是PINGRESP报文
	{
		return 1;//接受报文类型错误
	}

	return 0;
}

//MQTT测试函数
uint8_t test(void)
{
		uint32_t len;
		len = MQTTSerialize_pingreq(mqtt_buf, MQTT_MAX_BUFF);
		transport_sendPacketBuffer( mqtt_buf, len);
		  printf("Ping...\r\n");
		if (MQTTPacket_read(mqtt_buf, len, transport_getdata) == PINGRESP){ //检查是否为PINGRESP报文
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
        // 尝试连接MQTT
        connectResult = MQTT_Connect(Client_ID, UserName, Password, 300);
        
        if(connectResult == MQTT_SUCCESS) {
            printf("MQTT连接成功，订阅主题...\n");
            
            // 订阅关键主题
            bool sub3_ok = (MQTT_Subscribe_Topic(Sub_Topic3, 0, 1) == 0);
            bool sub4_ok = (MQTT_Subscribe_Topic(Sub_Topic4, 0, 2) == 0);
            
            // 订阅成功处理
            if(sub3_ok && sub4_ok) {
                printf("所有主题订阅成功\n");
                reconnectAttempts = 0;  // 重置计数器
                offline_flag = 0;      // 清除下线标志
                break;                  // 退出循环进入业务逻辑
            }
            
            // 订阅失败处理
            printf("主题订阅失败：Sub3:%d Sub4:%d\n", sub3_ok, sub4_ok);
            MQTT_Disconnect();  // 主动断开连接
        }

        // 连接/订阅失败处理
        if(++reconnectAttempts >= MAX_RECONNECT_ATTEMPTS) {
            printf("超过最大重连次数！设置下线标志\n");
            printf("进入循环等待重连模式...\n");
            offline_flag = 1;  // 设置全局下线标志
            break;					// 退出循环
					printf("offline_flag=%d...\n",offline_flag);
        }
        
        // 指数退避延迟（复用同一计数器）
        uint32_t delayMs = RECONNECT_BASE_DELAY_MS * (1 << reconnectAttempts);
        delayMs = (delayMs > 60000) ? 60000 : delayMs;
        printf("%lums后第%d次重试...\n", delayMs, reconnectAttempts);
        delay_ms(delayMs);
    }
}
