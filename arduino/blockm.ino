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


#define FLOW_PORT PORTD
#define FLOW_DDR DDRD

void flowsensor_init()
{
    FLOW_DDR &=~ (1<<DD2);
    FLOW_PORT |= (1<<PIN2);
    EIMSK |= (1<<INT0);
    MCUCR |= (1<<ISC00)|(1<<ISC01);
}

//uint16_t fs_timer0_overflow_count;

ISR(TIMER0_OVF_vect)
{
    fs_timer0_overflow_count += 1;
}

void setup()
{
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
}

unsigned char stmp[8] = {0, 0, 0, 0, 0, 0, 0, 0};
void loop()
{
   unsigned int can_id = 0x123;
   unsigned int can_msg_size = 8;
    // send data:  id = 0x00, standrad frame, data len = 8, stmp: data buf
    //stmp[7] = stmp[7]+1;
    unsigned char rslen = 0;
    unsigned char rsbuf[8];

    if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
    {
        CAN.readMsgBuf(&rslen, rsbuf);    // read data,  len: data length, buf: data buf

        unsigned int canId = CAN.getCanId();
        if(canId == 0x321)
        {
            if (rsbuf[7] & 0b00000001)
            {
                    if (rsbuf[7] > 0x30)
                    stmp[3] = 0x47;
                    else
                    stmp[3] = 0x00; 
            }
        }  
    }
    
    CAN.sendMsgBuf(can_id, 0, can_msg_size, stmp);
    delay(100);                       // send data per 100ms
//////////////////////////////
  
  ds.reset(); // Начинаем взаимодействие со сброса всех предыдущих команд и параметров
  ds.write(0xCC); // Даем датчику DS18b20 команду пропустить поиск по адресу. В нашем случае только одно устрйоство 
  ds.write(0x44); // Даем датчику DS18b20 команду измерить температуру. Само значение температуры мы еще не получаем - датчик его положит во внутреннюю память
  
  delay(1000); // Микросхема измеряет температуру, а мы ждем.  
  
  ds.reset(); // Теперь готовимся получить значение измеренной температуры
  ds.write(0xCC); 
  ds.write(0xBE); // Просим передать нам значение регистров со значением температуры
// Получаем и считываем ответ
  stmp[1] = ds.read(); // Читаем младший байт значения температуры
  stmp[0] = ds.read(); // А теперь старший
/////////////////////////////////////////////
}

// END FILE
