#include <OneWire.h>



// demo: CAN-BUS Shield, send data
// loovee@seeed.cc

#include <mcp_can.h>
#include <SPI.h>

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 10;
OneWire ds(8); // Создаем объект OneWire для шины 1-Wire, с помощью которого будет осуществляться работа с датчиком


MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

#define EEPROM_1 0x50
#define FLOW_PORT PORTD
#define FLOW_DDR DDRD

void flowsensor_init()
{
    FLOW_DDR &=~ (1<<DD2);
    FLOW_PORT |= (1<<PIN2);
    EIMSK |= (1<<INT0);
    MCUCR |= (1<<ISC00)|(1<<ISC01);
}
//http://forum.amperka.ru/threads/%D0%A0%D0%B0%D0%B1%D0%BE%D1%82%D0%B0-%D1%81-%D0%B2%D0%BD%D0%B5%D1%88%D0%BD%D0%B5%D0%B9-eeprom-24cxx.7231/
void EEPROM_WriteByte(byte Address, byte data)
{
  Wire.beginTransmission(EEPROM_1);
  Wire.write(Address);
  Wire.write(data);
  delay(5); //Не знаю точно, но в Datasheet описана задержка записи в 5мс, поправьте меня, если я не прав.
  Wire.endTransmission();
}

byte EEPROM_ReadByte(byte Address) 
{
  byte rdata = 0xFF;
  Wire.beginTransmission(EEPROM_1);
  Wire.write(Address);
  Wire.endTransmission();
  Wire.requestFrom(EEPROM_1, 1);
  if (Wire.available()) rdata = Wire.read();
  return rdata;
}

unsigned int flow_data;

void setup()
{
    flow_data = 0;
    TCCR1B |= (1 << WGM12); // configure timer1 for CTC mode
    TIMSK1 |= (1 << OCIE1A); // enable the CTC interrupt

    // timer0 code
    TIMSK0 |= (1<<TOIE0);
    OCR0A = 0xFF;
    TCNT0 = 0x00; // 7812 // two times in seconds // 30 times of 256
    TCCR0A |= (1<<CS02) | (1<<CS00);
    //timer0 end code
    sei();
    OCR1A = 15625; // 16 000 000 / 1024 //This flag is set in the timer clock cycle after the counter 
    TCCR1B |= ((1 << CS12) | (1 << CS10)); // start the timer at 16MHz/1024

    //Serial.begin(115200);
    while (CAN_OK != CAN.begin(CAN_50KBPS, MCP_8MHz))                 // init can bus : baudrate = 500k

    //while (CAN_OK != CAN.begin(CAN_95K24BPS, MCP_8MHz))              // init can bus : baudrate = 500k
    {
        //Serial.println("CAN BUS Shield init fail");
        //Serial.println(" Init CAN BUS Shield again");
        delay(100);
    }
    //Serial.println("CAN BUS Shield init ok!");


    byte flow_data_lo = EEPROM_ReadByte(0);
    byte flow_data_hi = EEPROM_ReadByte(1);

    flow_data = ((flow_data_hi << 8) | flow_data_lo)
}

unsigned char can_buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};

void loop()
{
    unsigned int can_id = 0x123;
    unsigned int can_msg_size = 8;
    // send data:  id = 0x00, standrad frame, data len = 8, stmp: data buf
    //stmp[7] = stmp[7]+1;
    unsigned char receive_len = 0;
    unsigned char receive_buf[8];

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ds.reset(); // Начинаем взаимодействие со сброса всех предыдущих команд и параметров
    ds.write(0xCC); // Даем датчику DS18b20 команду пропустить поиск по адресу. В нашем случае только одно устрйоство 
    ds.write(0x44); // Даем датчику DS18b20 команду измерить температуру. Само значение температуры мы еще не получаем - датчик его положит во внутреннюю память

    delay(100); // Микросхема измеряет температуру, а мы ждем.  

    ds.reset(); // Теперь готовимся получить значение измеренной температуры
    ds.write(0xCC); 
    ds.write(0xBE); // Просим передать нам значение регистров со значением температуры
    // Получаем и считываем ответ
    can_buffer[0] = ds.read(); // Читаем младший байт значения температуры
    can_buffer[1] = ds.read(); // А теперь старший

    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    if (current_flow_data > flow_data)
    {
        flow_data = current_flow_data;
        EEPROM_WriteByte(0, flow_data_lo);
        EEPROM_WriteByte(1, flow_data_hi);
    }

    can_buffer[2] = flow_data_lo; // Читаем младший байт значения температуры
    can_buffer[3] = flow_data_hi; // А теперь старший
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
    {
        CAN.readMsgBuf(&receive_len, receive_buf);    // read data,  len: data length, buf: data buf
        unsigned int receive_can_id = CAN.getCanId();
        if(receive_can_id == 0x321)
        {
            if (receive_buf[7] & 0b00000001)
            {
                if (receive_buf[7] > 0x30)
                    can_buffer[4] = 0x47;
                else
                    can_buffer[5] = 0x00; 
            }
            if (receive_buf[6] == 0x01)
            {
                flow_data = current_flow_data = 0;
                EEPROM_WriteByte(0, 0x00);
                EEPROM_WriteByte(1, 0x00);
            }
        }  
    }

    CAN.sendMsgBuf(can_id, 0, can_msg_size, can_buffer);

   
}

// END FILE
