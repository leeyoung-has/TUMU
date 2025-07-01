#include "Frq.h"


/**
 * @brief 验证频率的合法性
 * @param frq 待验证的频率值
 * @return true-合法, false-非法
 */
bool is_valid_frequency(uint8_t frq) {
    return (frq > 0) && (86400 % frq == 0);
}

//辅助函数：获取某年某月的天数
static uint8_t days_in_month(uint8_t year, uint8_t month) {
    const uint8_t days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2) {
        // 闰年判断
        return ((2000 + year) % 4 == 0) ? 29 : 28;
    }
    return (month >= 1 && month <= 12) ? days[month - 1] : 0;
}


/**
 * @brief 计算从零点开始的第一个合法时间点
 * @param frq 当前频率
 * @param now 当前时间
 * @return DateTime 下一个采集时间
 */

DateTime calculate_next_collect(uint8_t frq, const DateTime* now) {
    DateTime next = *now;
    uint32_t interval = 86400 / frq; // 计算间隔秒数
    uint32_t current_seconds = now->hours * 3600 + now->minutes * 60 + now->seconds;
    
    // 计算下一个对齐点
    uint32_t next_seconds = ((current_seconds / interval) + 1) * interval;
    
    // 处理跨天
    if (next_seconds >= 86400) {
        next_seconds -= 86400;
        next.day++;
    }
    
    // 转换秒数为时分秒
    next.hours = next_seconds / 3600;
    next.minutes = (next_seconds % 3600) / 60;
    next.seconds = next_seconds % 60;
    
    // 处理日期进位（需确保 days_in_month 函数正确）
    while (next.day > days_in_month(next.year, next.month)) {
        next.day -= days_in_month(next.year, next.month);
        next.month++;
        if (next.month > 12) {
            next.month = 1;
            next.year = (next.year + 1) % 100; // 年范围 2000-2099
        }
    }
    
    return next;
} 


//计算下一个凌晨时间
DateTime calculate_next_day(const DateTime* now) {
    DateTime next = *now;

    // 直接增加一天1天
    next.day++;
    next.hours = 0;
    next.minutes = 0;
    next.seconds = 0;

    // 处理日期进位
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
    DateTime next = *now;  // 复制当前时间
    
    // 强制设定时间为23:30:00
    next.hours = 23;
    next.minutes = 30;
    next.seconds = 0;      // 清空秒数
    
    // 判断当前时间是否已过今日23:30
    if (now->hours > 23 || (now->hours == 23 && now->minutes >= 30)) {
        // 需要计算次日的23:30
        next.day++;  // 增加一天
        
        // 处理日期进位（保持与原函数逻辑一致）
        uint8_t max_days = days_in_month(next.year, next.month);
        while (next.day > max_days) {
            next.day = 1;
            next.month++;
            if (next.month > 12) {
                next.month = 1;
                next.year++;
                if (next.year > 99) {  // 假设年份为两位数（如00-99）
                    next.year = 0; 
                }
            }
            max_days = days_in_month(next.year, next.month);
        }
    }
    
    return next;
}
