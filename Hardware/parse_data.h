#ifndef __PARSE_H
#define __PARSE_H

#include "stm32f4xx.h"
#include <stdbool.h>
#include "cJSON.h"
#include "time.h"
#define MAX_PROTOCOL_SIZE 512



/*---------------------- IMT话题数据结构 ----------------------*/
typedef struct {
    char deviceCode[16];    // 数据编码("GJO1")
    uint64_t timestamp;     // 时间戳(毫秒，字符串转uint64)
    uint8_t batchNo;        // 生产批次号(1-8)
    uint8_t measureMode;    // 测量模式(1-正常监测,2-加测下发)
} ImtMessage;

/*---------------------- FRQ话题数据结构----------------------*/
typedef struct {
    char deviceCode[16];    // 设备编码
    uint64_t timestamp;     // 时间戳
    uint16_t currentFrq;    // 当前频率
    uint8_t  frqType;        // 频率类型(1.频率变更 2.频率初始化)
} FrqMessage;

//错误码体系
typedef enum {
    PARSE_OK = 0,
    ERR_BUFFER_OVERFLOW = -1,
    ERR_JSON_INVALID = -2,
    ERR_TOPIC_UNKNOWN = -3,
    ERR_FIELD_MISSING = -10,
    ERR_VALUE_RANGE = -11,
    ERR_MODE_INVALID = -12,
    ERR_FREQ_INVALID = -13
} ParseStatus;

//函数声明
int parse_imt_message(uint8_t *json_start, uint16_t len, ImtMessage *msg);
int fill_imt_struct(cJSON* root, ImtMessage* msg);
int parse_frq_message(uint8_t *json_start, uint16_t len, FrqMessage *msg);
int fill_frq_struct(cJSON* root, FrqMessage* msg);
int parse_message(uint8_t *data, uint16_t len, FrqMessage *frq, ImtMessage *imt);
void* memmem(const void* haystack, size_t haystack_len,
            const void* needle, size_t needle_len);
bool is_leap_year(uint16_t year);
uint8_t get_month_days(uint16_t year, uint8_t month);
bool timestamp_to_datetime(uint64_t timestamp, DateTime* dt);
#endif

