#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "mcp_can.h"
#include "mcp_can33.h"
#include <stdio.h>

//#define MDEBUG
//
//
// Hardware configuration
//

#define LED_CAN33_ORANGE 7
#define LED_CAN95_YELLOW 8
#define LED_BLUE 14
#define LED_RED 15
#define LED_WHITE 16
#define LED_YELLOW 17

const int RF_CS_PIN = 9;
const int RF_CSN_PIN = 10;
const int SPI_CS_PIN1 = 6;
const int SPI_CS_PIN2 = 5;

#ifdef MDEBUG
/*
static FILE uartout = {0};
static int uart_putchar(char c, FILE *stream)
{
  Serial.write(c);
  return 0;
}
*/
#endif

// Set up nRF24L01 radio on SPI bus plus pins 7 & 8
RF24 radio(RF_CS_PIN, RF_CSN_PIN);

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xA0A0A0A0A0LL, 0xD0D0D0D0D0LL };

const int max_payload_size = 32;
char receive_payload[max_payload_size+1]; // +1 to allow room for a terminating NULL char

  

MCP_CAN_33 CAN33(SPI_CS_PIN1);                                    // Set CS pin
MCP_CAN CAN95(SPI_CS_PIN2);                                    // Set CS pin

unsigned char flagRecv33 = 0;
unsigned char flagRecv95 = 0;

unsigned char can_msg_len = 0;
unsigned char can_msg_buf[8];

unsigned int payload_size = 4;
unsigned char flag_send_counter = 0;
unsigned char flag_send = 3;

char send_payload[4] = {0x00,0x00,0x00,0x00};

void MCP2515_ISR_33()
{
    flagRecv33 = 1;
}

void MCP2515_ISR_95()
{
    flagRecv95 = 1;
}

void setup(void)
{
#ifdef MDEBUG
   Serial.begin(115200);
#endif
   pinMode(LED_CAN33_ORANGE, OUTPUT);
   pinMode(LED_CAN95_YELLOW, OUTPUT);
   pinMode(LED_BLUE, OUTPUT);
   pinMode(LED_RED, OUTPUT);
   pinMode(LED_WHITE, OUTPUT);
   pinMode(LED_YELLOW, OUTPUT);

   digitalWrite(LED_WHITE, HIGH);
   digitalWrite(LED_YELLOW, HIGH);
   digitalWrite(LED_BLUE, HIGH);
   digitalWrite(LED_RED, HIGH);

   while (CAN_OK != CAN33.begin(CAN_33KBPS, MCP_8MHz))
   {
#ifdef MDEBUG
      Serial.println("CAN BUS Shield init fail");
      Serial.println(" Init CAN BUS Shield again");
#endif
      delay(100);
   }
 
   while (CAN_OK != CAN95.begin(CAN_33KBPS, MCP_8MHz))              // CAN_95K24BPS init can bus : baudrate = 500k
   {
#ifdef MDEBUG
      Serial.println("CAN BUS Shield init fail");
      Serial.println(" Init CAN BUS Shield again");
#endif
      delay(100);
   }

   delay(250);

#ifdef MDEBUG  
   Serial.println("CAN BUS 33 & 95 Shields init ok!");
#endif

   digitalWrite(LED_RED, LOW);
   digitalWrite(LED_WHITE, LOW);
   digitalWrite(LED_YELLOW, LOW);
   digitalWrite(LED_BLUE, LOW);
   //attachInterrupt(0, MCP2515_ISR, FALLING); // start interrupt
   CAN95.init_Mask(0, 0, 0x3ff);
   CAN95.init_Mask(1, 0, 0x2ff);
   CAN95.init_Filt(0, 0, 0x350);                          // there is filter in mcp2515
   CAN95.init_Filt(1, 0, 0x260);                          // there is filter in mcp2515
 /*  CAN95.init_Mask(0, 0, 0x5ff);
   CAN95.init_Filt(0, 0, 0x450);                          // there is filter in mcp2515
   */
   delay(100);
  
#ifdef MDEBUG
/*
   fdev_setup_stream(&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
   stdout = &uartout;
   */
#endif
   //
   // Setup and configure rf radio
   //
   radio.begin();
   // enable dynamic payloads
   radio.enableDynamicPayloads();
   radio.enableAckPayload();
   //radio.setAutoAck(1);
   // optionally, increase the delay between retries & # of retries
   radio.setRetries(5,15);
   radio.setChannel(35);
   radio.setPALevel(RF24_PA_MAX);
   //radio.setCRCLength(RF24_CRC_8)
   // Open pipes to other nodes for communication
   // This simple sketch opens two pipes for these two nodes to communicate
   // back and forth.
   // Open 'our' pipe for writing
   // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)
   radio.openWritingPipe(pipes[0]);
   radio.openReadingPipe(1,pipes[1]);
   // Start listening
   radio.startListening();

#ifdef MDEBUG  
  // Dump the configuration of the rf unit for debugging
   radio.printDetails();
#endif
}

void loop(void)
{  /*
   if(CAN_MSGAVAIL == CAN95.checkReceive())
   {
      // flagRecv95 = 0;
      digitalWrite(LED_CAN95_YELLOW, HIGH);
      CAN95.readMsgBuf(&can_msg_len, can_msg_buf);
#ifdef MDEBUG
      Serial.print("Got can2 ");
      Serial.print(CAN95.getCanId(),HEX);
      Serial.print(" msg: ");
      for(int i = 0; i<can_msg_len; i++)
      {
         Serial.print("0x");
         Serial.print(can_msg_buf[i], HEX);
         Serial.print(" ");
      }
      Serial.println();
#endif
        switch(CAN95.getCanId())
        {
            case 0x450:
               if ( can_msg_buf[3] == 0b01000111 ) // 0x47 
                   send_payload[1] |= (1<<0);
               else
                   send_payload[1] &=~ (1<<0);
               break;
            default:
               break;
         } // switch
   }// if can.checkrecieve
*/
   if(CAN_MSGAVAIL == CAN95.checkReceive())
   {
      // flagRecv33 = 0;                // clear flag
      digitalWrite(LED_CAN33_ORANGE, HIGH);
      CAN95.readMsgBuf(&can_msg_len, can_msg_buf);


#ifdef MDEBUG
      Serial.print("Got can1 ");
      Serial.print(CAN95.getCanId(),HEX);
      Serial.print(" msg: ");
      for(int i = 0; i<can_msg_len; i++) 
      {
         Serial.print("0x");
         Serial.print(can_msg_buf[i], HEX);
         Serial.print(" ");
      }
      Serial.println();    
#endif
      switch(CAN95.getCanId())
      {
         case 0x350:
            if ( can_msg_buf[0] & (1<<2) )
               send_payload[1] |= (1<<0);
            else
               send_payload[1] &=~ (1<<0);
            break;
         case 0x260:
            send_payload[1] &=~ (1<<1);
            send_payload[1] &=~ (1<<2);
            send_payload[1] &=~ (1<<3);
            if ( can_msg_buf[0] == 0b00100101 )
               send_payload[1] |= (1<<1);
            if ( can_msg_buf[0] == 0b00111010 )
               send_payload[1] |= (1<<2);
            if ( can_msg_buf[0] == 0b00011111 )
               send_payload[1] |= (1<<3);
            break;
         default:
            break;
      }// switch
   }// if can.checkrecieve
  
   if (send_payload[1] & (1<<0))
   {
      digitalWrite(LED_BLUE, HIGH);

      if ( send_payload[1] & (1<<3) )
         digitalWrite(LED_RED, HIGH); // ON
      else
         digitalWrite(LED_RED, LOW); 
      
      if( send_payload[1] & (1<<1) )
         digitalWrite(LED_WHITE, HIGH); // ON
      else      
         digitalWrite(LED_WHITE, LOW); // OFF

      if ( send_payload[1] & (1<<2) )
         digitalWrite(LED_YELLOW, HIGH); // ON
      else
         digitalWrite(LED_YELLOW, LOW); // OFF
   }
   else
   {
      digitalWrite(LED_BLUE, LOW); // OFF
      digitalWrite(LED_WHITE, LOW); // OFF
      digitalWrite(LED_YELLOW, LOW); // OFF  
      digitalWrite(LED_RED, LOW); // OFF  
   }

   // First, stop listening so we can talk.
   radio.stopListening();

    // Take the time, and send it.  This will block until complete
#ifdef MDEBUG
   // Serial.print(F("Try to send with length "));
   // Serial.println(payload_size);
#endif
   radio.flush_tx();
   radio.write( send_payload, payload_size );

   //return;
 
   // Now, continue listening
   radio.startListening();
   // Wait here until we get a response, or timeout
   unsigned long started_waiting_at = millis();
   bool timeout = false;
   while ( ! radio.available() && ! timeout )
      if (millis() - started_waiting_at > 100 )
         timeout = true;
   //digitalWrite(BLUE_LED, LOW);
   
   // Describe the results
   if ( timeout )
   {
#ifdef MDEBUG
   //   Serial.println(F("Failed, response timed out."));
#endif
   }
   else
   {
      // Grab the response, compare, and send to debugging spew
      uint8_t len = radio.getDynamicPayloadSize();
      
      // If a corrupt dynamic payload is received, it will be flushed
      if(!len)
        return; 
      
      radio.read( receive_payload, len );
      // Put a zero at the end for easy printing
      receive_payload[len] = 0;

#ifdef MDEBUG
      Serial.print(F("Got response size="));
      Serial.print(len);
      Serial.print(F(" value="));
      for(int i = 0; i<len; i++)    // print the data
      {
         Serial.print("0x");
         Serial.print(receive_payload[i], HEX);
         Serial.print(" ");
      }
      Serial.println();
#endif
   }
   // Update size for next time.    
   // Try again 1s later
   //delay(250);
   digitalWrite(LED_CAN33_ORANGE, LOW);
   digitalWrite(LED_CAN95_YELLOW, LOW);
}
// vim:cin:ai:sts=2 sw=2 ft=cpp
