// demo: CAN-BUS Shield, send data
// loovee@seeed.cc

#include <mcp_can.h>
#include <SPI.h>

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 10;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

void setup()
{
    Serial.begin(115200);

    while (CAN_OK != CAN.begin(CAN_95K24BPS, MCP_8MHz))              // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println(" Init CAN BUS Shield again");
        delay(100);
    }
    Serial.println("CAN BUS Shield init ok!");
}

unsigned char stmp[8] = {0, 0, 0, 0, 0, 0, 0, 0};
void loop()
{
  unsigned int can_id = 0x123;
   unsigned int can_msg_size = 8;
    // send data:  id = 0x00, standrad frame, data len = 8, stmp: data buf
    stmp[7] = stmp[7]+1;
    
    if(stmp[7] == 100)
    {
        stmp[7] = 0; 
        stmp[6] = stmp[6] + 1;
        
        if(stmp[6] == 100)
        {
            stmp[6] = 0;
            stmp[5] = stmp[6] + 1;
        }
    }
    
    if (stmp[7] > 0x30 && stmp[7] & 0b00000010)
      {
        can_id = 0x450;
        can_msg_size = 4;
        stmp[3] = 0x47;
      }
  else if (stmp[7] & 0b00000001)
{
        can_id = 0x450;
        can_msg_size = 4;
        if (stmp[7] > 0x30)
        stmp[3] = 0x47;
        else
        stmp[3] = 0x00; 
}  
    
    CAN.sendMsgBuf(can_id, 0, can_msg_size, stmp);
    delay(100);                       // send data per 100ms
}

// END FILE
