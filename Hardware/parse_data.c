#include "parse_data.h"
#include "transport.h"
#include "usart.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "delay.h"
#include <stdint.h>


void* memmem(const void* haystack, size_t haystack_len,
            const void* needle, size_t needle_len) {
    if (needle_len == 0) return (void*)haystack;
    if (haystack_len < needle_len) return NULL;

    const uint8_t* h = haystack;
    for (size_t i = 0; i <= haystack_len - needle_len; ++i) {
        if (memcmp(h + i, needle, needle_len) == 0) {
            return (void*)(h + i);
        }
    }
    return NULL;
}

 int parse_imt_message(uint8_t *json_start, uint16_t len, ImtMessage *msg) {
    cJSON *root = cJSON_ParseWithLength((char*)json_start, len);
    if(!root) return -3; // JSON解析失败
    int ret = fill_imt_struct(root, msg);
    cJSON_Delete(root);
    return ret;
}
 

// IMT结构体填充
int fill_imt_struct(cJSON* root, ImtMessage* msg) {
    // 字段存在性检查
    const char* required[] = {"deviceCode","batchNo","measureMode","timestamp"};
    for(int i=0; i<4; i++) {
        if(!cJSON_GetObjectItem(root, required[i])) {
            return -10; // ERR_FIELD_MISSING
        }
    }
   
    // 设备编码
    cJSON* code = cJSON_GetObjectItem(root, "deviceCode");
    strncpy(msg->deviceCode, code->valuestring, sizeof(msg->deviceCode)-1);
    msg->deviceCode[sizeof(msg->deviceCode)-1] = 0;
    
    // 检测批次
    cJSON* batch = cJSON_GetObjectItem(root, "batchNo");
    long batch_val = (long)cJSON_GetNumberValue(batch);
    if(batch_val <1 || batch_val>255) return -11; // ERR_VALUE_RANGE
    msg->batchNo = batch_val;
    
    // 测量模式
    cJSON* mode = cJSON_GetObjectItem(root, "measureMode");
    long mode_val = strtol(mode->valuestring, NULL, 10);
    if(mode_val <1 || mode_val>2) return -12; // ERR_MODE_INVALID
    msg->measureMode=mode_val;
		
    // 时间戳
    cJSON* ts = cJSON_GetObjectItem(root, "timestamp");
    if (ts != NULL && cJSON_IsNumber(ts)) {
    msg->timestamp = (uint64_t)cJSON_GetNumberValue(ts); 
}
    return 0;
}



//FRQ消息解析（复用用户提供的基础）
int parse_frq_message(uint8_t *json_start, uint16_t len, FrqMessage *msg) {
    cJSON *root = cJSON_ParseWithLength((char*)json_start, len);
    if(!root) return -3; // JSON解析失败
    
    int ret = fill_frq_struct(root, msg);
    cJSON_Delete(root);
    return ret;
}


//FRQ结构体填充
int fill_frq_struct(cJSON* root, FrqMessage* msg) {
     // 字段存在性检查
    const char* required[] = {"deviceCode","frqType","currentFrq","timestamp"};//"currentFrq"
    for(int i=0; i<4; i++) {
        if(!cJSON_GetObjectItem(root, required[i])) {
            return -10; // ERR_FIELD_MISSING
        }
    }
    
    // 设备编码
    cJSON* code = cJSON_GetObjectItem(root, "deviceCode");
    strncpy(msg->deviceCode, code->valuestring, sizeof(msg->deviceCode)-1);
    msg->deviceCode[sizeof(msg->deviceCode)-1] = 0;
    
		// 频率类型
    cJSON* type = cJSON_GetObjectItem(root, "frqType");
    long type_val = strtol(type->valuestring, NULL, 10);
    if(type_val <1 || type_val>2) return -13; // ERR_FREQ_INVALID
		
    // 当前频率
    cJSON* frq = cJSON_GetObjectItem(root, "currentFrq");
    long frq_val = (long)cJSON_GetNumberValue(frq);
    if(frq_val <1 || frq_val>256) return -11; // ERR_VALUE_RANGE
    msg->currentFrq = frq_val;
     
    
    // 时间戳
    cJSON* ts = cJSON_GetObjectItem(root, "timestamp");
     if (ts != NULL && cJSON_IsNumber(ts)) {
    msg->timestamp = (uint64_t)cJSON_GetNumberValue(ts); 
}
    return 1;
}



//主解析函数，路由分发

int parse_message(uint8_t *data, uint16_t len, FrqMessage *frq, ImtMessage *imt) {
    // 快速定位话题类型
    uint8_t *topic_pos = memmem(data, len, "down/point/", 11);
    if(!topic_pos) return -1; // 话题头缺失
    
    // 判断话题具体类型
    if(memcmp(topic_pos+11, "frq", 3) == 0) {
        return parse_frq_message(topic_pos+14, data+len - (topic_pos+14), frq);
    } 
    else if(memcmp(topic_pos+11, "imt", 3) == 0) {
        return parse_imt_message(topic_pos+14, data+len - (topic_pos+14), imt);
    }
    return -2; // 未知话题
}



// 判断是否为闰年(公历)
bool is_leap_year(uint16_t year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// 获取某年某月的天数
uint8_t get_month_days(uint16_t year, uint8_t month) {
    const uint8_t days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (month == 2 && is_leap_year(year)) return 29;
    return (month >= 1 && month <= 12) ? days[month-1] : 0;
}

bool timestamp_to_datetime(uint64_t timestamp, DateTime* dt) {
    // 1. 计算总天数和剩余天数
	  timestamp /= 1000;
    const uint64_t sec_per_day = 86400;
    uint64_t days = timestamp / sec_per_day;
    uint32_t rem_sec = timestamp % sec_per_day;

    // 2. 计算年份(从1970年开始)
    uint16_t year = 1970;
    while (days > 0) {
        uint16_t days_in_year = is_leap_year(year) ? 366 : 365;
        if (days >= days_in_year) {
            days -= days_in_year;
            year++;
        } else {
            break;
        }
    }

    // 3. 检查年份是否在2000-2099范围内
    if (year < 2000 || year > 2099) return false;
    dt->year = year - 2000;

    // 4. 计算月份和日期
    uint8_t month = 1;
    while (month <= 12) {
        uint8_t month_days = get_month_days(year, month);
        if (days >= month_days) {
            days -= month_days;
            month++;
        } else {
            break;
        }
    }
    dt->month = month;
    dt->day = days + 1; // days是0-based

    // 5. 计算时间
    dt->hours = rem_sec / 3600;
		dt->hours += 8; // ???
if (dt->hours >= 24) {
    dt->hours -= 24;
    dt->day++;
}
    rem_sec %= 3600;
    dt->minutes = rem_sec / 60;
    dt->seconds = rem_sec % 60;

    // 6. 验证结果有效性
    if (dt->month > 12 || dt->day > get_month_days(year, month) ||
        dt->hours >= 24 || dt->minutes >= 60 || dt->seconds >= 60) {
        return false;
    }
    printf("时间转换成功\r\n");
    return true;
}

