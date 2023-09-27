/** A class for eeprom accesss operation
 *
 *  @author  Poushen Ou
 *  @version 1.0
 *  @date    15-Jun-2018
 *
 *  This code provide classic access operation for I2C EEPROM
 *
 *  About I2C EEPROM 24FC256:
 *      http://ww1.microchip.com/downloads/en/DeviceDoc/21203M.pdf
 */
#ifndef _EEPROM_H_
#define _EEPROM_H_

#include "mbed.h"

#define EEPROM_ADDR 0xA0
#define EEPROM_SIZE 32768
/** eeprom Class Library
 * to provide very simple interface for mbed
 *
 * Example:
 * @code
 *#include "mbed.h"
#include "eeprom.h"
 
#define BUFFER_SIZE  64
 
// make eeprom instance using I2C object.
// with default slave address 0xA0 (0x50 in 7bit format)
// test ok with 24FC256 EEPROM

I2C i2c(p11,p12);       // I2C device sda, scl
eeprom epm(i2c);
#define BAUDRATE 9600
Serial pc(p6, p8 , BAUDRATE);      // tx    rx

// import!!! must prefix 3 bytes for memory address
uint8_t buffer[BUFFER_SIZE + 3]; 

int main()
    {
      pc.printf("LPC1114 demo.\n\r");
 
      // -------- page write --------------------
      for (int i=0; i<BUFFER_SIZE; i++)
          buffer[i+3] = i;     // for prefix 3 memory address
 
      epm.page_write(12288, TWO_BYTES_ADDRESS, buffer, BUFFER_SIZE);
      //wait(0.008);
      epm.ack_polling();
 
      // -------- current read ------------------
      pc.printf("below shold be 00 01 ... 3f\n\r");
      epm.write_address(12288, TWO_BYTES_ADDRESS);
  
      for (int i=0; i<BUFFER_SIZE; i++) {
        pc.printf("%.2x ", epm.current_read());
      }
      pc.printf("\n\r");
      
      // -------- byte write in ACKNOWLEDGE POLLING mode--------------------
      epm.byte_write(12280, TWO_BYTES_ADDRESS, 0xAB);
      i2c.start();
      epm.ack_polling();
      epm.byte_write(12281, TWO_BYTES_ADDRESS, 0xAC);
      i2c.start();
      epm.ack_polling();
      epm.byte_write(12282, TWO_BYTES_ADDRESS, 0xAE);
      i2c.start();
      epm.ack_polling();
      epm.byte_write(12283, TWO_BYTES_ADDRESS, 0xAD);
      epm.ack_polling();
      
      // -------- sequential read ---------------
      pc.printf("below shold be ab ac ae ad\n\r");
      epm.write_address(12280, TWO_BYTES_ADDRESS);
    
      uint8_t data[4];
      epm.sequential_read(data, 4);
      for (int i=0; i<4; i++) {
          pc.printf("%.2x ", data[i]);
      }
      pc.printf("\n\r");
 
      // -------- byte write --------------------
      epm.byte_write(12289, TWO_BYTES_ADDRESS, 0xBB);
      epm.ack_polling();
 
      // -------- random read -------------------
      pc.printf("below shold be aa bb 02 03 ... 3f\n\r");
      epm.random_read(12288, TWO_BYTES_ADDRESS, buffer, 64);
      for (int i=0; i<64; i++) {
          pc.printf("%.2x ", buffer[i]);
      }
      pc.printf("\n\r");
      
      
      
      // ------- var write------------
      pc.printf("write diffrent variables \n\r");
      int32_t a;
      epm.var_write_8(100 ,TWO_BYTES_ADDRESS , 12 );
      epm.ack_polling();
      epm.var_write_16(101 ,TWO_BYTES_ADDRESS , -1200 );
      epm.ack_polling();
      epm.var_write_32(104 ,TWO_BYTES_ADDRESS , 1206200 );
      epm.ack_polling();
      // -------Read  var ------------
      pc.printf("below shold be c fffffb50 1267B8   \n\r");
      a = (int32_t)epm.var_read_8(100 ,TWO_BYTES_ADDRESS) ;
      pc.printf("%.2x \r", a);
      a = (int32_t)epm.var_read_16(101 ,TWO_BYTES_ADDRESS) ;
      pc.printf("%.2x \r", a);
      a = (int32_t)epm.var_read_32(104 ,TWO_BYTES_ADDRESS) ;
      pc.printf("%.2x \n\r", a);
      
      
      // -------  write in acknowledge polling mode------------
      pc.printf("write in acknowledge polling mode \n\r");
      
      epm.var_write_8_AckPoll(50 ,TWO_BYTES_ADDRESS , 120 );
      epm.var_write_16_AckPoll(51 ,TWO_BYTES_ADDRESS , -500 );
      epm.var_write_32_AckPoll(55 ,TWO_BYTES_ADDRESS , 64646 );
      // -------Read  var ------------
      pc.printf("below shold be 78 fffffe0c FC86   \n\r");
      a = (int32_t)epm.var_read_8(50 ,TWO_BYTES_ADDRESS) ;
      pc.printf("%.2x \r", a);
      a = (int32_t)epm.var_read_16(51 ,TWO_BYTES_ADDRESS) ;
      pc.printf("%.2x \r", a);
      a = (int32_t)epm.var_read_32(55 ,TWO_BYTES_ADDRESS) ;
      pc.printf("%.2x \n\r", a);
      
      pc.printf("\n\r");
      
      
      // ------- reset ---------------------
      
      pc.printf("now we reset the EEPROM\n") ;
      epm.reset(); 
      
      pc.printf("below shold be 00 00 00 00 ... 00\n\r");
      
      epm.write_address(12288, TWO_BYTES_ADDRESS);
      uint8_t data_[BUFFER_SIZE];
      epm.sequential_read(data_, BUFFER_SIZE);
      for (int i=0; i<BUFFER_SIZE; i++) {
          pc.printf("%.2x ", data_[i]);
      }
      pc.printf("\n\r");
      
      //while(1);
}
 * @endcode
 */
class eeprom
{
public:
    /** Create a eeprom instance connected to specified I2C pins with specified address
     *
     * @param i2c_obj I2C object (instance)
     * @param address (option) I2C-bus slave address (default: 0xA0)
     */
    eeprom(I2C &i2c_obj, char address = EEPROM_ADDR);

    /** Initialization */
    void init(void);
    
    /** Write address with specify address size
     *  
     * @param address eeprom memory address
     * @param address_size should be 1 - 3(ONE_BYTE_ADDRESS, TWO_BYTES_ADDRESS, THREE_BYTES_ADDRESS)
     */
    void write_address(int address, int address_size, bool repeated=false);

    /** Current read, read the current memory data
     *
     * @return the current memory data
     */
    uint8_t current_read(void);
    
    /** Sequential read, read one or many bytes from current memory address
     *
     * @param buffer the start address point to buffer
     * @param buffer_size the length of buffer
     */
    void sequential_read(uint8_t *buffer, int buffer_size);
    
    /** Random read, read one or more memory data from assign memory address
     *
     * @param address eeprom memory address
     * @param address_size should be 1 - 3 (ONE_BYTE_ADDRESS, TWO_BYTES_ADDRESS, THREE_BYTES_ADDRESS)
     * @param buffer the start address point to buffer
     * @param buffer_size the length of buffer
     */
     void random_read(int address, int address_size, uint8_t *buffer, int buffer_size);
     
    /** byte write, write one byte to assign memory address
     *
     * @param address eeprom memory address
     * @param address_size should be 1 - 3(ONE_BYTE_ADDRESS, TWO_BYTES_ADDRESS, THREE_BYTES_ADDRESS)
     * @param data the byte data to write
     */
    void byte_write(int address, int address_size, uint8_t data, bool repeated=false);
    
    /** page write, write many bytes to assign memory address
     * , do not deal with page size and aligned problem, be careful to use!!!
     *
     * @param address eeprom memory address
     * @param address_size should be 1 - 3(ONE_BYTE_ADDRESS, TWO_BYTES_ADDRESS, THREE_BYTES_ADDRESS)
     * @param buffer the page data to write
     */
    void page_write(int address, int address_size, uint8_t *buffer, int buffer_size, bool repeated=false);
    
    /** ack polling, wait for EEPROM write completed */
    void ack_polling(void);
    
    /** reset all addresses form 0 to 32768  */
    void reset(void);
    
    /** write 1 byte variable in eeprom*/
    void var_write_8(int address, int address_size, uint8_t data, bool repeated=false);
    /** write 2 byte variable in eeprom*/
    void var_write_16(int address, int address_size, int16_t variable ,int size = 2 , bool repeated=false) ;
    /** write 4 byte variable in eeprom*/
    void var_write_32(int address, int address_size, int32_t variable ,int size = 4 , bool repeated=false) ;
    /** write 8 byte variable in eeprom*/
    void var_write_64(int address, int address_size, int64_t variable ,int size = 8 , bool repeated=false) ;
    
    /** write 1 byte variable in eeprom*/
    void var_write_8_AckPoll(int address, int address_size, uint8_t data, bool repeated=false);
    /** write 2 byte variable in eeprom*/
    void var_write_16_AckPoll(int address, int address_size, int16_t variable ,int size = 2 , bool repeated=false) ;
    /** write 4 byte variable in eeprom*/
    void var_write_32_AckPoll(int address, int address_size, int32_t variable ,int size = 4 , bool repeated=false) ;
    /** write 8 byte variable in eeprom*/
    void var_write_64_AckPoll(int address, int address_size, int64_t variable ,int size = 8 , bool repeated=false) ;
    
    /**read 1 byte variable in eeprom  */
    int8_t var_read_8(int address , int address_size ,int buffer_size = 1  ) ;
    /**read 2 byte variable in eeprom  */
    int16_t var_read_16(int address , int address_size ,int buffer_size = 2  ) ;
    /**read 4 byte variable in eeprom  */
    int32_t var_read_32(int address , int address_size ,int buffer_size = 4  ) ;
    /**read 8 byte variable in eeprom  */
    int64_t var_read_64(int address , int address_size ,int buffer_size = 8  ) ;
    
    
private:
    I2C &i2c;
    char adr;
};

typedef union
    {
        uint16_t u16;
        int16_t i16;
        uint8_t b[2];   //b[0] = LSB      , b[1] = MSB 
    }data16;

typedef union
    {
        uint32_t u32;
        int32_t i32;
        uint8_t b[4];   
    }data32;
    
typedef union
    {
        uint64_t u64;
        int64_t i64;
        uint8_t b[8];   
    }data64;


enum {
    ONE_BYTE_ADDRESS = 1,
    TWO_BYTES_ADDRESS,
    THREE_BYTES_ADDRESS
};

#endif