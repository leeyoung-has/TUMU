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

int time_flag=0; //时间标志位
int heartbeat=0; //心跳包标志位
volatile uint32_t up_data1_ready=0;//一对一数据上传标识位
volatile uint32_t up_data2_ready=0;//一对多数据上传标识位
volatile uint32_t newday=0;//凌晨更新标识位
volatile uint32_t next_valid=0; //校验标识位
int MQTT_Receive=0;  //接受报文类型
volatile uint32_t reconnect_flag=0;//重连标志

const char *deviceCode1="CJY05";  //一对一设备标识
const char *deviceCode2="SD006";//一对多设备标识

char  deviceStatus[16]="0";
char  batteryLevel[16]="0"; //设备状态参数

char sort[SORT_MAX_LEN]="1";  //一对多测点顺序号

char batchNo1_a[BATCH_NO_MAX_LEN] = "1";  //正常批次号
char batchNo2_a[BATCH_NO_MAX_LEN] = "1";  

char batchNo1_b[BATCH_NO_MAX_LEN] = "1"; 
char batchNo2_b[BATCH_NO_MAX_LEN] = "1";//加测批次号

char currValue1[10] = "16";   //数值
char currValue2[10] = "14";

char collectTime[20];

const char *measureMode1="1";//测量模式参数
const char *measureMode2="2";



DateTime current;	
DateTime next1;
DateTime next2;
DateTime next_day;
DateTime next_validation;

char Pub_buff[512];  //发布缓存区
char buf[512] = {0};  //接受缓存区

static ImtMessage imt = {0}; //加测数据结构体
static FrqMessage frq = {0}; //变频数据结构体


int current_num=0;

char timestamp[20]; //时间字符串转换参数

int current_frq1=3;//当前数据上传频率(times per day)
int current_frq2=6;//当前数据上传频率(times per day);

int next_frq1=0;   //明天频率
int next_frq2=0;

/*一对一数据*/
float current_force;

/*一对多数据*/
AngleData angles[8];
uint8_t valid_points = 0;
SensorStatus status;

/*电池电量数据*/
 SensorStatus result;
 uint8_t soc = 0;
 
 float test_data=0;
 
int main(void)
{
	//硬件初始化
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
	//断开之前的连接
	MQTT_Disconnect();
	UartFlagStC.MQTTReceiveFlag=0;
	UartFlagStC.UART2Counter=0;
	memset(buf,0,512);
	delay_ms(1000);
	//连接mqtt服务器
  MQTT_Client_Task();
//MQTT_Connect(Client_ID, UserName, Password, 300);
//	delay_ms(1000);
//	if(MQTT_Subscribe_Topic(Sub_Topic3, 0, 1) == 0)
//  printf("加测主题订阅成功\n");
//  if(MQTT_Subscribe_Topic(Sub_Topic4, 0, 2) == 0)
//  printf("变频主题订阅成功\n");
	UartFlagStC.MQTTReceiveFlag=0;
	UartFlagStC.UART2Counter=0;
	memset(buf,0,512);
   //初始化时间
   //DS3231_setDate(2025,5,12);	
   //DS3231_setTime(9,36,00);
   
	DS3231_gettime(&current);
	DS3231_getdate(&current);
	DateTime_ToString(&current, timestamp);
	printf("当前时间: %s\n", timestamp);
	next1=calculate_next_collect(current_frq1,&current);
	DateTime_ToString(&next1, timestamp);
	printf("下次一对一数据上传时间: %s\n", timestamp);
	next2=calculate_next_collect(current_frq2,&current);
	DateTime_ToString(&next2, timestamp);
	printf("下次一对多数据上传时间：%s\n",timestamp);
	next_day=calculate_next_day(&current);
	DateTime_ToString(&next_day, timestamp);
	printf("下次更新频率和批次的时间：%s\n",timestamp);
	next_validation=calculate_next_night_validation(&current);
	DateTime_ToString(&next_validation,timestamp);
	printf("下次校验批次时间：%s\n",timestamp);
	
	 //获取设备状态
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
	 printf("设备状态上传成功\r\n"); 
			
				
	//进入主循环
	 while(1)
	{
		if(UartFlagStC.MQTTReceiveFlag){
			printf("检测到数据\n");
	    MQTT_Receive=MQTTPacket_read(buf, MQTT_MAX_BUFF, transport_getdata);
			 printf("MQTT_Receive = %d\n", MQTT_Receive);
			for(int i=0;i<64;i++)
			{
			 printf("%d ",buf[i]);
			} 
			switch(MQTT_Receive){				
				case PUBLISH:{
					int ret = parse_message(buf,MQTT_MAX_BUFF,&frq,&imt);

			//检测到频率更改数据
		      if(ret == 1) {
						  printf("检测到频率更改数据\r\n");
					 if(strcmp(frq.deviceCode,deviceCode1)==0){
		       if(is_valid_frequency(frq.currentFrq)){
					   printf("一对一预变频成功,频率为：%d\r\n",frq.currentFrq);			  
					   next_frq1=frq.currentFrq;
				  }
				 else 
					 printf("ERR_INVALID_FRQ");	
			  }
   			if(strcmp(frq.deviceCode,deviceCode2)==0){
             if(is_valid_frequency(frq.currentFrq)){
					   printf("一对多预变频成功,频率为：%d\r\n",frq.currentFrq);			  
					   next_frq2=frq.currentFrq;
				}
        	else	
            printf("ERR_INVALID_FRQ");						
			}
		} 
	
		//检测到加测数据
		   else if((ret ==0)&&(imt.measureMode==2)){
			
			 printf("检测到加测数据,measureMode:%d\n",imt.measureMode);
       printf("timestamp:%llu\r\n",imt.timestamp);
	     if (timestamp_to_datetime(imt.timestamp, &current))
			 {
				 DateTime_ToString(&current, collectTime);
			 }
				 
			 //一对一加测
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
				printf("一对一加测发布成功\n");
		   }

		   //一对多加测
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
				  printf("一对多加测发布成功\n");
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
					printf("检测到服务器心跳包\r\n");
					break;
				}
			  case CONNACK:{
					printf("MQTT连接成功\r\n");
					break;
				}
			  case SUBACK:{
             printf("话题订阅成功\r\n");		        
					   break;
				}					        
				default:{
          printf("未知或错误MQTT包类型，尝试重连...\r\n");
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
			//计算新一天首次数据上传时间
			DS3231_gettime(&current);
	        DS3231_getdate(&current);
			//计算下一次一对一数据上传时间
            next1 = calculate_next_collect(current_frq1, &current);
            DateTime_ToString(&next1, timestamp);
	    printf("下次一对一数据上传时间: %s\n", timestamp);
			//计算下一次一对多数据上传时间
		  next2 = calculate_next_collect(current_frq2, &current);
      DateTime_ToString(&next2, timestamp);
	    printf("下次一对多数据上传时间: %s\n", timestamp);
      //更新批次号
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
	 printf("设备状态上传成功\r\n");
		UartFlagStC.MQTTReceiveFlag=0;
	  UartFlagStC.UART2Counter=0;
	  memset(buf,0,512);
		}

			//一对一数据上传
	  if(up_data1_ready==1){
        DS3231_gettime(&current);
	    DS3231_getdate(&current);
			//计算下一次一对一数据上传时间
        next1 = calculate_next_collect(current_frq1, &current);
        DateTime_ToString(&next1, timestamp);
	      printf("下次一对一数据上传时间: %s\n", timestamp);
        // 执行数据采集与上传	 
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
			printf("一对一按时数据已经上报\r\n");
			up_data1_ready=0;
			UartFlagStC.MQTTReceiveFlag=0;
	    UartFlagStC.UART2Counter=0;
	    memset(buf,0,512);
      }

		if(up_data2_ready==1){
			DS3231_gettime(&current);
	        DS3231_getdate(&current);
			  //计算下一次一对多数据上传时间
            next2 = calculate_next_collect(current_frq2, &current);
            DateTime_ToString(&next2, timestamp);
	        printf("下次一对多数据上传时间: %s\n", timestamp);
            // 执行数据采集与上传	 
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
			    printf("一对多按时数据已经上报\r\n");
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
		transport_sendPacketBuffer( mqtt_buf, len);       // 发送心跳包
		printf("心跳包发送成功\r\n");
		delay_ms(1000);
		if (MQTTPacket_read(buf, MQTT_MAX_BUFF, transport_getdata)== PINGRESP)
		{
			printf("检测到服务器心跳包\r\n");
		}
		else{
			for(int i=0;i<64;i++)
			{
			 printf("%d ",buf[i]);
			} 
			printf("ERR_NO_PINGRESP,正在尝试重连\r\n");
			
			MQTT_Client_Task();
		}
      heartbeat=0;
		  UartFlagStC.MQTTReceiveFlag = 0;
      UartFlagStC.UART2Counter = 0;
      memset(buf,0,512);
	 }

    if(next_valid==1){
	    //校验一对一
	    current_num=atoi(batchNo1_a)-1;
        if(current_num<current_frq1)
		{
			DS3231_gettime(&current);
			DS3231_getdate(&current);
				//计算下一次一对一数据上传时间
			next1 = calculate_next_collect(current_frq1, &current);
			DateTime_ToString(&next1, timestamp);
			  printf("下次一对一数据上传时间: %s\n", timestamp);
			// 执行数据采集与上传	 
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
				printf("一对一按时数据已经上报\r\n");
        } 
		//校验一对多
		current_num=atoi(batchNo2_a)-1;
        if(current_num<current_frq2)
		{
			DS3231_gettime(&current);
	        DS3231_getdate(&current);
			  //计算下一次一对多数据上传时间
            next2 = calculate_next_collect(current_frq2, &current);
            DateTime_ToString(&next2, timestamp);
	        printf("下次一对多数据上传时间: %s\n", timestamp);
            // 执行数据采集与上传	 
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
			      printf("一对多按时数据已经上报\r\n");
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
		printf("下次校验批次时间：%s\n",timestamp);
		next_valid=0;
		UartFlagStC.MQTTReceiveFlag=0;
	  UartFlagStC.UART2Counter=0;
	  memset(buf,0,512);
  }
		
		//重连机制
	if(reconnect_flag==1)
	{
		MQTT_Disconnect();
	  UartFlagStC.MQTTReceiveFlag=0;
	  UartFlagStC.UART2Counter=0;
	  memset(buf,0,512);
	  delay_ms(1000);
		if(MQTT_Connect(Client_ID, UserName, Password, 300)==MQTT_SUCCESS){
			printf("MQTT重新连接成功，订阅主题...\n");
			delay_ms(500);
			bool sub3_ok = (MQTT_Subscribe_Topic(Sub_Topic3, 0, 1) == 0);
      bool sub4_ok = (MQTT_Subscribe_Topic(Sub_Topic4, 0, 2) == 0);
            if(sub3_ok && sub4_ok){
                printf("订阅成功\n");
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
			printf("当前时间: %s\n", timestamp);
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
	 printf("设备状态上传成功\r\n");
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

//按时上传数据处理函数
void TIM5_IRQHandler(void){
	 if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM5, TIM_IT_Update); // 清除中断标识位
		time_flag++;
		DS3231_gettime(&current);
	    DS3231_getdate(&current);

//		if(time_flag==4){
//			DateTime_ToString(&current, timestamp);
//			printf("当前时间: %s\n", timestamp);
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
//	 printf("设备状态上传成功\r\n");
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

// 心跳包发送中断函数
void TIM2_IRQHandler(void) {
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
         // 300秒触发 
			if(offline_flag==0){				
		     heartbeat=1;
			}
    }
}



//自动监测数据加测下发{"deviceCode":"GJO1","batchNo":"1","measureMode":"2","timestamp":"1734679545903"}                                                                                                                                            
//自动监测数据频率变更{"deviceCode":"GJO1","currentFrq":"4","frqType":"1","timestamp":"1734679545903"}
//{"deviceCode":"GJO1","batchNo":"1","currValue":"3",collectTime:"2024-07-24 09:06:16","measureMode":"2"};
		
		
				