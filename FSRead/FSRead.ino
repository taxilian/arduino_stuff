#include <I2C_eeprom.h>

#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>

#define k_MODE_Open 0
#define k_MODE_Pending 1
#define k_MODE_Writing 2
#define k_BlockSize 32

#define k_FileCountAddr 0
#define k_FileDirAddr k_FileCountAddr+1

const byte EEPROM_ID = 0x50;      // I2C address for 24LC256 EEPROM

// Enter a MAC address, IP address and Portnumber for your Server below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
int serverPort=8001;

// Initialize the Ethernet server library
// with the IP address and port you want to use
EthernetServer server(serverPort);

I2C_eeprom ee(EEPROM_ID);

byte buffer[k_BlockSize];

void setup()
{
  // start the serial for debugging
  Serial.begin(115200);
  
  byte fCount = ee.readByte(k_FileCountAddr);
  Serial.print("Filesystem scanned; ");
  Serial.print((int)fCount);
  Serial.println(" files found");
  
  for (byte n = 0; n < fCount; ++n) {
    // First read the file descriptor
    int addr = k_FileDirAddr + (n * (k_BlockSize));
    Serial.print("Reading from address: "); Serial.println(addr);
    ee.readBlock(addr, buffer, k_BlockSize);
    
    int fileAddr = *((int *)buffer);
    int fileSize = *((int *)(buffer+2));
    char* fileName = (char*)buffer+4;
    
    Serial.print("Name: ");
    Serial.print(fileName);
    Serial.print(", size: ");
    Serial.print(fileSize);
    Serial.print(" bytes, addr: ");
    Serial.println(fileAddr);
  }
}

int address = 0;
int bufferOffset = 0;

void loop()
{

}


