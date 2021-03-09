#include <bcm2835.h>  
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "ad_i2c.h"

#define I2C_ADDR 0x18

// private functions
uint8_t ads7142_read_reg(uint8_t addr, uint8_t reg);
void ads7142_write_reg(uint8_t addr, uint8_t reg, uint8_t data);
uint16_t ads7142_get_sample(uint8_t addr, uint8_t chnl);

// ---------------------------------
uint8_t setup_ad()  
{ 
    uint8_t i2c_data;
  
    if (!bcm2835_init()) return 0;

    bcm2835_i2c_begin();                //Start I2C operations.
    bcm2835_i2c_set_baudrate(100000);    //1M baudrate

    //Enable 2 channel single ended config - CH_INPUT_CFG register
    ads7142_write_reg(I2C_ADDR, 0x24, 0x00);
    i2c_data = ads7142_read_reg(0x18, 0x24);
    printf("CH_INPUT_CFG set to: 0x%02X should be 0x%02X\n", i2c_data, 0x00);

    //Set device for two channel AUTO sequence - OPMODE_SEL 
    ads7142_write_reg(I2C_ADDR, 0x1C, 0x04);
    i2c_data = ads7142_read_reg(0x18, 0x1C);
    printf("OPMODE set to: 0x%02X should be 0x%02X\n\n", i2c_data, 0x04);

    return 1;
}

// ---------------------------------
uint16_t read_lvl(uint8_t chnl)  
{ 
    //Read data
    uint16_t ch_data = ads7142_get_sample(I2C_ADDR, chnl);
    return ch_data; 
}

// ---------------------------------
uint16_t ads7142_get_sample(uint8_t addr, uint8_t chnl){

  char buf[2];
  uint16_t data = -99;  

  buf[0] = 0;
  buf[1] = 0;

  bcm2835_i2c_setSlaveAddress(addr);  //I2C address
    
  //Set channel select AUTO_SEQ_REG 
  ads7142_write_reg(addr, 0x20, chnl);
    
  //Enable sampling START_SEQUENCE 
  ads7142_write_reg(addr, 0x1E, 0x01);
  
  //Read in a sample  
  bcm2835_i2c_read(buf,2);

  //Disable sampling ABORT_SEQUENCE
  ads7142_write_reg(addr, 0x1F, 0x01);

  data = (buf[0]<<8) | buf[1];

  data = data>>4; 

  //printf("buf0: 0x%02x, buf1: 0x%02x\n", buf[0], buf[1]);

  return data;
}

// ---------------------------------
uint8_t ads7142_read_reg(uint8_t addr, uint8_t reg){

  char buf[3];	

  bcm2835_i2c_setSlaveAddress(addr);  //I2C address

  buf[0] = 0x10;    //single byte read opcode
  buf[1] = reg;    
  bcm2835_i2c_write(buf,2);
    
  bcm2835_i2c_read(buf,1);

  return buf[0];
}

void ads7142_write_reg(uint8_t addr, uint8_t reg, uint8_t data){

  char buf[3];	

  bcm2835_i2c_setSlaveAddress(addr);  //I2C address

  buf[0] = 0x08;    //single byte read opcode
  buf[1] = reg;    
  buf[2] = data;
  bcm2835_i2c_write(buf,3);

}

// ---------------------------------
void close_ad() {    
    bcm2835_i2c_end();  
    bcm2835_close();  
} 

