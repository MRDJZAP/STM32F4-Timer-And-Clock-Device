#include "rtc.h"
#include "chip_headers/CMSIS/Device/ST/STM32F4xx/Include/stm32f411xe.h"
#include <stdint.h>

#define PWREN (1U << 28)      // power module clock enable
#define CR_DBP (1U << 8)      // enables access to backup domain
#define CSR_LSION (1U << 0)   // enables LSI oscillator
#define CSR_LSIRDY (1U << 1)  // state of the LSI
#define BDCR_LSEON (1U << 0)  // turns on the LSE clock
#define BDCR_LSERDY (1U << 1) // whether LSE is ready or not
#define BDCR_BDRST (1U << 16) // forces a reset of the backup domain
#define BDCR_RTCEN (1U << 15) // enables the RTC

#define RTC_WRITE_PROTECTION_KEY_1                                             \
  ((uint8_t)0xCAU) // First key to disable write protection on RTC
#define RTC_WRITE_PROTECTION_KEY_2                                             \
  ((uint8_t)0x53U) // Second key to disable write protection on RTC
#define RTC_INIT_MASK 0xFFFFFFFFU // Mask to enter intizialization mode of RTC
#define ISR_INITF (1U << 6)       // State of initialization mode of RTC

#define WEEKDAY_MONDAY                                                         \
  ((uint8_t)0x01U) // Configure the weekday of the calender to Monady
#define MONTH_JULY ((uint8_t)0x07U) // Configure the month to July
#define TIME_FORMAT_PM                                                         \
  (1U << 22)                  // Sets the time format to PM in 12-hour format
#define CR_FMT_12HR (1U << 6) // Sets the hour format to 24-hour format
#define ISR_RSF (1U << 5)     // RTC time register syncronized or not

#define RTC_ASYNCH_PREDIV ((uint32_t)0x7FU) // 127 -> Div 128
#define RTC_SYNCH_PREDIV                                                       \
  ((uint32_t)0x0FFU) // 255 -> Div 256 (Change from 0x00F9)

// returns the BCD of the given value
static uint8_t dec_to_bcd(uint8_t value) {
  return ((value / 10U) << 4U) | (value % 10U);
}

// returns the decimal equivalant to BCD value
static uint8_t bcd_to_dec(uint8_t value) {
  return (uint8_t)(((value >> 4) * 10U) + (value & 0x0FU));
}

// sets async prescaler division value
// PRE: must have called ____ beforehand,
//      value must be no larger than 127
static void rtc_set_asynch_prescaler(uint32_t value) {
  // reset the prescale value
  RTC->PRER &= ~(0b1111111U << 16);
  RTC->PRER |= (value << 16);
}

// sets sync prescaler division value
// PRE: must have called ____ beforehand
//      value must be no larger than 32,767
static void rtc_set_synch_prescaler(uint32_t value) {
  // reset sync prescale value
  RTC->PRER &= ~(0b111111111111111U);
  RTC->PRER |= value;
}

// sets RTC to initialization mode
static void rtc_enable_init_mode() { RTC->ISR = RTC_INIT_MASK; }

// sets RTC back to normal mode and clears some flags
static void rtc_disable_init_mode() { RTC->ISR = ~RTC_INIT_MASK; }

// returns 1 if rtc is in initialization mode
static uint8_t rtc_is_init_mode() { return (RTC->ISR & ISR_INITF) != 0; }

// returns 1 if the rtc register's are synchronized
static uint8_t rtc_reg_is_sync() { return (RTC->ISR & ISR_RSF) != 0; }

// safely sets rtc to initializing mode and waits
// until it is in that mode before returning 1
static uint8_t rtc_init_seq() {
  // start init mode
  rtc_enable_init_mode();

  // wait untill it is in init mode
  while (!rtc_is_init_mode())
    ;

  return 1;
}

// waits until the RTC registers are synchronized before returning 1
static uint8_t wait_for_synchro() {
  // Clear RSF before checking it
  RTC->ISR &= ~ISR_RSF;

  while (!rtc_reg_is_sync())
    ;

  return 1;
}

// exits the RTC intitialization mode, returns 1
// when registers have been fully syncrhonized
static uint8_t exit_init_seq() {
  rtc_disable_init_mode();

  return wait_for_synchro();
}

// sets the date of the RTC
// PRE: RTC must be in intialization mode and write proteciton must be disabled
//      weekday is [1, 7]
//      day is [1, 31]
//      month [1, 12]
//      year is [0, 99]
static void rtc_date_config(uint8_t weekday, uint8_t day, uint8_t month,
                            uint8_t year) {
  uint32_t temp = ((uint32_t)dec_to_bcd(year) << 16) |
                  ((uint32_t)(weekday & 0x07U) << 13) |
                  ((uint32_t)(dec_to_bcd(month) & 0x1FU) << 8) |
                  ((uint32_t)(dec_to_bcd(day) & 0x3FU));

  RTC->DR = temp;
}

// sets the time of the RTC
// PRE: RTC must be in initialization mode and write protection must be disabled
//      format12_24 is [0, 1], 0 for AM or 24hr, 1 for PM
//      hours is [0, 23]
//      minutes is [0, 59]
//      seconds is [0, 59]
static void rtc_time_config(uint8_t format12_24, uint8_t hours, uint8_t minutes,
                            uint8_t seconds) {

  uint32_t temp = ((uint32_t)(format12_24 & 0x01U) << 22) |
                  ((uint32_t)(dec_to_bcd(hours) & 0x3FU) << 16) |
                  ((uint32_t)(dec_to_bcd(minutes) & 0x7FU) << 8) |
                  ((uint32_t)(dec_to_bcd(seconds) & 0x7FU));

  RTC->TR = temp;
}

// initializes the RTC time with specified time and date
// PRE: hourFormat is [0, 1], 0 for 24hr, 1 for AM/PM hr, defaults to 24hr
void rtc_init(DateStruct *date, TimeStruct *time, uint8_t hourFormat) {
  // Enable clock access to PWR
  RCC->APB1ENR |= PWREN;

  // Disable Backupt Domain write protection
  PWR->CR |= CR_DBP;

  (void)PWR->CR;

  // force backup domain reset
  // We must reset to be able to write to the bits of BDCR since the BDCR is not
  // reset by system reset and it presists its values even after reboots
  RCC->BDCR |= BDCR_BDRST;

  // Release backup domain reset
  RCC->BDCR &= ~BDCR_BDRST;

  // Enable LSE oscilator
  RCC->BDCR |= BDCR_LSEON;

  // Wait for LSE to be ready
  while (!(RCC->BDCR & BDCR_LSERDY))
    ;

  // Set RTC clock source to LSE
  RCC->BDCR &= ~(1U << 9);
  RCC->BDCR |= (1U << 8);

  // Enable the RTC
  RCC->BDCR |= BDCR_RTCEN;

  // disable RTC registers write protection
  RTC->WPR = RTC_WRITE_PROTECTION_KEY_1;
  RTC->WPR = RTC_WRITE_PROTECTION_KEY_2;

  // Enter initialization mode
  if (!rtc_init_seq()) {
    // Handle initialization failure
  }

  // set desired date
  rtc_date_config(date->weekday, date->day, date->month, date->year);

  // set desired
  rtc_time_config(time->format12_24, time->hours, time->minutes, time->seconds);

  if (hourFormat == 1) {
    RTC->CR |= CR_FMT_12HR;
  } else {
    RTC->CR &= ~CR_FMT_12HR;
  }

  // set Asyncrhonous prescaler
  rtc_set_asynch_prescaler(RTC_ASYNCH_PREDIV);

  // set Syncrhonous prescaler
  rtc_set_synch_prescaler(RTC_SYNCH_PREDIV);

  // exit initialization mode
  exit_init_seq();

  // Enable RTC registers write protection
  RTC->WPR = 0xFFU;
}

// changes time
// PRE: rtc_init(...) must be called before hand
void change_time(TimeStruct *time, uint8_t hourFormat) {

  // disable RTC registers write protection
  RTC->WPR = RTC_WRITE_PROTECTION_KEY_1;
  RTC->WPR = RTC_WRITE_PROTECTION_KEY_2;

  // Enter initialization mode
  if (!rtc_init_seq()) {
    // Handle initialization failure
  }

  // set desired
  rtc_time_config(time->format12_24, time->hours, time->minutes, time->seconds);

  if (hourFormat == 1) {
    RTC->CR |= CR_FMT_12HR;
  } else {
    RTC->CR &= ~CR_FMT_12HR;
  }

  // exit initialization mode
  exit_init_seq();

  // Enable RTC registers write protection
  RTC->WPR = 0xFFU;
}

// changes date
// PRE: rtc_init(...) must be called before hand
void change_date(DateStruct *date) {
  // disable RTC registers write protection
  RTC->WPR = RTC_WRITE_PROTECTION_KEY_1;
  RTC->WPR = RTC_WRITE_PROTECTION_KEY_2;

  // Enter initialization mode
  if (!rtc_init_seq()) {
    // Handle initialization failure
  }

  // set desired date
  rtc_date_config(date->weekday, date->day, date->month, date->year);

  // exit initialization mode
  exit_init_seq();

  // Enable RTC registers write protection
  RTC->WPR = 0xFFU;
}

// returns year
// PRE: milinium_century must have century and milinium value
//      (ex. year 2024) would have milinium_century = 2000
uint32_t rtc_date_get_year(uint32_t milinium_century) {
  uint8_t year_bcd = (uint8_t)((RTC->DR >> 16U) & 0xFFU);
  return (uint32_t)bcd_to_dec(year_bcd) + milinium_century;
}

// Returns the weekday as an enum
WeekDays rtc_date_get_weekday(void) {
  return (WeekDays)((RTC->DR >> 13U) & 0x07U);
}

// Returns the month as an enum
Months rtc_date_get_month(void) {
  uint8_t month_bcd = (uint8_t)((RTC->DR >> 8U) & 0x1FU);
  return (Months)bcd_to_dec(month_bcd);
}

// Returns the day of the month (1-31)
uint32_t rtc_date_get_dayOfMonth(void) {
  uint8_t day_bcd = (uint8_t)(RTC->DR & 0x3FU);
  return (uint32_t)bcd_to_dec(day_bcd);
}

// returns AM/PM as enum, defaults to AM if using 24-hour
HR_AM_PM rtc_time_get_AM_PM() {
  HR_AM_PM hr_format = (HR_AM_PM)((RTC->TR >> 22U) & 0x01U);
  (void)RTC->DR; // read date to prevent shadow register locking
  return hr_format;
}

// returns hours
uint32_t rtc_time_get_hour() {
  uint32_t hour = (uint32_t)bcd_to_dec((uint8_t)((RTC->TR >> 16U) & 0x3FU));
  (void)RTC->DR; // read date to prevent shadow register locking
  return hour;
}

// returns minutes
uint32_t rtc_time_get_minute() {
  uint32_t minute = (uint32_t)bcd_to_dec((uint8_t)((RTC->TR >> 8U) & 0x7FU));
  (void)RTC->DR; // read date to prevent shadow register locking
  return minute;
}

// returns seconds
uint32_t rtc_time_get_seconds() {
  uint32_t seconds = (uint32_t)bcd_to_dec((uint8_t)(RTC->TR & 0x7FU));
  (void)RTC->DR; // read date to prevent shadow register locking
  return seconds;
}
