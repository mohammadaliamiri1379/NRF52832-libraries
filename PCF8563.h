#ifndef PCF8563_H
#define PCF8563_H

#define W8563_ADDR     0xA2    // I2C address for write
#define R8563_ADDR     0xA3    // I2C address for read
#define PCF8563_FREQ     400000  // bus speed 400 kHz
/*
/*
    for transfer time_t we use this struct.
struct tm {
   int tm_sec;   // seconds of minutes from 0 to 61
   int tm_min;   // minutes of hour from 0 to 59
   int tm_hour;  // hours of day from 0 to 24
   int tm_mday;  // day of month from 1 to 31
   int tm_mon;   // month of year from 0 to 11
   int tm_year;  // year since 1900
   int tm_wday;  // days since sunday
   int tm_yday;  // days since January 1st
   int tm_isdst; // hours of daylight savings time
}
*/

/*

#include "mbed.h"
#include "eeprom.h"
#include "PCF8563.h"
#define BUFFER_SIZE  64
#define BAUDRATE 9600

//I2C i2c(p11,p12);       // I2C device sda, scl
PCF8563 rtc(p11,p12);   // rtc object sda , scl 
Serial pc(p6, p8 , BAUDRATE);      // tx    rx

int main()
    {
      // -----------set time  & read time-------------
      rtc.init();
      time_t seconds = 1256729730 ;   // number of seconds elapsed since January 1, 1970.
      rtc.set_time(seconds) ;
      pc.printf("set time to %lld\n" , (long long) seconds ) ;
      
      char* dt = ctime(&seconds) ; // char *ctime(const time_t *time);
                                   // This returns a pointer to a string of the form 
                                   // day month year hours:minutes:seconds year\n\0.
      printf(dt);  // you should see  Wed Oct 28 11:35:30 2009
      pc.printf("wait 5 second \n") ;
      
      wait(5) ; // wait 5 second
      pc.printf("\n") ;
      seconds = rtc.now(); 
      pc.printf("%lld\n" , (long long) seconds ) ;
      dt = ctime(&seconds) ;
      printf(dt); // you should see  Wed Oct 28 11:35:35 2009
      printf(" \n" );
      //------------ use this code to make time_t-----------------
      struct tm now_t;
      now_t.tm_sec = 0 ;
      now_t.tm_min = 0 ;
      now_t.tm_hour = 12;
      now_t.tm_mday = 1 ;
      now_t.tm_mon = 1 ;
      now_t.tm_year=  120 ;
      seconds = mktime(&now_t);  // you can send this variable to set_time function
      dt = ctime(&seconds) ;
      printf(dt); // you should see Sat Feb  1 12:00:00 2020
      
      wait(2) ;
      // ----------------- set alarm ------------------
      //while(true)
      //{
      seconds = 1256816130;   // one day after rtc.now
                              // make sure that day does'nt 
                              // effect alrm
      seconds+= 60 ;
      pc.printf("alarm set for %lld\n" , (long long) rtc.now() ) ;
      dt = ctime(&seconds) ;
      printf(dt); // you should see  Wed Oct 28 11:35:37 2009
      printf(" \n" );
      
      // set an alarm 
      
      rtc.enable_alarm() ;
      rtc.set_alarm(seconds);
      // check RTC alarm 
      rtc.time_in_alarm();// just for deboug
      rtc.check_status() ;// just for deboug
      rtc.check_alarm() ; // just for deboug
      
      wait(2) ;
      
      while(true)
      {
      seconds = rtc.now();
      dt = ctime(&seconds) ;
      printf(dt); 
      printf(" \n" );
      rtc.check_status() ;// just for deboug
      if(rtc.check_alarm() == 1) // just for deboug
      {
          wait(5) ;
          rtc.alarmOff();
          }
      wait(3);
      }
      
}

*/
class PCF8563
{
public:
    PCF8563(PinName sda, PinName scl);

    /** Read current real time from PCF8563
     *
     * @returns
     *   current time on success,
     *   0 on error (I2C fail, clock not set)
     */
    time_t now();
    
    /** Write current real time to PCF8563
     *
     * @param time Real time to set up
     * @returns
     *   true on success,
     *   false on error (I2C fail)
     */
    bool set_time(time_t time);
    
    /** Write alarm time to PCF8563
     * @param time is set time for alarm
     * @returns
     *    true on success,
     *    false on error
     */
    bool set_alarm(time_t time);
    
    /**
     * set AF bit to zero
     * to shout down the INT
     */
    bool alarmOff();
    
    /**
     * check that if alarm is ON
     * or OFF
     */
    bool check_alarm();
    
    /**
     * send a master acknowledge
     */
    void ack_polling() ;
    /**
     * enable alarm
     * set bit AIE 0X01H to 1
     */
    void enable_alarm();
    /**
     * check status 2
     * refister 0x01 
     */
    void check_status();
    /**
     * print alarm datas in uart
     */
    void time_in_alarm();
    /**
     * initial the alarm and clkout 
     */
    void init() ;
    
private:
    I2C pcf8563_i2c;
    // convert bcd to decimal
    static int bcdToDecimal(int bcd)
     {
        return ((bcd & 0xF0) >> 4) * 10 + (bcd & 0x0F);
    }
    // convert decimal to bcd
    static int decimalToBcd(int dec) 
    {
        return (dec % 10) + ((dec / 10) << 4);
    }
};

#endif
