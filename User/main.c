#include "stm32f4xx.h"
#include "LED.h"
#include "delay.h"
#include "usart.h"
#include "stdio.h"
#include "string.h"
#include "mqtt.h"
#include "stdlib.h"
#include "time.h"
#include "Frq.h"
#include "parse_data.h"
#include "stm32f4xx_rtc.h"
#include "stm32f4xx_pwr.h"
#include "stm32f4xx_rcc.h"
#include "MQTTPacket.h" 
#include "transport.h"
#include "DS3231.h"
#include "softwareIIC.h"
#include "sensor.h"

#define BATCH_NO_MAX_LEN 7
#define SORT_MAX_LEN 5

int time_flag=0; //ʱ���־λ
int heartbeat=0; //��������־λ
volatile uint32_t up_data1_ready=0;//һ��һ�����ϴ���ʶλ
volatile uint32_t up_data2_ready=0;//һ�Զ������ϴ���ʶλ
volatile uint32_t newday=0;//�賿���±�ʶλ
volatile uint32_t next_valid=0; //У���ʶλ
int MQTT_Receive=0;  //���ܱ�������
volatile uint32_t reconnect_flag=0;//������־

const char *deviceCode1="CJY05";  //һ��һ�豸��ʶ
const char *deviceCode2="SD006";//һ�Զ��豸��ʶ

char  deviceStatus[16]="0";
char  batteryLevel[16]="0"; //�豸״̬����

char sort[SORT_MAX_LEN]="1";  //һ�Զ���˳���

char batchNo1_a[BATCH_NO_MAX_LEN] = "1";  //�������κ�
char batchNo2_a[BATCH_NO_MAX_LEN] = "1";  

char batchNo1_b[BATCH_NO_MAX_LEN] = "1"; 
char batchNo2_b[BATCH_NO_MAX_LEN] = "1";//�Ӳ����κ�

char currValue1[10] = "16";   //��ֵ
char currValue2[10] = "14";

char collectTime[20];

const char *measureMode1="1";//����ģʽ����
const char *measureMode2="2";



DateTime current;	
DateTime next1;
DateTime next2;
DateTime next_day;
DateTime next_validation;

char Pub_buff[512];  //����������
char buf[512] = {0};  //���ܻ�����

static ImtMessage imt = {0}; //�Ӳ����ݽṹ��
static FrqMessage frq = {0}; //��Ƶ���ݽṹ��


int current_num=0;

char timestamp[20]; //ʱ���ַ���ת������

int current_frq1=3;//��ǰ�����ϴ�Ƶ��(times per day)
int current_frq2=6;//��ǰ�����ϴ�Ƶ��(times per day);

int next_frq1=0;   //����Ƶ��
int next_frq2=0;

/*һ��һ����*/
float current_force;

/*һ�Զ�����*/
AngleData angles[8];
uint8_t valid_points = 0;
SensorStatus status;

/*��ص�������*/
 SensorStatus result;
 uint8_t soc = 0;
 
 float test_data=0;
 
int main(void)
{
	//Ӳ����ʼ��
	LED_Init();
	LED0_ON();	 
  uart1_init(115200);
	uart2_init(115200);
	uart3_init(9600);
	TIM4_Delay_Init();
	TIM3_Init();
	TIM2_Init();
	TIM5_Init();
	DS3231_Init();
	//�Ͽ�֮ǰ������
	MQTT_Disconnect();
	UartFlagStC.MQTTReceiveFlag=0;
	UartFlagStC.UART2Counter=0;
	memset(buf,0,512);
	delay_ms(1000);
	//����mqtt������
  MQTT_Client_Task();
//MQTT_Connect(Client_ID, UserName, Password, 300);
//	delay_ms(1000);
//	if(MQTT_Subscribe_Topic(Sub_Topic3, 0, 1) == 0)
//  printf("�Ӳ����ⶩ�ĳɹ�\n");
//  if(MQTT_Subscribe_Topic(Sub_Topic4, 0, 2) == 0)
//  printf("��Ƶ���ⶩ�ĳɹ�\n");
	UartFlagStC.MQTTReceiveFlag=0;
	UartFlagStC.UART2Counter=0;
	memset(buf,0,512);
   //��ʼ��ʱ��
   //DS3231_setDate(2025,5,12);	
   //DS3231_setTime(9,36,00);
   
	DS3231_gettime(&current);
	DS3231_getdate(&current);
	DateTime_ToString(&current, timestamp);
	printf("��ǰʱ��: %s\n", timestamp);
	next1=calculate_next_collect(current_frq1,&current);
	DateTime_ToString(&next1, timestamp);
	printf("�´�һ��һ�����ϴ�ʱ��: %s\n", timestamp);
	next2=calculate_next_collect(current_frq2,&current);
	DateTime_ToString(&next2, timestamp);
	printf("�´�һ�Զ������ϴ�ʱ�䣺%s\n",timestamp);
	next_day=calculate_next_day(&current);
	DateTime_ToString(&next_day, timestamp);
	printf("�´θ���Ƶ�ʺ����ε�ʱ�䣺%s\n",timestamp);
	next_validation=calculate_next_night_validation(&current);
	DateTime_ToString(&next_validation,timestamp);
	printf("�´�У������ʱ�䣺%s\n",timestamp);
	
	 //��ȡ�豸״̬
	 DS3231_gettime(&current);
	 DS3231_getdate(&current);
	 DateTime_ToString(&current, collectTime);
     result=GetBatterySOC(&soc);
     if(result == SENSOR_OK){
      snprintf(batteryLevel, sizeof(batteryLevel), "%d", soc);
	 }
     else{
      snprintf(deviceStatus, sizeof(deviceStatus), "%d",1);
		  snprintf(batteryLevel, sizeof(batteryLevel), "%d",0);
	 }		 
	 snprintf(
        Pub_buff, sizeof(Pub_buff),
        "{\"deviceCode\":\"%s\",\"deviceStatus\":\"%s\",\"batteryLevel\":%s,\"statusTime\":\"%s\"}",
        deviceCode1, deviceStatus, batteryLevel, collectTime
        );		
	 MQTT_Publish(Pub_Topic4,(unsigned char*)Pub_buff);
	 printf("�豸״̬�ϴ��ɹ�\r\n"); 
			
				
	//������ѭ��
	 while(1)
	{
		if(UartFlagStC.MQTTReceiveFlag){
			printf("��⵽����\n");
	    MQTT_Receive=MQTTPacket_read(buf, MQTT_MAX_BUFF, transport_getdata);
			 printf("MQTT_Receive = %d\n", MQTT_Receive);
			for(int i=0;i<64;i++)
			{
			 printf("%d ",buf[i]);
			} 
			switch(MQTT_Receive){				
				case PUBLISH:{
					int ret = parse_message(buf,MQTT_MAX_BUFF,&frq,&imt);

			//��⵽Ƶ�ʸ�������
		      if(ret == 1) {
						  printf("��⵽Ƶ�ʸ�������\r\n");
					 if(strcmp(frq.deviceCode,deviceCode1)==0){
		       if(is_valid_frequency(frq.currentFrq)){
					   printf("һ��һԤ��Ƶ�ɹ�,Ƶ��Ϊ��%d\r\n",frq.currentFrq);			  
					   next_frq1=frq.currentFrq;
				  }
				 else 
					 printf("ERR_INVALID_FRQ");	
			  }
   			if(strcmp(frq.deviceCode,deviceCode2)==0){
             if(is_valid_frequency(frq.currentFrq)){
					   printf("һ�Զ�Ԥ��Ƶ�ɹ�,Ƶ��Ϊ��%d\r\n",frq.currentFrq);			  
					   next_frq2=frq.currentFrq;
				}
        	else	
            printf("ERR_INVALID_FRQ");						
			}
		} 
	
		//��⵽�Ӳ�����
		   else if((ret ==0)&&(imt.measureMode==2)){
			
			 printf("��⵽�Ӳ�����,measureMode:%d\n",imt.measureMode);
       printf("timestamp:%llu\r\n",imt.timestamp);
	     if (timestamp_to_datetime(imt.timestamp, &current))
			 {
				 DateTime_ToString(&current, collectTime);
			 }
				 
			 //һ��һ�Ӳ�
			 if(strcmp(imt.deviceCode,deviceCode1)==0)
			 {
			 snprintf(batchNo1_b,BATCH_NO_MAX_LEN,"%d",imt.batchNo); 
			 current_force = Get_Channel2_Data();
			 
       if(current_force>=0){
          snprintf(currValue1, sizeof(currValue1), "%.2f", current_force);//%.2f
			 }
       else{
          strncpy(currValue1, "25.0", sizeof(currValue1));
				}					
			 snprintf(
        Pub_buff, sizeof(Pub_buff),
        "{\"deviceCode\":\"%s\",\"batchNo\":%s,\"currValue\":%s,\"collectTime\":\"%s\",\"measureMode\":\"%s\"}",
        deviceCode1, batchNo1_b, currValue1, collectTime, measureMode2
        );				
	      MQTT_Publish(Pub_Topic1,(unsigned char*)Pub_buff);
				printf("һ��һ�Ӳⷢ���ɹ�\n");
		   }

		   //һ�Զ�Ӳ�
			 if(strcmp(imt.deviceCode,deviceCode2)==0)
			 {
				 
				snprintf(batchNo2_b,BATCH_NO_MAX_LEN,"%d",imt.batchNo);
				status=GetAngleData(angles,&valid_points);
				if(status==SENSOR_OK){
				  for(int i=1;i<=8;i++){
					 snprintf(currValue2, sizeof(currValue2), "%7.2f", angles[i-1].yaw);//%7.2f
			         snprintf(sort,SORT_MAX_LEN,"%d",i);
                     snprintf(Pub_buff, sizeof(Pub_buff),
				     "{\"deviceCode\":\"%s\",\"batchNo\":%s,\"currValue\":%s,\"sort\":%s,\"collectTime\":\"%s\",\"measureMode\":\"%s\"}",
                     deviceCode2, batchNo2_b, currValue2,sort,collectTime, measureMode2
                    );				
	                MQTT_Publish(Pub_Topic2,(unsigned char*)Pub_buff);
			         delay_ms(50);	
			     } 
				  printf("һ�Զ�Ӳⷢ���ɹ�\n");
				 }
        else{
			for(int i=1;i<=8;i++){
				snprintf(currValue2, sizeof(currValue2), "%7.2f", 18.0);
				snprintf(sort,SORT_MAX_LEN,"%d",i);
				snprintf(Pub_buff, sizeof(Pub_buff),
				"{\"deviceCode\":\"%s\",\"batchNo\":%s,\"currValue\":%s,\"sort\":%s,\"collectTime\":\"%s\",\"measureMode\":\"%s\"}",
				deviceCode2, batchNo2_b, currValue2,sort,collectTime, measureMode2
			   );				
			   MQTT_Publish(Pub_Topic2,(unsigned char*)Pub_buff);
				delay_ms(50);	
			} 
				printf("INVALID DATA!\r\n");
				}
         					
			 }
		 }
		    break;
	  }
			  case PINGRESP:{
					printf("��⵽������������\r\n");
					break;
				}
			  case CONNACK:{
					printf("MQTT���ӳɹ�\r\n");
					break;
				}
			  case SUBACK:{
             printf("���ⶩ�ĳɹ�\r\n");		        
					   break;
				}					        
				default:{
          printf("δ֪�����MQTT�����ͣ���������...\r\n");
					UartFlagStC.MQTTReceiveFlag = 0;
                    UartFlagStC.UART2Counter = 0;
                    memset(buf,0,512);
					MQTT_Client_Task();	        
					break;
				}					
			}
			UartFlagStC.MQTTReceiveFlag = 0;
      UartFlagStC.UART2Counter = 0;
      memset(buf,0,512);
		}

		if( newday==1){
			DS3231_gettime(&current);
	        DS3231_getdate(&current);
			next_day=calculate_next_day(&current);
			if(next_frq1==0){
				next_frq1=current_frq1;
			}
			if(next_frq2==0){
				next_frq2=current_frq2; 
			}
			current_frq1=next_frq1;
			current_frq2=next_frq2;
			//������һ���״������ϴ�ʱ��
			DS3231_gettime(&current);
	        DS3231_getdate(&current);
			//������һ��һ��һ�����ϴ�ʱ��
            next1 = calculate_next_collect(current_frq1, &current);
            DateTime_ToString(&next1, timestamp);
	    printf("�´�һ��һ�����ϴ�ʱ��: %s\n", timestamp);
			//������һ��һ�Զ������ϴ�ʱ��
		  next2 = calculate_next_collect(current_frq2, &current);
      DateTime_ToString(&next2, timestamp);
	    printf("�´�һ�Զ������ϴ�ʱ��: %s\n", timestamp);
      //�������κ�
			strcpy(batchNo1_a, "1");
			strcpy(batchNo2_a, "1");
			printf("current_frq1=%d,current_frq2=%d\r\n",current_frq1,current_frq2);
			printf("batchNo1_a=%s,batchNo2_a=%s\r\n",batchNo1_a,batchNo2_a);
            newday=0;
			up_data1_ready=0;
			up_data2_ready=0;

     result=GetBatterySOC(&soc);
     if(result == SENSOR_OK){
      snprintf(batteryLevel, sizeof(batteryLevel), "%d", soc);
	   }
     else{
      snprintf(deviceStatus, sizeof(deviceStatus), "%d",0);
		  snprintf(batteryLevel, sizeof(batteryLevel), "%d",0);
	   }		 
	 snprintf(
        Pub_buff, sizeof(Pub_buff),
        "{\"deviceCode\":\"%s\",\"deviceStatus\":\"%s\",\"batteryLevel\":%s,\"statusTime\":\"%s\"}",
        deviceCode1, deviceStatus, batteryLevel, collectTime
        );		
	 MQTT_Publish(Pub_Topic4,(unsigned char*)Pub_buff);
	 printf("�豸״̬�ϴ��ɹ�\r\n");
		UartFlagStC.MQTTReceiveFlag=0;
	  UartFlagStC.UART2Counter=0;
	  memset(buf,0,512);
		}

			//һ��һ�����ϴ�
	  if(up_data1_ready==1){
        DS3231_gettime(&current);
	    DS3231_getdate(&current);
			//������һ��һ��һ�����ϴ�ʱ��
        next1 = calculate_next_collect(current_frq1, &current);
        DateTime_ToString(&next1, timestamp);
	      printf("�´�һ��һ�����ϴ�ʱ��: %s\n", timestamp);
        // ִ�����ݲɼ����ϴ�	 
		    DateTime_ToString(&current, collectTime);	
			  current_force = Get_Channel2_Data();
        if(current_force>=0){
          snprintf(currValue1, sizeof(currValue1), "%.2f", current_force);
			  }
        else{
          strncpy(currValue1, "10.0", sizeof(currValue1));
				}					
			  snprintf(
        Pub_buff, sizeof(Pub_buff),
        "{\"deviceCode\":\"%s\",\"batchNo\":\"%s\",\"currValue\":%s,\"collectTime\":\"%s\",\"measureMode\":%s}",
        deviceCode1, batchNo1_a, currValue1, collectTime, measureMode1
        );
	      MQTT_Publish(Pub_Topic1,(unsigned char*)Pub_buff);
			current_num = atoi(batchNo1_a);
            current_num++;
			snprintf(batchNo1_a, BATCH_NO_MAX_LEN, "%d", current_num);
			printf("һ��һ��ʱ�����Ѿ��ϱ�\r\n");
			up_data1_ready=0;
			UartFlagStC.MQTTReceiveFlag=0;
	    UartFlagStC.UART2Counter=0;
	    memset(buf,0,512);
      }

		if(up_data2_ready==1){
			DS3231_gettime(&current);
	        DS3231_getdate(&current);
			  //������һ��һ�Զ������ϴ�ʱ��
            next2 = calculate_next_collect(current_frq2, &current);
            DateTime_ToString(&next2, timestamp);
	        printf("�´�һ�Զ������ϴ�ʱ��: %s\n", timestamp);
            // ִ�����ݲɼ����ϴ�	 
		    DateTime_ToString(&current, collectTime);
			status=GetAngleData(angles,&valid_points);
			if(status==SENSOR_OK){
			    for(int i=1;i<=8;i++){
				  snprintf(currValue2, sizeof(currValue2), "%7.2f", angles[i-1].yaw);
			      snprintf(sort,SORT_MAX_LEN,"%d",i);
                  snprintf(Pub_buff, sizeof(Pub_buff),
				    "{\"deviceCode\":\"%s\",\"batchNo\":%s,\"currValue\":%s,\"sort\":%s,\"collectTime\":\"%s\",\"measureMode\":\"%s\"}",
                  deviceCode2, batchNo2_a, currValue2,sort,collectTime, measureMode1
            );	
            MQTT_Publish(Pub_Topic2,(unsigned char*)Pub_buff);
            delay_ms(50);					
			  }
				  current_num = atoi(batchNo2_a);
                  current_num++;
				  snprintf(batchNo2_a, BATCH_NO_MAX_LEN, "%d", current_num);
			    printf("һ�Զఴʱ�����Ѿ��ϱ�\r\n");
				  up_data2_ready=0;
			}
    else{
		for(int i=1;i<=8;i++){
			snprintf(currValue2, sizeof(currValue2), "%7.2f", 0.0);//angles[i-1].yaw
			snprintf(sort,SORT_MAX_LEN,"%d",i);
			snprintf(Pub_buff, sizeof(Pub_buff),
			  "{\"deviceCode\":\"%s\",\"batchNo\":%s,\"currValue\":%s,\"sort\":%s,\"collectTime\":\"%s\",\"measureMode\":\"%s\"}",
			deviceCode2, batchNo2_a, currValue2,sort,collectTime, measureMode1
	  );	
	  MQTT_Publish(Pub_Topic2,(unsigned char*)Pub_buff);
	  delay_ms(50);					
		}
			current_num = atoi(batchNo2_a);
			current_num++;
			snprintf(batchNo2_a, BATCH_NO_MAX_LEN, "%d", current_num);
			up_data2_ready=0;
            printf("INVALID DATA!\r\n");
		}
		UartFlagStC.MQTTReceiveFlag=0;
	  UartFlagStC.UART2Counter=0;
	  memset(buf,0,512);
	}

    if(heartbeat==1){
		uint32_t len;
		len = MQTTSerialize_pingreq(mqtt_buf, MQTT_MAX_BUFF);
		transport_sendPacketBuffer( mqtt_buf, len);       // ����������
		printf("���������ͳɹ�\r\n");
		delay_ms(1000);
		if (MQTTPacket_read(buf, MQTT_MAX_BUFF, transport_getdata)== PINGRESP)
		{
			printf("��⵽������������\r\n");
		}
		else{
			for(int i=0;i<64;i++)
			{
			 printf("%d ",buf[i]);
			} 
			printf("ERR_NO_PINGRESP,���ڳ�������\r\n");
			
			MQTT_Client_Task();
		}
      heartbeat=0;
		  UartFlagStC.MQTTReceiveFlag = 0;
      UartFlagStC.UART2Counter = 0;
      memset(buf,0,512);
	 }

    if(next_valid==1){
	    //У��һ��һ
	    current_num=atoi(batchNo1_a)-1;
        if(current_num<current_frq1)
		{
			DS3231_gettime(&current);
			DS3231_getdate(&current);
				//������һ��һ��һ�����ϴ�ʱ��
			next1 = calculate_next_collect(current_frq1, &current);
			DateTime_ToString(&next1, timestamp);
			  printf("�´�һ��һ�����ϴ�ʱ��: %s\n", timestamp);
			// ִ�����ݲɼ����ϴ�	 
				DateTime_ToString(&current, collectTime);	
				  current_force = Get_Channel2_Data();
			if(current_force>=0){
			  snprintf(currValue1, sizeof(currValue1), "%.2f", current_force);
				  }
			else{
			  strncpy(currValue1, "10.0", sizeof(currValue1));
					}					
				  snprintf(
			Pub_buff, sizeof(Pub_buff),
			"{\"deviceCode\":\"%s\",\"batchNo\":\"%s\",\"currValue\":%s,\"collectTime\":\"%s\",\"measureMode\":%s}",
			deviceCode1, batchNo1_a, currValue1, collectTime, measureMode1
			);
			  MQTT_Publish(Pub_Topic1,(unsigned char*)Pub_buff);
				current_num = atoi(batchNo1_a);
			    current_num++;
			    snprintf(batchNo1_a, BATCH_NO_MAX_LEN, "%d", current_num);
				printf("һ��һ��ʱ�����Ѿ��ϱ�\r\n");
        } 
		//У��һ�Զ�
		current_num=atoi(batchNo2_a)-1;
        if(current_num<current_frq2)
		{
			DS3231_gettime(&current);
	        DS3231_getdate(&current);
			  //������һ��һ�Զ������ϴ�ʱ��
            next2 = calculate_next_collect(current_frq2, &current);
            DateTime_ToString(&next2, timestamp);
	        printf("�´�һ�Զ������ϴ�ʱ��: %s\n", timestamp);
            // ִ�����ݲɼ����ϴ�	 
		    DateTime_ToString(&current, collectTime);
			status=GetAngleData(angles,&valid_points);
			if(status==SENSOR_OK){
			    for(int i=1;i<=8;i++){
				  snprintf(currValue2, sizeof(currValue2), "%7.2f", angles[i-1].yaw);
			      snprintf(sort,SORT_MAX_LEN,"%d",i);
                  snprintf(Pub_buff, sizeof(Pub_buff),
				    "{\"deviceCode\":\"%s\",\"batchNo\":%s,\"currValue\":%s,\"sort\":%s,\"collectTime\":\"%s\",\"measureMode\":\"%s\"}",
                  deviceCode2, batchNo2_a, currValue2,sort,collectTime, measureMode1
            );	
            MQTT_Publish(Pub_Topic2,(unsigned char*)Pub_buff);
            delay_ms(50);					
			  }
				  current_num = atoi(batchNo2_a);
                  current_num++;
				  snprintf(batchNo2_a, BATCH_NO_MAX_LEN, "%d", current_num);
			      printf("һ�Զఴʱ�����Ѿ��ϱ�\r\n");
				  up_data2_ready=0;
			}
    else{
		for(int i=1;i<=8;i++){
			snprintf(currValue2, sizeof(currValue2), "%7.2f", 0.0);//angles[i-1].yaw
			snprintf(sort,SORT_MAX_LEN,"%d",i);
			snprintf(Pub_buff, sizeof(Pub_buff),
			  "{\"deviceCode\":\"%s\",\"batchNo\":%s,\"currValue\":%s,\"sort\":%s,\"collectTime\":\"%s\",\"measureMode\":\"%s\"}",
			deviceCode2, batchNo2_a, currValue2,sort,collectTime, measureMode1
	  );	
	  MQTT_Publish(Pub_Topic2,(unsigned char*)Pub_buff);
	  delay_ms(50);					
		}
		current_num = atoi(batchNo2_a);
		current_num++;
		snprintf(batchNo2_a, BATCH_NO_MAX_LEN, "%d", current_num);
		up_data2_ready=0;
        printf("INVALID DATA!\r\n");
		}
	} 
	    DS3231_getdate(&current);
		DS3231_gettime(&current);
		next_validation=calculate_next_night_validation(&current);
		DateTime_ToString(&next_validation,timestamp);
		printf("�´�У������ʱ�䣺%s\n",timestamp);
		next_valid=0;
		UartFlagStC.MQTTReceiveFlag=0;
	  UartFlagStC.UART2Counter=0;
	  memset(buf,0,512);
  }
		
		//��������
	if(reconnect_flag==1)
	{
		MQTT_Disconnect();
	  UartFlagStC.MQTTReceiveFlag=0;
	  UartFlagStC.UART2Counter=0;
	  memset(buf,0,512);
	  delay_ms(1000);
		if(MQTT_Connect(Client_ID, UserName, Password, 300)==MQTT_SUCCESS){
			printf("MQTT�������ӳɹ�����������...\n");
			delay_ms(500);
			bool sub3_ok = (MQTT_Subscribe_Topic(Sub_Topic3, 0, 1) == 0);
      bool sub4_ok = (MQTT_Subscribe_Topic(Sub_Topic4, 0, 2) == 0);
            if(sub3_ok && sub4_ok){
                printf("���ĳɹ�\n");
		        offline_flag=0;
            }
	    }
		 UartFlagStC.MQTTReceiveFlag=0;
	   UartFlagStC.UART2Counter=0;
	   memset(buf,0,512);
		 reconnect_flag=0;
	}
	if(time_flag==20){
			DateTime_ToString(&current, timestamp);
			printf("��ǰʱ��: %s\n", timestamp);
			DS3231_gettime(&current);
	    DS3231_getdate(&current);
	    DateTime_ToString(&current, collectTime);
      result=GetBatterySOC(&soc);
      if(result == SENSOR_OK){
				snprintf(deviceStatus, sizeof(deviceStatus), "%d",0);
      snprintf(batteryLevel, sizeof(batteryLevel), "%d", soc);
	 }
     else{
      snprintf(deviceStatus, sizeof(deviceStatus), "%d",1);
		  snprintf(batteryLevel, sizeof(batteryLevel), "%d",0);
	 }		 
	 snprintf(
        Pub_buff, sizeof(Pub_buff),
        "{\"deviceCode\":\"%s\",\"deviceStatus\":\"%s\",\"batteryLevel\":%s,\"statusTime\":\"%s\"}",
        deviceCode1, deviceStatus, batteryLevel, collectTime
        );		
	 MQTT_Publish(Pub_Topic4,(unsigned char*)Pub_buff);
	 printf("�豸״̬�ϴ��ɹ�\r\n");
			if(offline_flag==1){
			  reconnect_flag=1;
			}
			time_flag=0;
			UartFlagStC.MQTTReceiveFlag=0;
	    UartFlagStC.UART2Counter=0;
	    memset(buf,0,512);
		}
	  delay_ms(100);
  }
	
}

//��ʱ�ϴ����ݴ�����
void TIM5_IRQHandler(void){
	 if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM5, TIM_IT_Update); // ����жϱ�ʶλ
		time_flag++;
		DS3231_gettime(&current);
	    DS3231_getdate(&current);

//		if(time_flag==4){
//			DateTime_ToString(&current, timestamp);
//			printf("��ǰʱ��: %s\n", timestamp);
//			DS3231_gettime(&current);
//	    DS3231_getdate(&current);
//	    DateTime_ToString(&current, collectTime);
//      result=GetBatterySOC(&soc);
//      if(result == SENSOR_OK){
//      snprintf(batteryLevel, sizeof(batteryLevel), "%d", soc);
//	 }
//     else{
//      snprintf(deviceStatus, sizeof(deviceStatus), "%d",1);
//		  snprintf(batteryLevel, sizeof(batteryLevel), "%d",0);
//	 }		 
//	 snprintf(
//        Pub_buff, sizeof(Pub_buff),
//        "{\"deviceCode\":\"%s\",\"deviceStatus\":\"%s\",\"batteryLevel\":%s,\"statusTime\":\"%s\"}",
//        deviceCode1, deviceStatus, batteryLevel, collectTime
//        );		
//	 MQTT_Publish(Pub_Topic4,(unsigned char*)Pub_buff);
//	 printf("�豸״̬�ϴ��ɹ�\r\n");
//			if(offline_flag==1){
//			  reconnect_flag=1;
//			}
//			time_flag=0;
//		}
		if(current.year == next1.year &&
        current.month == next1.month &&
        current.day == next1.day &&
        current.hours == next1.hours&&
		current.minutes==next1.minutes) 
      {
		  up_data1_ready=1;
          printf("get time1!!!\r\n");				
			}
		 if(current.year == next2.year &&
        current.month == next2.month &&
        current.day == next2.day &&
        current.hours == next2.hours
		&&current.minutes==next2.minutes) 
      {
		  up_data2_ready=1;
          printf("get time2!!!\r\n");				
			}
     if(current.year == next_day.year &&
        current.month == next_day.month &&
        current.day == next_day.day &&
        current.hours == next_day.hours) 
      {
				   newday=1;
				printf("get new day!!!\r\n");
      }
	   if(current.year == next_validation.year 
		&&current.month == next_validation.month
		 &&current.day == next_validation.day 
		 &&current.hours == next_validation.hours
		 &&current.minutes == next_validation.minutes)
		 {
			next_valid=1;
			printf("get next validation!!!\r\n");
		 }	              
  }
}

// �����������жϺ���
void TIM2_IRQHandler(void) {
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
         // 300�봥�� 
			if(offline_flag==0){				
		     heartbeat=1;
			}
    }
}



//�Զ�������ݼӲ��·�{"deviceCode":"GJO1","batchNo":"1","measureMode":"2","timestamp":"1734679545903"}                                                                                                                                            
//�Զ��������Ƶ�ʱ��{"deviceCode":"GJO1","currentFrq":"4","frqType":"1","timestamp":"1734679545903"}
//{"deviceCode":"GJO1","batchNo":"1","currValue":"3",collectTime:"2024-07-24 09:06:16","measureMode":"2"};
		
		
				