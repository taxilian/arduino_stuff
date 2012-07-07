
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <I2C_eeprom.h>

#define k_MODE_Open 0
#define k_MODE_Pending 1
#define k_MODE_Writing 2
#define k_BlockSize 32

#define k_FileCountAddr 0
#define k_FileDirAddr k_FileCountAddr+1

#define BUFSIZE 100

#define DEBUG 1

const byte EEPROM_ID = 0x50;      // I2C address for 24LC256 EEPROM

// Enter a MAC address, IP address and Portnumber for your Server below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Initialize the Ethernet server library
// with the IP address and port you want to use
EthernetServer server(80);
I2C_eeprom ee(EEPROM_ID);
LiquidCrystal lcd(9, 8, 6, 5, 3, 2);

byte buffer[k_BlockSize];

struct FSBlock {
  word addr;
  word length;
  char name[28];
};

void writeBlock(EthernetClient& client, byte* data, word len) {
//  while (len-- > 0) {
//    client.write(*(data++));
//  }
  for (word i = 0; i < len; ++i) {
    client.write(data[i]);
    Serial.write(data[i]);
  }
}

struct FSBlock* findFile(const char* filename) {
  byte fCount = ee.readByte(k_FileCountAddr);
  FSBlock* block = new FSBlock;
  
  for (byte n = 0; n < fCount; ++n) {
    // First read the file descriptor
    int addr = k_FileDirAddr + (n * (k_BlockSize));
    ee.readBlock(addr, (byte*)block, k_BlockSize);
    Serial.print("Examining file: "); Serial.println(block->name);
    if (strcmp(block->name, filename) == 0) {
      // We found our file!
      Serial.println("File found!");
      return block;
    }
  }
  // We didn't find anything
  delete block;
  return NULL;
}

void writeFile(FSBlock* file, EthernetClient& client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/plain");
  client.println();
  byte block[k_BlockSize];
  
  Serial.print("Returning file: "); Serial.println(file->name);
  Serial.print("Size: "); Serial.println(file->length);
  Serial.print("Address: "); Serial.println(file->addr);
  
  // Loop over all blocks in the file
  for (word pos = 0; pos < file->length; pos += k_BlockSize) {
    // Calculate the absolute position on the chip
    word addr = pos + file->addr;
    // Calculate the length of the block to read; this will always be k_BlockSize
    // except on the last read. Read the block
    word len = min(file->length - pos, k_BlockSize);
    ee.readBlock(addr, block, len);
    
    writeBlock(client, block, len);
  }
}

void setup()
{
  // start the serial for debugging
  Serial.begin(115200);
  
  // start the LCD
  lcd.begin(16, 2);
  
  byte fCount = ee.readByte(k_FileCountAddr);
  Serial.print("Filesystem scanned; ");
  Serial.print((int)fCount);
  Serial.println(" files found");
  
  // start the Ethernet connection and the server:
  Serial.println("Fetching DHCP address....");
  lcd.print("Getting DHCP...");
  Ethernet.begin(mac);
  lcd.clear();
  lcd.print(Ethernet.localIP());
  
  lcd.setCursor(0, 1);
  lcd.print((int)fCount);
  lcd.print(" files found");
  
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
  
  server.begin();
}

int address = 0;
int bufferOffset = 0;

void loop()
{
  char clientline[BUFSIZE];
  byte index = 0;
  
  EthernetClient client = server.available();
  if (client) {
    index = 0;
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c != '\n' && c != '\r' && index < BUFSIZE) {
          // Read 'til buffer is full or EOL
          clientline[index++] = c;
          continue;
        }
        Serial.println("Handling a request");
        
#if DEBUG
        Serial.print("client available bytes before flush: "); Serial.println(client.available());
        Serial.print("request = "); Serial.println(clientline);
#endif

        // Flush any remaining bytes from the client buffer
        client.flush();

#if DEBUG
        // Should be 0
        Serial.print("client available bytes after flush: "); Serial.println(client.available());
#endif

        String request = String(clientline);
        // Get the request method
        String method = request.substring(0, request.indexOf(' '));
        byte startIdx = request.indexOf('/');
        request = request.substring(startIdx, request.indexOf(' ', startIdx));
        
        request.toCharArray(clientline, BUFSIZE);

#if DEBUG
        // Should be 0
        Serial.print("Client method: "); Serial.println(method);
        Serial.print("Client request: "); Serial.println(clientline);
#endif
        
        char* slashPos = strchr(clientline+1, '/');
        char* firstSection = strtok(clientline, "/");
        Serial.print("section: "); Serial.println(firstSection);
        Serial.print("Slashpos: "); Serial.println(slashPos);
        if (strcmp(firstSection, "files") == 0 && slashPos != NULL) {
          char* filename = slashPos + 1;
          Serial.print("File request: "); Serial.println(filename);
          
          FSBlock* file = findFile(filename);
          if (file) {
            writeFile(file, client);
          }
        } else {
          Serial.println("Unknown request");
          client.println("HTTP/1.1 404 Not Found");
          client.println();
          client.print("Could not find ");
          client.println(clientline);
        }
      }
      delay(1);
      client.stop();
      while (client.status() != 0) { delay(5); }
    }
  }
}


