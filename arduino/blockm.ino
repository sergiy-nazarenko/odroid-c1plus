#include <OneWire.h>
#include <Wire.h>

#include <Eeprom24C04_16.h>

// demo: CAN-BUS Shield, send data
// loovee@seeed.cc

#include <mcp_can.h>
#include <SPI.h>

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 10;

#define EPPROM_ADRESS 0x50


static Eeprom24C04_16 eeprom(0x50);

#define RELAY_PORT PORTC
#define RELAY_DDR DDRC
#define RELAY_R_PIN 3
#define RELAY_L_PIN 2

#define FLOW_PIN 2
#define FLOW_PORT PORTD
#define FLOW_DDR DDRD

#define DS18B20_PIN 8

OneWire ds(DS18B20_PIN); //  Создаем объект OneWire для шины 1-Wire, с помощью которого будет осуществляться работа с датчиком

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

volatile uint8_t radio_delay = 0;
volatile uint8_t temperature_meter = 0;
volatile uint16_t flow_total_pulse_count = 0;
uint16_t storage_flowmeter = 0;


#define M_FLOW_LO 0
#define M_FLOW_HI 1
#define M_TEMPER_LO 2
#define M_TEMPER_HI 3
#define M_WIPES_CNT 5
#define M_STATE 6
#define M_VARS 7


unsigned char recieve_can_len = 0;
unsigned char recieve_can_buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte lo, hi;
unsigned int can_id = 0x123;
unsigned int can_msg_size = 8;
unsigned char can_buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned int can2_id = 0x150;
unsigned int can2_msg_size = 8;
unsigned char can2_buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};


void flow_pulse()
{
   radio_delay=0;
   flow_total_pulse_count++; //Every time this function is called, increment "count" by 1
}


ISR(TIMER1_COMPA_vect)
{
    radio_delay++;
    temperature_meter++;
}

byte hilo[2] = { 0 };

void setup()
{
    pinMode(FLOW_PIN, INPUT);           //Sets the pin as an input
    attachInterrupt(0, flow_pulse, RISING);  //Configures interrupt 0 (pin 2 on the Arduino Uno) to run the function "Flow"  
     //Serial.begin(115200);
    while (CAN_OK != CAN.begin(CAN_50KBPS, MCP_8MHz))                 // init can bus : baudrate = 500k
    //while (CAN_OK != CAN.begin(CAN_95K24BPS, MCP_8MHz))              // init can bus : baudrate = 500k
    {
        //Serial.println("CAN BUS Shield init fail");
        //Serial.println(" Init CAN BUS Shield again");
        delay(100);
    }
    //Serial.println("CAN BUS Shield init ok!");

    temperature_meter = 0;
    ds.reset(); // Начинаем взаимодействие со сброса всех предыдущих команд и параметров
    ds.write(0xCC); // Даем датчику DS18b20 команду пропустить поиск по адресу. В нашем случае только одно устрйоство 
    ds.write(0x44); // Даем датчику DS18b20 команду измерить температуру. Само значение температуры мы еще не получаем - датчик его положит во внутреннюю память
    
    cli();//stop interrupts
    //set timer1 interrupt at 1Hz
    TCCR1A = 0;// set entire TCCR1A register to 0
    TCCR1B = 0;// same for TCCR1B
    TCNT1  = 0;//initialize counter value to 0
    OCR1A = 15624; // 16 000 000 / 1024 //This flag is set in the timer clock cycle after the counter 
    
    TCCR1B |= (1 << WGM12);
    TIMSK1 |= (1 << OCIE1A);
    TCCR1B |= (1 << CS12) | (1 << CS10);
    // set prescaler to 1024 and start the timer
    sei();
    
    eeprom.initialize();
    eeprom.readBytes(0, 2, hilo);
    flow_total_pulse_count = (hilo[0] << 8) | hilo[1];
    storage_flowmeter = flow_total_pulse_count;
}

void loop()
{
    if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
    {
        CAN.readMsgBuf(&recieve_can_len, recieve_can_buffer);    // read data,  len: data length, buf: data buf

        unsigned int recieve_canid = CAN.getCanId();
        if(recieve_canid == 0x321)
        {
            if ((recieve_can_buffer[1] >> 0) & 1UL )
            {
                can_buffer[M_STATE] |= (1<<0);
                can_buffer[M_STATE] &=~ (1<<1);
                can_buffer[M_STATE] &=~ (1<<2);
                can_buffer[M_STATE] &=~ (1<<3);

                if ( (recieve_can_buffer[1] >> 3) & 1UL )
                {
                    RELAY_PORT &=~ (1<<RELAY_L_PIN); // ON
                    RELAY_PORT &=~ (1<<RELAY_R_PIN); // ON
                    can_buffer[M_STATE] |= (1<<3);
                }
                else if( (recieve_can_buffer[1] >> 1) & 1UL )
                {
                    RELAY_PORT &=~ (1<<RELAY_L_PIN); // ON
                    RELAY_PORT |= (1<<RELAY_R_PIN); // OFF
                    can_buffer[M_STATE] |= (1<<1);
                }
                else if ( (recieve_can_buffer[1] >> 2) & 1UL )
                {
                    RELAY_PORT &=~ (1<<RELAY_R_PIN); // ON 
                    RELAY_PORT |= (1<<RELAY_L_PIN); // OFF
                    can_buffer[M_STATE] |= (1<<2);
                }
                else
                {
                    RELAY_PORT |= (1<<RELAY_L_PIN); // OFF
                    RELAY_PORT |= (1<<RELAY_R_PIN); // OFF
                }
            }
            else
            {
                RELAY_PORT &=~ (1<<RELAY_R_PIN);
                RELAY_PORT &=~ (1<<RELAY_L_PIN);
                can_buffer[M_STATE] &=~ (1<<0);
                can_buffer[M_STATE] &=~ (1<<1);
                can_buffer[M_STATE] &=~ (1<<2);
            }


            if (!((recieve_can_buffer[0] >> 0) & 1UL))
            {
              if(((can_buffer[M_VARS] >> 0) & 1UL))
            { // inverse can_buffer[7] ^= 1UL << 0;
                flow_total_pulse_count = 0;
                byte flow_state[6] = {0};          
                eeprom.readBytes(0, 6, flow_state);
                // lo << 8 | hi
                uint16_t flow_clean_counter = (flow_state[4] << 8) | flow_state[5];
                if ( flow_clean_counter >= 0xFFFA )
                {
                  flow_clean_counter = 0;
                }
                flow_clean_counter++;
                lo = flow_clean_counter >> 8;
                hi = flow_clean_counter;
                                
                can2_buffer[0] = lo;
                can2_buffer[1] = hi;
                flow_state[0] = 0x00;
                flow_state[1] = 0x00;
                flow_state[2] = 0x00;
                flow_state[3] = 0x00;
                flow_state[4] = 0x00;
                flow_state[5] = 0x00;
                
                eeprom.writeBytes(0, 6,  flow_state);
                
                can_buffer[M_VARS] &=~ (1UL << 0);
                can_buffer[M_STATE] &=~ (1UL << 7);
            }}
            if ((recieve_can_buffer[0] >> 0) & 1UL)
            {
                can_buffer[M_VARS] |= (1UL << 0);
                can_buffer[M_STATE] |= (1UL << 7);
            }
        }  
    }


    if ( flow_total_pulse_count >= 0xFFFA )
    {
        can_buffer[M_STATE] |= (1<<4);
        can_buffer[M_FLOW_LO] = 0xFF;
        can_buffer[M_FLOW_HI] = 0xFF;
    }
    else
    {
      can_buffer[M_STATE] &=~ (1<<4);
        
        lo = flow_total_pulse_count >> 8;
        hi = flow_total_pulse_count;
        can_buffer[M_FLOW_LO] = lo;
        can_buffer[M_FLOW_HI] = hi;

        if((flow_total_pulse_count > storage_flowmeter) &&
           (radio_delay >= 20))
        {
           storage_flowmeter = flow_total_pulse_count;
           byte cln[2] = {lo,hi};
           eeprom.writeBytes(0, 2,  cln);
        }
        else if (radio_delay >= 20) 
        {
          radio_delay = 0;
        }
    }
 //////////////////////////////
    if(temperature_meter >= 5) // delay(1000); // Микросхема измеряет температуру, а мы ждем.
    {
        ds.reset(); // Теперь готовимся получить значение измеренной температуры
        ds.write(0xCC); 
        ds.write(0xBE); // Просим передать нам значение регистров со значением температуры
        // Получаем и считываем ответ
        can_buffer[M_TEMPER_LO] = ds.read(); // Читаем младший байт значения температуры
        can_buffer[M_TEMPER_HI] = ds.read(); // А теперь старший
        can_buffer[M_STATE] &=~ (1<<5);
        
        temperature_meter = 0;
        ds.reset(); // Начинаем взаимодействие со сброса всех предыдущих команд и параметров
        ds.write(0xCC); // Даем датчику DS18b20 команду пропустить поиск по адресу. В нашем случае только одно устрйоство 
        ds.write(0x44); // Даем датчику DS18b20 команду измерить температуру. Само значение температуры мы еще не получаем - датчик его положит во внутреннюю память
    }
    else
    {
      can_buffer[M_STATE] |= (1<<5);
    }
/////////////////////////////////////////////

    delay(100); 

    CAN.sendMsgBuf(can_id, 0, can_msg_size, can_buffer);

    // delay(50);


    CAN.sendMsgBuf(can2_id, 0, can2_msg_size, can2_buffer);
}

// END FILE
