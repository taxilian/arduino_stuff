#include <I2C_eeprom.h>

#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>

#define k_MODE_Open 0
#define k_MODE_Pending 1
#define k_MODE_Writing 2
#define k_BlockSize 32

const byte EEPROM_ID = 0x50;      // I2C address for 24LC256 EEPROM

// Enter a MAC address, IP address and Portnumber for your Server below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
int serverPort=8001;

// Initialize the Ethernet server library
// with the IP address and port you want to use
EthernetServer server(serverPort);

I2C_eeprom ee(EEPROM_ID);

void setup()
{
  // start the serial for debugging
  Serial.begin(115200);
  // start the Ethernet connection and the server:
  Ethernet.begin(mac);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

void print_mode(int mode) {
  Serial.print("Server is currently in ");
  switch(mode) {
    case k_MODE_Open:
      Serial.print("Open");
      break;
    case k_MODE_Pending:
      Serial.print("Pending");
      break;
    case k_MODE_Writing:
      Serial.print("Writing");
      break;
    default:
      Serial.print("Unknown");
      break;
  }
  Serial.println(" mode.");
}

int mode = 0;
int address = 0;
int bufferOffset = 0;

void loop()
{
  byte buffer[k_BlockSize] = { 0 };
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    String clientMsg ="";
    while (client.connected()) {
      if (client.available()) {
        print_mode(mode);
        byte c = client.read();
        switch(mode) {
          case k_MODE_Open:
            if (c == '~') { mode = k_MODE_Pending; }
            else { Serial.print(c); }
            break;
          case k_MODE_Pending:
            if (c == 'w') { mode = k_MODE_Writing; }
            break;
          case k_MODE_Writing:
            // This is where the interesting stuff happens
            if (bufferOffset < k_BlockSize) {
              // Store the new byte in the buffer
              buffer[bufferOffset++] = c;
            } else {
              // Write the buffer and flush
              Serial.print("Writing block to eeprom: ");
              Serial.println(address);
              ee.writeBlock(address, buffer, bufferOffset);
              address += bufferOffset;
              bufferOffset = 0;
            }
            break;
          default:
            Serial.println("Unknown mode");
        }
      }
    }
    if (bufferOffset > 0) {
      // data is not aligned on buffer boundaries; that's fine. Write whatever we have left
      Serial.println("Writing final buffer to eeprom");
      ee.writeBlock(address, buffer, bufferOffset);
      address += bufferOffset;
      bufferOffset = 0;
    }
    // give the Client time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
}


