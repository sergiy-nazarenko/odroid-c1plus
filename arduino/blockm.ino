#include <OneWire.h>
#include <Wire.h>


// demo: CAN-BUS Shield, send data
// loovee@seeed.cc

#include <mcp_can.h>
#include <SPI.h>

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 10;

OneWire ds(8); //  Создаем объект OneWire для шины 1-Wire, с помощью которого будет осуществляться работа с датчиком

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

#define RELAY_PORT PORTB
#define RELAY_DDR DDRB
#define RELAY_R_PIN 6
#define RELAY_L_PIN 7

#define FLOW_PIN 2
#define FLOW_PORT PORTD
#define FLOW_DDR DDRD

void EEPROM_WriteByte( byte Address, byte data)
{
    Wire.beginTransmission(0xA0);
    Wire.write(Address);
    Wire.write(data);
    delay(5); //Не знаю точно, но в Datasheet описана задержка записи в 5мс, поправьте меня, если я не прав.
    Wire.endTransmission();
}

byte EEPROM_ReadByte(byte Address) 
{
    byte rdata = 0xFF;
    Wire.beginTransmission(0xA0);
    Wire.write(Address);
    Wire.endTransmission();
    Wire.requestFrom(0xA0, 1);
    if (Wire.available()) rdata = Wire.read();
    return rdata;
}

volatile uint8_t radio_delay = 0;
volatile uint8_t temperature_meter = 0;
volatile uint16_t flow_total_pulse_count = 0;
uint16_t storage_flowmeter = 0;
unsigned char can_buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned char recieve_can_len = 0;
unsigned char recieve_can_buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte lo, hi;
unsigned int can_id = 0x123;
unsigned int can_msg_size = 8;
// send data:  id = 0x00, standrad frame, data len = 8, stmp: data buf
    


void flow_pulse()
{
   flow_total_pulse_count++; //Every time this function is called, increment "count" by 1
}


ISR(TIMER1_COMPA_vect)
{
    radio_delay++;
    temperature_meter++;
}


void setup()
{
    TCCR1B |= (1 << WGM12); // configure timer1 for CTC mode
    TIMSK1 |= (1 << OCIE1A); // enable the CTC interrupt
    sei();
    OCR1A = 15625; // 16 000 000 / 1024 //This flag is set in the timer clock cycle after the counter 
    TCCR1B |= ((1 << CS12) | (1 << CS10)); // start the timer at 16MHz/1024

    pinMode(FLOW_PIN, INPUT);           //Sets the pin as an input
    attachInterrupt(0, flow_pulse, RISING);  //Configures interrupt 0 (pin 2 on the Arduino Uno) to run the function "Flow"  
 
    byte lo = EEPROM_ReadByte(0);
    byte hi = EEPROM_ReadByte(1);


    flow_total_pulse_count = (lo << 8) | hi;
    storage_flowmeter = flow_total_pulse_count;
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
                can_buffer[6] |= (1<<0);
                can_buffer[6] &=~ (1<<1);
                can_buffer[6] &=~ (1<<2);
                can_buffer[6] &=~ (1<<3);

                if ( (recieve_can_buffer[1] >> 3) & 1UL )
                {
                    RELAY_PORT &=~ (1<<RELAY_L_PIN); // ON
                    RELAY_PORT &=~ (1<<RELAY_R_PIN); // ON
                    can_buffer[6] |= (1<<3);
                }
                else if( (recieve_can_buffer[1] >> 1) & 1UL )
                {
                    RELAY_PORT &=~ (1<<RELAY_L_PIN); // ON
                    RELAY_PORT |= (1<<RELAY_R_PIN); // OFF
                    can_buffer[6] |= (1<<1);
                }
                else if ( (recieve_can_buffer[1] >> 2) & 1UL )
                {
                    RELAY_PORT &=~ (1<<RELAY_R_PIN); // ON 
                    RELAY_PORT |= (1<<RELAY_L_PIN); // OFF
                    can_buffer[6] |= (1<<2);
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
                can_buffer[6] &=~ (1<<0);
                can_buffer[6] &=~ (1<<1);
                can_buffer[6] &=~ (1<<2);
            }


            if ( (recieve_can_buffer[7] >> 0) & 1UL)
            {
                can_buffer[3] = 0x00; 
            }
            
            if (((recieve_can_buffer[0] >> 0) & 1UL) && 
                    ((can_buffer[7] >> 0) & 1UL))
            { // inverse can_buffer[7] ^= 1UL << 0;
                can_buffer[7] |= (1UL << 0);
                EEPROM_WriteByte(0, 0x00);
                EEPROM_WriteByte(1, 0x00);
          //    EEPROM_WriteByte(2, 0x00);
          //    EEPROM_WriteByte(3, 0x00);

                lo = EEPROM_ReadByte(4);
                hi = EEPROM_ReadByte(5);
                uint16_t flow_clean_counter = (lo << 8) | hi;
                flow_clean_counter++;
                lo = flow_clean_counter >> 8;
                hi = flow_clean_counter;
                EEPROM_WriteByte(4, lo);
                EEPROM_WriteByte(5, hi);
            }
            if ((recieve_can_buffer[0] >> 1) & 1UL)
            {
                can_buffer[7] &= ~(1UL << 0);
            }
        }  
    }

    lo = flow_total_pulse_count >> 8;
    hi = flow_total_pulse_count;
    can_buffer[0] = lo;
    can_buffer[1] = hi;

    if((flow_total_pulse_count > storage_flowmeter) &&
      (radio_delay >= 70))
    {
        storage_flowmeter = flow_total_pulse_count;
        EEPROM_WriteByte(0, lo);
        EEPROM_WriteByte(1, hi);
    }
 //////////////////////////////
    if(temperature_meter >= 5) // delay(1000); // Микросхема измеряет температуру, а мы ждем.
    {
        ds.reset(); // Теперь готовимся получить значение измеренной температуры
        ds.write(0xCC); 
        ds.write(0xBE); // Просим передать нам значение регистров со значением температуры
        // Получаем и считываем ответ
        can_buffer[2] = ds.read(); // Читаем младший байт значения температуры
        can_buffer[3] = ds.read(); // А теперь старший

        temperature_meter = 0;
        ds.reset(); // Начинаем взаимодействие со сброса всех предыдущих команд и параметров
        ds.write(0xCC); // Даем датчику DS18b20 команду пропустить поиск по адресу. В нашем случае только одно устрйоство 
        ds.write(0x44); // Даем датчику DS18b20 команду измерить температуру. Само значение температуры мы еще не получаем - датчик его положит во внутреннюю память
    }
/////////////////////////////////////////////

    delay(100); 

    CAN.sendMsgBuf(can_id, 0, can_msg_size, can_buffer);
}

// END FILE
