#include "eeprom.h"

eeprom::eeprom(I2C &i2c_obj, char address)
    : i2c(i2c_obj), adr(address)
{
    init();
}

void eeprom::init(void)
{
    i2c.frequency(400*1000);
}

void eeprom::write_address(int address, int address_size, bool repeated)
{   
    uint8_t buffer[4] = { 0, 0, 0, 0 };
    page_write(address, address_size, buffer, 0);
}

uint8_t eeprom::current_read(void)
{
    uint8_t data;
    i2c.read((int)adr,(char *)&data,1);
    return data;
}

void eeprom::sequential_read(uint8_t *buffer, int buffer_size)
{
    i2c.read((int)adr, (char *)buffer, buffer_size);   
}

void eeprom::random_read(int address, int address_size, uint8_t *buffer, int buffer_size)
{
    write_address(address, address_size, true);
    i2c.read((int)adr, (char *)buffer, buffer_size);
}

void eeprom::byte_write(int address, int address_size, uint8_t data, bool repeated)
{   
    uint8_t buffer[4] = { 0, 0, 0, 0 };
    buffer[3] = data;
    page_write(address, address_size, buffer, 1);
}

void eeprom::page_write(int address, int address_size, uint8_t *buffer, int buffer_size, bool repeated)
{
    // for simplify, buffer must prefix 3 bytes for memory address
    switch (address_size) {
        case ONE_BYTE_ADDRESS:
            buffer[2] = address & 0xFF;
            break;
        case TWO_BYTES_ADDRESS:
            buffer[1] = (address >> 8) & 0xFF;
            buffer[2] = address & 0xFF;
            break;
        case THREE_BYTES_ADDRESS:
            buffer[0] = (address >> 16) & 0xFF;
            buffer[1] = (address >> 8) & 0xFF;
            buffer[2] = address & 0xFF;
            break;
    }
    i2c.write((int)adr,(char *)(buffer + (3 - address_size)),address_size + buffer_size,repeated);
}

void eeprom::ack_polling(void)
{
    uint8_t buffer[] = { 0 };
    int ack;
    
    do {
        ack = i2c.write((int)adr, (char *)buffer, 1, true);
    } while(ack == 1);
    i2c.stop();
}
void eeprom::reset(void)
{
    uint8_t buffer[50] = { 0, 0, 0, 0 , 0 ,0 ,0 ,0 ,0 ,0 ,0, 0, 0, 0 , 0 ,0 ,0 ,0 ,0 ,0,0, 0, 0, 0 , 0 ,0 ,0 ,0 ,0 ,0,0, 0, 0, 0 , 0 ,0 ,0 ,0 ,0 ,0,0, 0, 0, 0 , 0 ,0 ,0 ,0 ,0 ,0 }; 
    uint8_t buffer_[18] = { 0, 0, 0, 0 , 0 ,0 ,0 ,0,0, 0, 0, 0 , 0 ,0 ,0 ,0 ,0 ,0 } ;
    for(int i =0 ; i < EEPROM_SIZE - 18 ; i+=25)
    {
        page_write(i ,  TWO_BYTES_ADDRESS , buffer ,  25) ;
        ack_polling();
    }   
    
    page_write( EEPROM_SIZE - 18 ,  TWO_BYTES_ADDRESS , buffer_ ,  18) ;
    ack_polling();
        
}

void eeprom::var_write_8(int address, int address_size, uint8_t data, bool repeated)
{   
    uint8_t buffer[4] = { 0, 0, 0, 0 };
    buffer[3] = data;
    page_write(address, address_size, buffer, 1);
}

void eeprom::var_write_16(int address, int address_size, int16_t variable ,int size , bool repeated)
{
    uint8_t buffer[11] = { 0, 0, 0, 0 , 0 , 0 ,0 ,0 ,0 ,0 ,0};
    data16 data;
    data.i16 = variable;
    buffer[3] = data.b[0]; 
    buffer[4] = data.b[1];
    page_write(address, address_size, buffer, 2);
    
    }
void eeprom::var_write_32(int address, int address_size, int32_t variable ,int size , bool repeated)
{
    uint8_t buffer[11] = { 0, 0, 0, 0 , 0 , 0 ,0 ,0 ,0 ,0 ,0};
    data32 data2;
            data2.i32 = variable;
            buffer[3] = data2.b[0]; 
            buffer[4] = data2.b[1];
            buffer[5] = data2.b[2];
            buffer[6] = data2.b[3];
    page_write(address, address_size, buffer, 4);
            
    }
void eeprom::var_write_64(int address, int address_size, int64_t variable ,int size , bool repeated)
{
        uint8_t buffer[11] = { 0, 0, 0, 0 , 0 , 0 ,0 ,0 ,0 ,0 ,0};
        data64 data3;
            data3.i64 = variable;
            buffer[3] = data3.b[0]; 
            buffer[4] = data3.b[1];
            buffer[5] = data3.b[2];
            buffer[6] = data3.b[3];
            buffer[7] = data3.b[4]; 
            buffer[8] = data3.b[5];
            buffer[9] = data3.b[6];
            buffer[10]= data3.b[7];
        page_write(address, address_size, buffer, 8);
    }
    

void eeprom::var_write_8_AckPoll(int address, int address_size, uint8_t data, bool repeated)
{   uint8_t buffer[1] = { 0 };
    i2c.start();
    byte_write(address, address_size, data);
    i2c.start();
    ack_polling();
}

void eeprom::var_write_16_AckPoll(int address, int address_size, int16_t variable ,int size , bool repeated)
{
    uint8_t buffer[1] = { 0 };
    data16 data;
    data.i16 = variable;
    i2c.start();
    ack_polling();
    byte_write(address, address_size, data.b[0]);
    i2c.start();
    ack_polling();
    byte_write(address+1, address_size, data.b[1]);
    i2c.start();
    ack_polling();
    }
void eeprom::var_write_32_AckPoll(int address, int address_size, int32_t variable ,int size , bool repeated)
{
    uint8_t buffer[1] = { 0 };
    data32 data2;
    data2.i32 = variable;
    i2c.start();
    ack_polling();
    byte_write(address, address_size, data2.b[0]);
    i2c.start();
    ack_polling();
    byte_write(address+1, address_size, data2.b[1]);
    i2c.start();
    ack_polling();
    byte_write(address+2, address_size, data2.b[2]);
    i2c.start();
    ack_polling();
    byte_write(address+3, address_size, data2.b[3]);
    i2c.start();
    ack_polling();
    
            
    }
void eeprom::var_write_64_AckPoll(int address, int address_size, int64_t variable ,int size , bool repeated)
{
    uint8_t buffer[1] = { 1 };
    data64 data3;
    i2c.start();
    ack_polling();
    byte_write(address, address_size, data3.b[0]);
    i2c.start();
    ack_polling();
    byte_write(address+1, address_size, data3.b[1]);
    i2c.start();
    ack_polling();
    byte_write(address+2, address_size, data3.b[2]);
    i2c.start();
    ack_polling();
    byte_write(address+3, address_size, data3.b[3]);
    i2c.start();
    ack_polling();
    byte_write(address+4, address_size, data3.b[4]);
    i2c.start();
    ack_polling();
    byte_write(address+5, address_size, data3.b[5]);
    i2c.start();
    ack_polling();
    byte_write(address+6, address_size, data3.b[6]);
    i2c.start();
    ack_polling();
    byte_write(address+7, address_size, data3.b[7]);
    i2c.start();
    ack_polling();

    }

int8_t eeprom::var_read_8(int address , int address_size ,int buffer_size )
{
    uint8_t buffer[8] = { 0, 0, 0, 0 , 0 , 0 ,0 ,0 };
    write_address(address, address_size, true);
    i2c.read((int)adr, (char *)buffer, buffer_size);
    return buffer[0] ;
}
  
int16_t eeprom::var_read_16(int address , int address_size ,int buffer_size )
{
    uint8_t buffer[8] = { 0, 0, 0, 0 , 0 , 0 ,0 ,0 };
    data16 data;
            write_address(address, address_size, true);
            i2c.read((int)adr, (char *)buffer, buffer_size);
            data.b[0] = buffer[0] ;
            data.b[1] = buffer[1] ;
            return data.i16 ;
    }
int32_t eeprom::var_read_32(int address , int address_size ,int buffer_size )
{
    uint8_t buffer[8] = { 0, 0, 0, 0 , 0 , 0 ,0 ,0 };
    data32 data2;
            write_address(address, address_size, true);
            i2c.read((int)adr, (char *)buffer, buffer_size);
            data2.b[0] = buffer[0] ;
            data2.b[1] = buffer[1] ;
            data2.b[2] = buffer[2] ;
            data2.b[3] = buffer[3] ;
            return data2.i32 ;
            
    }
int64_t eeprom::var_read_64(int address , int address_size ,int buffer_size )
{
    uint8_t buffer[8] = { 0, 0, 0, 0 , 0 , 0 ,0 ,0 };
    data64 data3;
            write_address(address, address_size, true);
            i2c.read((int)adr, (char *)buffer, buffer_size);
            data3.b[0] = buffer[0] ;
            data3.b[1] = buffer[1] ;
            data3.b[2] = buffer[2] ;
            data3.b[3] = buffer[3] ;
            data3.b[4] = buffer[4] ;
            data3.b[5] = buffer[5] ;
            data3.b[6] = buffer[6] ;
            data3.b[7] = buffer[7] ;
            return data3.i64 ;
    }
