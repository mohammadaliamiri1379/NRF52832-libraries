#include "mbed.h"
#include "PCF8563.h"

extern Serial pc;

PCF8563::PCF8563(PinName sda, PinName scl) : pcf8563_i2c (sda, scl)
{
    pcf8563_i2c.frequency(PCF8563_FREQ);
}
void PCF8563::init()
{
    char Int[2];
    Int[0] = 0x01; // control address
    Int[1] = 0x1a; // set AIE to 1 for INT pin
    if(pcf8563_i2c.write(W8563_ADDR, Int, 2) != 0) 
    {
        pc.printf("error in init \n");
        }
    
    
    
    Int[0] = 0x0d; // control address
    Int[1] = 0x83; /** 0x83  ---> 1Hz
                    * 0x82  --->
                    * 0x81  ---> 
                    * 0x80  --->
                    */
    if(pcf8563_i2c.write(W8563_ADDR, Int, 2) != 0) 
    {
        pc.printf("error in init \n");
        }
    
        
    pc.printf("init succeed \n");
    }
time_t PCF8563::now()
{
    struct tm now;
    char start = 0x02;
    char buffer[7];
    
    if(pcf8563_i2c.write(W8563_ADDR, &start, 1) != 0) 
    {
        pc.printf("now function failed --1-- \n");
        return 0;
    }
    
    
    
    if(pcf8563_i2c.read(R8563_ADDR, buffer, 7) != 0)
    {
         pc.printf("now function failed --2-- \n");
         return 0;
    }
    
    ThisThread::sleep_for(20) ; 
    
    now.tm_sec = bcdToDecimal(buffer[0] & 0x7F);
    now.tm_min = bcdToDecimal(buffer[1] & 0x7F);
    now.tm_hour = bcdToDecimal(buffer[2] & 0x3F);
    now.tm_mday = bcdToDecimal(buffer[3] & 0x3F);
    now.tm_mon = bcdToDecimal(buffer[5] & 0x1F) - 1;
    now.tm_year = bcdToDecimal(buffer[6]) + 2000 - 1900;
    
    return mktime(&now);
}

void PCF8563::time_in_alarm()
{
    struct tm now;
    char start = 0x09;
    char buffer[4];
    
    if(pcf8563_i2c.write(W8563_ADDR, &start, 1) != 0) 
    {
        pc.printf("now function failed --1-- \n");
       
    }
    
    
    
    if(pcf8563_i2c.read(R8563_ADDR, buffer, 4) != 0)
    {
         pc.printf("now function failed --2-- \n");
        
    }
    ThisThread::sleep_for(20) ; 
    
    now.tm_sec = 0 ;
    now.tm_min = bcdToDecimal(buffer[0] & 0x7F);
    pc.printf("min is %d \n" , bcdToDecimal(buffer[0] & 0x7F));
    pc.printf("min is %.2x \n" , buffer[0]  );
    pc.printf("\n" ) ;
    now.tm_hour = bcdToDecimal(buffer[1] & 0x3F);
    pc.printf("hour is %d \n" , bcdToDecimal(buffer[1] & 0x3F));
    pc.printf("min is %.2x \n" , buffer[1]  );
    pc.printf("\n" ) ;
    now.tm_mday = bcdToDecimal(buffer[2] & 0x3F);
    pc.printf("mday is %d \n" , bcdToDecimal(buffer[2] & 0x3F));
    pc.printf("min is %.2x \n" , buffer[2]  );
    pc.printf("\n" ) ;
    
}

bool PCF8563::set_time(time_t time)
{
    struct tm *now;
    char buffer[8];
    
    now = localtime(&time);
    
    buffer[0] = 0x02; // memory address
    buffer[1] = decimalToBcd(now->tm_sec) & 0x7F; // VL = 0
    buffer[2] = decimalToBcd(now->tm_min) & 0x7F;
    buffer[3] = decimalToBcd(now->tm_hour) & 0x3F;
    buffer[4] = decimalToBcd(now->tm_mday) & 0x3F;
    buffer[5] = now->tm_wday + 1;
    buffer[6] = decimalToBcd(now->tm_mon+1) & 0x1F;
    buffer[7] = decimalToBcd(now->tm_year + 1900 - 2000);
    
    if(pcf8563_i2c.write(W8563_ADDR, buffer, 9) != 0) 
    {
        pc.printf("set_time function failed --1-- \n");
        return 0;
        }
    //ack_polling();
    ThisThread::sleep_for(20) ; 
    return true;
}


void PCF8563::check_status()
{
    char start = 0x01;
    char buffer;
    
    if(pcf8563_i2c.write(W8563_ADDR, &start, 1) != 0) 
    {
        pc.printf("check_status function failed --1-- \n");
        
    }
    ThisThread::sleep_for(20) ;
     
    if(pcf8563_i2c.read(R8563_ADDR, &buffer, 1) != 0)
    {
         pc.printf("check_status function failed --2-- \n");
        
         }
    
   // pcf8563_i2c.start();
   // pcf8563_i2c.write(R8563_ADDR);
    
   // buffer = pcf8563_i2c.read(0);
   // pcf8563_i2c.stop();
    pc.printf("check_status is %.2x\n" ,buffer );
    ThisThread::sleep_for(200) ; 
    
}
    

void PCF8563::enable_alarm()
{
    char Int[2];
    Int[0] = 0x01; // control address
    Int[1] = 0x02; // set AIE to 1 for INT pin
    if(pcf8563_i2c.write(W8563_ADDR, Int, 2) != 0) 
    {
        pc.printf("enable_alarm function failed --1-- \n");
        
    }
    pc.printf("alarm enabled \n");
    ThisThread::sleep_for(20) ;
    
}
bool PCF8563::set_alarm(time_t time)
{
    struct tm *now;
    char buffer[5];
    
    now = localtime(&time); 
    
    
    buffer[0] = 0x09; // memory address
    buffer[1] = decimalToBcd(now->tm_min) & 0x7F;
    buffer[2] = decimalToBcd(now->tm_hour) & 0x7F;
    buffer[3] = 0x80 ; //  use this code to enable mday_alarm      decimalToBcd(now->tm_mday) & 0x3F
    buffer[4] = 0x80; //   use this code to enable wday_alarm      (now->tm_wday + 1) 
    
    if(pcf8563_i2c.write(W8563_ADDR, buffer, 5) != 0) 
    {
        pc.printf("alarm function failed --1-- \n");
        return 0;
    }
    
    ThisThread::sleep_for(20) ; 
    
    return true;
}

bool PCF8563::alarmOff()
{
    char start = 0x01;
    char buffer;
    
    if(pcf8563_i2c.write(W8563_ADDR, &start, 1) != 0)
    {
         pc.printf("alarmOff function failed --1-- \n");
         return 0;
         }
         
    
    if(pcf8563_i2c.read(R8563_ADDR, &buffer, 1) != 0) 
    {
        pc.printf("alarmOff function failed --2-- \n");
        return 0;
    }
    
    
    if((buffer & 0x08) == 0x08)
    {
        buffer = buffer - 0x08; // clear AF to turn off alarm
        char send[2];
        send[0] = 0x01; //control 1 address
        send[1] = buffer;
        if(pcf8563_i2c.write(W8563_ADDR, send, 2) != 0)
        {
             pc.printf("alarmOff function failed --3-- \n");
             return 0;
        }
        pc.printf("alarm is off \n");
    }
    else
    {
        pc.printf("alarm was not setted on \n");
        }
    ThisThread::sleep_for(20) ; 
    
    return true;
}

bool PCF8563::check_alarm()
{
    char start = 0x01;
    char buffer;
    
    if(pcf8563_i2c.write(W8563_ADDR, &start, 1) != 0) 
    {
        pc.printf("check_alarm function failed --1-- \n");
        
    }
    ThisThread::sleep_for(20) ; 
    
     
    if(pcf8563_i2c.read(R8563_ADDR, &buffer, 1) != 0)
    {
         pc.printf("check_status function failed --2-- \n");
        
         }
    ThisThread::sleep_for(20) ; 
    
    if( (buffer & 0x08 )== 0x08)
    {
        pc.printf("== ALARM ON ==\r\n");
        ThisThread::sleep_for(500) ;  
        return true;
    }
    else if ( (buffer & 0x08 )== 0x00)
    {
        pc.printf("== ALARM OFF ==\r\n");
        ThisThread::sleep_for(500) ; 
        return 0;
    }
    
    ThisThread::sleep_for(20) ; 
    return 0;
    
}

void PCF8563::ack_polling(void)
{
    uint8_t buffer[] = { 0 };
    int ack;
    
    do {
        ack = pcf8563_i2c.write(W8563_ADDR, (char *)buffer, 1, true);
        pc.printf(".");
    } while(ack == 1);
    pcf8563_i2c.stop();
}