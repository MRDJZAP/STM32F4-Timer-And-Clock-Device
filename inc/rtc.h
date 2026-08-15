#ifndef RTC_H
#define RTC_H

#include "stdint.h"

// an enum representing AM or PM
typedef enum HR_AM_PM { AM = 0, PM } HR_AM_PM;

// an enum representing weekdays
typedef enum WeekDays { MON = 1, TUE, WED, THU, FRI, SAT, SUN } WeekDays;

typedef enum Months {
  JAN = 1,
  FEB,
  MAR,
  APR,
  MAY,
  JUN,
  JUL,
  AUG,
  SEP,
  OCT,
  NOV,
  DEC
} Months;

// format12_24 is [0, 1], 0 for AM or 24hr, 1 for PM
// hours is [0, 23]
// minutes is [0, 59]
// seconds is [0, 59]
typedef struct {
  uint8_t format12_24;
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
} TimeStruct;

// weekday is [1, 7]
// day is [1, 31]
// month [1, 12]
// year is [0, 99]
typedef struct {
  uint8_t weekday;
  uint8_t day;
  uint8_t month;
  uint8_t year;
} DateStruct;

// initializes the RTC time with specified time and date
// PRE: hourFormat is [0, 1], 0 for 24hr, 1 for AM/PM hr, defaults to 24hr
void rtc_init(DateStruct *date, TimeStruct *time, uint8_t hourFormat);

// changes time
// PRE: rtc_init(...) must be called before hand
void change_time(TimeStruct *time, uint8_t hourFormat);

// changes date
// PRE: rtc_init(...) must be called before hand
void change_date(DateStruct *date);

// returns year
// PRE: milinium_century must have century and milinium value
//      (ex. year 2024) would have milinium_century = 2000
uint32_t rtc_date_get_year(uint32_t milinium_century);

// Returns the weekday as an enum
WeekDays rtc_date_get_weekday(void);

// Returns the month as an enum
Months rtc_date_get_month(void);

// Returns the day of the month (1-31)
uint32_t rtc_date_get_dayOfMonth(void);

// returns AM/PM as enum, defaults to AM if using 24-hour
HR_AM_PM rtc_time_get_AM_PM();

// returns hours
uint32_t rtc_time_get_hour();

// returns minutes
uint32_t rtc_time_get_minute();

// returns seconds
uint32_t rtc_time_get_seconds();

#endif
