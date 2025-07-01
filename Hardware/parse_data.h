#ifndef __PARSE_H
#define __PARSE_H

#include "stm32f4xx.h"
#include <stdbool.h>
#include "cJSON.h"
#include "time.h"
#define MAX_PROTOCOL_SIZE 512



/*---------------------- IMT�������ݽṹ ----------------------*/
typedef struct {
    char deviceCode[16];    // ���ݱ���("GJO1")
    uint64_t timestamp;     // ʱ���(���룬�ַ���תuint64)
    uint8_t batchNo;        // �������κ�(1-8)
    uint8_t measureMode;    // ����ģʽ(1-�������,2-�Ӳ��·�)
} ImtMessage;

/*---------------------- FRQ�������ݽṹ----------------------*/
typedef struct {
    char deviceCode[16];    // �豸����
    uint64_t timestamp;     // ʱ���
    uint16_t currentFrq;    // ��ǰƵ��
    uint8_t  frqType;        // Ƶ������(1.Ƶ�ʱ�� 2.Ƶ�ʳ�ʼ��)
} FrqMessage;

//��������ϵ
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

//��������
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

