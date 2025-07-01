#include "Frq.h"


/**
 * @brief ��֤Ƶ�ʵĺϷ���
 * @param frq ����֤��Ƶ��ֵ
 * @return true-�Ϸ�, false-�Ƿ�
 */
bool is_valid_frequency(uint8_t frq) {
    return (frq > 0) && (86400 % frq == 0);
}

//������������ȡĳ��ĳ�µ�����
static uint8_t days_in_month(uint8_t year, uint8_t month) {
    const uint8_t days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2) {
        // �����ж�
        return ((2000 + year) % 4 == 0) ? 29 : 28;
    }
    return (month >= 1 && month <= 12) ? days[month - 1] : 0;
}


/**
 * @brief �������㿪ʼ�ĵ�һ���Ϸ�ʱ���
 * @param frq ��ǰƵ��
 * @param now ��ǰʱ��
 * @return DateTime ��һ���ɼ�ʱ��
 */

DateTime calculate_next_collect(uint8_t frq, const DateTime* now) {
    DateTime next = *now;
    uint32_t interval = 86400 / frq; // ����������
    uint32_t current_seconds = now->hours * 3600 + now->minutes * 60 + now->seconds;
    
    // ������һ�������
    uint32_t next_seconds = ((current_seconds / interval) + 1) * interval;
    
    // �������
    if (next_seconds >= 86400) {
        next_seconds -= 86400;
        next.day++;
    }
    
    // ת������Ϊʱ����
    next.hours = next_seconds / 3600;
    next.minutes = (next_seconds % 3600) / 60;
    next.seconds = next_seconds % 60;
    
    // �������ڽ�λ����ȷ�� days_in_month ������ȷ��
    while (next.day > days_in_month(next.year, next.month)) {
        next.day -= days_in_month(next.year, next.month);
        next.month++;
        if (next.month > 12) {
            next.month = 1;
            next.year = (next.year + 1) % 100; // �귶Χ 2000-2099
        }
    }
    
    return next;
} 


//������һ���賿ʱ��
DateTime calculate_next_day(const DateTime* now) {
    DateTime next = *now;

    // ֱ������һ��1��
    next.day++;
    next.hours = 0;
    next.minutes = 0;
    next.seconds = 0;

    // �������ڽ�λ
    uint8_t max_days = days_in_month(next.year, next.month);
    while (next.day > max_days) {
        next.day = 1;
        next.month++;
        if (next.month > 12) {
            next.month = 1;
            next.year++;
           
            if (next.year > 99) {
                next.year = 0; 
            }
        }
        max_days = days_in_month(next.year, next.month);
    }

    return next;
}

DateTime calculate_next_night_validation(const DateTime* now) {
    DateTime next = *now;  // ���Ƶ�ǰʱ��
    
    // ǿ���趨ʱ��Ϊ23:30:00
    next.hours = 23;
    next.minutes = 30;
    next.seconds = 0;      // �������
    
    // �жϵ�ǰʱ���Ƿ��ѹ�����23:30
    if (now->hours > 23 || (now->hours == 23 && now->minutes >= 30)) {
        // ��Ҫ������յ�23:30
        next.day++;  // ����һ��
        
        // �������ڽ�λ��������ԭ�����߼�һ�£�
        uint8_t max_days = days_in_month(next.year, next.month);
        while (next.day > max_days) {
            next.day = 1;
            next.month++;
            if (next.month > 12) {
                next.month = 1;
                next.year++;
                if (next.year > 99) {  // �������Ϊ��λ������00-99��
                    next.year = 0; 
                }
            }
            max_days = days_in_month(next.year, next.month);
        }
    }
    
    return next;
}
