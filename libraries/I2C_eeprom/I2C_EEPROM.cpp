//
//    FILE: I2C_eeprom.cpp
//  AUTHOR: Rob Tillaart
// VERSION: 0.2.00
// PURPOSE: Simple I2C_eeprom library for Arduino with EEPROM 24LC256 et al.
//
// HISTORY:
// 0.1.00 - 2011-01-21 initial version
// 0.1.01 - 2011-02-07 added setBlock function
// 0.2.00 - 2011-02-11 fixed 64 bit boundary bug
//
// Released to the public domain
//

#include <I2C_eeprom.h>

#include "Wstring.h"
#include "Arduino.h"

////////////////////////////////////////////////////////////////////
//
// PUBLIC
//

I2C_eeprom::I2C_eeprom(unsigned char device)
{
    _Device = device;
    Wire.begin();       // initialise the connection
}

void I2C_eeprom::writeByte(unsigned int address, unsigned char data )
{
    _WriteBlock(address, &data, 1);
}

void I2C_eeprom::writeBlock(unsigned int address, unsigned char* buffer, int length)
{
    // determine length until end of page
    int le = endOfPage(address);
    if (le > 0)
    {
        _WriteBlock(address, buffer, le);
        address += le;
        buffer += le;
        length -= le;
    }

    // write the rest at BLOCKSIZE (16) byte boundaries
    while (length > 0)
    {
        _WriteBlock(address, buffer, min(length, BLOCKSIZE));
        address += BLOCKSIZE;
        buffer += BLOCKSIZE;
        length -= BLOCKSIZE;
    }
}

void I2C_eeprom::setBlock(unsigned int address, unsigned char data, int length)
{
    unsigned char buffer[BLOCKSIZE];
    for (unsigned char i =0; i< BLOCKSIZE; i++) buffer[i] = data;

    // determine length until end of page
    int le = endOfPage(address);
    if (le > 0)
    {
        _WriteBlock(address, buffer, le);
        address += le;
        length -= le;
    }

    while (length > 0)
    {
        _WriteBlock(address, buffer, min(length, BLOCKSIZE));
        address += BLOCKSIZE;
        length -= BLOCKSIZE;
    }
}


unsigned char I2C_eeprom::readByte(unsigned int address)
{
    unsigned char rdata;
    _ReadBlock(address, &rdata, 1);
    return rdata;
}

// maybe let's not read more than 30 or 32 unsigned chars at a time!
void I2C_eeprom::readBlock(unsigned int address, unsigned char* buffer, int length)
{
    while (length > 0)
    {
        _ReadBlock(address, buffer, min(length, BLOCKSIZE));
        address += BLOCKSIZE;
        buffer += BLOCKSIZE;
        length -= BLOCKSIZE;
    }
}

////////////////////////////////////////////////////////////////////
//
// PRIVATE
//


// detemines length until first multiple of 16 of an address
// so writing allways occurs up to 16 byte boundaries
// this is automatically 64 byte boundaries
int I2C_eeprom::endOfPage(unsigned int address)
{
    const int m = BLOCKSIZE;
    unsigned int eopAddr = ((address + m - 1) / m) * m;  // "end of page" address
    return eopAddr - address;  // length until end of page
}

// pre: length < 32;
void I2C_eeprom::_WriteBlock(unsigned int address, unsigned char* buffer, unsigned char length)
{
    Wire.beginTransmission(_Device);
    Wire.write((int)(address >> 8));
    Wire.write((int)(address & 0xFF));
    for (unsigned char c = 0; c < length; c++)
    Wire.write(buffer[c]);
    Wire.endTransmission();
    delay(5);
}

// pre: buffer is large enough to hold length bytes
void I2C_eeprom::_ReadBlock(unsigned int address, unsigned char* buffer, unsigned char length)
{
    Wire.beginTransmission(_Device);
    Wire.write((int)(address >> 8));
    Wire.write((int)(address & 0xFF));
    Wire.endTransmission();
    Wire.requestFrom(_Device, length);
    for (int c = 0; c < length; c++ )
    if (Wire.available()) buffer[c] = Wire.read();
}
//
// END OF FILE
//
