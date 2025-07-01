#ifndef __FRQ_H
#define __FRQ_H

#include "stm32f4xx.h"
#include "time.h"
#include <stdbool.h>

bool is_valid_frequency(uint8_t frq);
uint8_t get_current_frequency(void);
DateTime calculate_next_collect(uint8_t frq, const DateTime* now);
DateTime calculate_next_day(const DateTime* now);
static uint8_t days_in_month(uint8_t year, uint8_t month);
DateTime calculate_next_night_validation(const DateTime* now);
#endif
