#ifndef I2C_EEPROM_H
#define I2C_EEPROM_H
//
//    FILE: I2C_eeprom.h
//  AUTHOR: Rob Tillaart
// PURPOSE: Simple I2C_eeprom library for Arduino with EEPROM 24LC256 et al.
// VERSION: 0.2.00
//     URL: http://arduino.cc/playground/Main/LibraryForI2CEEPROM
// HISTORY: See I2C_eeprom.cpp
//
// Released to the public domain
//

#include <Wire.h>

// BLOCKSIZE must be 16
#define BLOCKSIZE 16

#define I2C_EEPROM_VERSION "0.2"

// interface
class I2C_eeprom
{
    public:
    // (I2C address)
    I2C_eeprom(unsigned char);
    // (mem_address, value)
    void writeByte(unsigned int, unsigned char );
    // (mem_address, buffer, length)
    void writeBlock(unsigned int, unsigned char*, int );
    // (mem_address, value, count)
    void setBlock(unsigned int, unsigned char, int );
    // (mem_address)
    unsigned char readByte(unsigned int );
    // (mem_address, buffer, length)
    void readBlock(unsigned int, unsigned char*, int );

    private:
    unsigned char _Device;
    // (address)
    int endOfPage(unsigned int);
    // (mem_address, buffer, length)
    void _WriteBlock(unsigned int, unsigned char*, unsigned char );
    void _ReadBlock(unsigned int, unsigned char*, unsigned char );
};

#endif
// END OF FILE
