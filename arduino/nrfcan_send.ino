/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Example using Dynamic Payloads 
 *
 * This is an example of how to use payloads of a varying (dynamic) size. 
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "mcp_can.h"
#include <stdio.h>

//#define MDEBUG
//
//
// Hardware configuration
//
#define BLUE_LED 5
#define WHITE_LED 7
#define YELLOW_LED 6

#ifdef MDEBUG
static FILE uartout = {0};
static int uart_putchar(char c, FILE *stream)
{
  Serial.write(c);
  return 0;
}
#endif

// Set up nRF24L01 radio on SPI bus plus pins 7 & 8
const int RF_CS_PIN = 9;
RF24 radio(RF_CS_PIN, 10);

// sets the role of this unit in hardware.  Connect to GND to be the 'pong' receiver
// Leave open to be the 'ping' transmitter
const int role_pin = 5;


//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xA0A0A0A0A0LL, 0xD0D0D0D0D0LL };

const int min_payload_size = 4;
const int max_payload_size = 32;
const int payload_size_increments_by = 1;
int next_payload_size = min_payload_size;

char receive_payload[max_payload_size+1]; // +1 to allow room for a terminating NULL char

const int SPI_CS_PIN = 4;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

unsigned char flagRecv = 0;
unsigned char can_msg_len = 0;
unsigned char can_msg_buf[8];

unsigned char flag_send_counter = 0;
unsigned char flag_send = 3;

char send_payload[4] = {0x00,0x00,0x00,0x00};


void MCP2515_ISR()
{
    flagRecv = 1;
}

void setup(void)
{

#ifdef MDEBUG
  Serial.begin(9600);
#endif
  pinMode(WHITE_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  digitalWrite(WHITE_LED, HIGH);
  digitalWrite(YELLOW_LED, HIGH);
  digitalWrite(BLUE_LED, HIGH);
  
    while (CAN_OK != CAN.begin(CAN_33KBPS, MCP_8MHz))              // init can bus : baudrate = 500k
    {
#ifdef MDEBUG
        Serial.println("CAN BUS Shield init fail");
        Serial.println(" Init CAN BUS Shield again");
#endif
        delay(100);
    }
    delay(500);
#ifdef MDEBUG  
    Serial.println("CAN BUS Shield init ok!");
#endif
    digitalWrite(WHITE_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(BLUE_LED, LOW);

//    attachInterrupt(0, MCP2515_ISR, FALLING); // start interrupt
  CAN.init_Mask(0, 0, 0x5ff);
  CAN.init_Mask(1, 0, 0x5ff);
  CAN.init_Filt(0, 0, 0x450);                          // there is filter in mcp2515
  CAN.init_Filt(1, 0, 0x350);                          // there is filter in mcp2515
  CAN.init_Filt(2, 0, 0x260);                          // there is filter in mcp2515
  delay(100);
  // Role
  //
  // set up the role pin
  pinMode(role_pin, INPUT);
  digitalWrite(role_pin,HIGH);//HIGH);
  delay(20); // Just to get a solid reading on the role pin
  
#ifdef MDEBUG
  fdev_setup_stream(&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
  stdout = &uartout;
  
  Serial.println(F("RF24/examples/pingpair_dyn/"));
 
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
  
  //
  // Open pipes to other nodes for communication
  //

  // This simple sketch opens two pipes for these two nodes to communicate
  // back and forth.
  // Open 'our' pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
  //
  // Start listening
  //

  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
#ifdef MDEBUG  
  radio.printDetails();
#endif
}

void loop(void)
{
  //
  // Ping out role.  Repeatedly send the current time
  //
   unsigned char len = 0;
   unsigned char buf[8];

    // The payload will always be the same, what will change is how much of it we send.
    unsigned int payload_size = 4;
    //if(flagRecv)                   // check if get data
    if(CAN_MSGAVAIL == CAN.checkReceive())
    {
        //flagRecv = 0;                // clear flag
        CAN.readMsgBuf(&can_msg_len, can_msg_buf);    // read data,  len: data length, buf: data buf
        if(CAN.getCanId() == 0x450)
        {
            digitalWrite(WHITE_LED, HIGH);
#ifdef MDEBUG        
            Serial.println("\r\n-----------");
            Serial.print("Get Data From id: 0x");
            Serial.print(CAN.getCanId(),HEX);
            Serial.println();
            Serial.print(" ");
            Serial.print(can_msg_len);
            Serial.print(" ");
#endif
            if (can_msg_len == 4 && flag_send_counter >= flag_send)
              send_payload[0] = can_msg_buf[3];
            
            if (send_payload[0] != can_msg_buf[3])
                flag_send_counter++;
            else
                flag_send_counter = 0;
            
            if (send_payload[0] == 0x47)
              digitalWrite(BLUE_LED, HIGH);         
            else
              digitalWrite(BLUE_LED, LOW);         
        
#ifdef MDEBUG
            for(int i = 0; i<can_msg_len; i++)    // print the data
            {
                Serial.print("0x");
                Serial.print(can_msg_buf[i], HEX);
                Serial.print(" ");
            }
            Serial.println();
#endif
        }// if can.getid() == 450
        if(CAN.getCanId() == 0x350) 
        {
          if ( can_msg_buf[0] & (1<<2) )
          {
              digitalWrite(BLUE_LED, HIGH);
              send_payload[1] |= (1<<0);
          }
          else
          {
              digitalWrite(BLUE_LED, LOW);
              send_payload[1] &=~ (1<<0);
          }
        }
        if(CAN.getCanId() == 0x260)
        {
          digitalWrite(WHITE_LED, HIGH);
          if ( can_msg_buf[0] == 0b00100101 )
              send_payload[1] |= (1<<1);
          else
              send_payload[1] &=~ (1<<1);

          if ( can_msg_buf[0] == 0b00111010 )
              send_payload[1] |= (1<<2);
          else
              send_payload[1] &=~ (1<<2);

          if ( can_msg_buf[0] == 0b00011111 )
          {
              digitalWrite(YELLOW_LED, HIGH);
              send_payload[1] |= (1<<3);
          }
          else
          {
              digitalWrite(YELLOW_LED, LOW);
              send_payload[1] &=~ (1<<3);
          }
          
        }
        
    }// if can.checkrecieve
    /*else
    {
        flag_send_counter = 0;
    }*/
    
    // First, stop listening so we can talk.
    radio.stopListening();

    // Take the time, and send it.  This will block until complete
#ifdef MDEBUG
    Serial.print(F("Now sending length "));
    Serial.println(payload_size);
#endif
    radio.flush_tx();
    radio.write( send_payload, payload_size );

    // Now, continue listening
    radio.startListening();

    // Wait here until we get a response, or timeout
    unsigned long started_waiting_at = millis();
    bool timeout = false;
    while ( ! radio.available() && ! timeout )
      if (millis() - started_waiting_at > 500 )
        timeout = true;
    digitalWrite(WHITE_LED, LOW);
    digitalWrite(BLUE_LED, LOW);
    
    // Describe the results
    if ( timeout )
    {
#ifdef MDEBUG
      Serial.println(F("Failed, response timed out."));
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
      // Spew it
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
    delay(500);
}
// vim:cin:ai:sts=2 sw=2 ft=cpp
