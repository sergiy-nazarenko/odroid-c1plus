#include "nrf24l01p_defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


#ifdef MDEBUG
    #include "uart/uart.h"
    #define UART_BAUDRATE (F_CPU/16/9600)-1
#endif

#define SPI_DDR DDRB
#define SPI_SS 4
#define SPI_MOSI 5
#define SPI_SCK 7

void spi_init()
{
    SPI_DDR |= (1<<SPI_MOSI) | (1<<SPI_SCK) | (1<<SPI_SS);
    SPCR = (1<<SPE) | (1<<MSTR);
}

uint8_t spi_send_recv(uint8_t data)
{
    SPDR = data;
    while(!(SPSR&(1<<SPIF)));
    return SPDR;
}

#define RADIO_PORT PORTB
#define RADIO_DDR DDRB
#define RADIO_PIN PINB
#define RADIO_CSN 4
#define RADIO_CE 3
#define RADIO_IRQ 2


#define RELAY_PORT PORTA
#define RELAY_R_PIN 4
#define RELAY_L_PIN 5
#define BLINK_PIN 3

// Выбирает активное состояние (высокий уровень) на линии CE
inline void radio_assert_ce() 
{
  RADIO_PORT |= (1 << RADIO_CE); // Установка высокого уровня на линии CE
}

// Выбирает неактивное состояние (низкий уровень) на линии CE
inline void radio_deassert_ce() 
{
  RADIO_PORT &= ~(1 << RADIO_CE); // Установка низкого уровня на линии CE  
}

// Поскольку функции для работы с csn не предполагается использовать в иных файлах, их можно объявить static

// Выбирает активное состояние (низкий уровень) на линии CSN
inline static void csn_assert() 
{
  RADIO_PORT &= ~(1 << RADIO_CSN); // Установка низкого уровня на линии CSN
}

// Выбирает неактивное состояние (высокий уровень) на линии CSN
inline static void csn_deassert() 
{
  RADIO_PORT |= (1 << RADIO_CSN); // Установка высокого уровня на линии CSN
}

// Инициализирует порты 
void radio_init() 
{
  RADIO_DDR |= (1 << RADIO_CSN) | (1 << RADIO_CE); // Ножки CSN и CE на выход
  RADIO_DDR &= ~(1 < RADIO_IRQ); // IRQ - на вход
  csn_deassert();
  radio_deassert_ce();
  spi_init();
}

// Выполняет команду cmd, и читает count байт ответа, помещая их в буфер buf, возвращает регистр статуса
uint8_t radio_read_buf(uint8_t cmd, uint8_t * buf, uint8_t count) 
{
  csn_assert();
  uint8_t status = spi_send_recv(cmd);
  while (count--) {
    *(buf++) = spi_send_recv(0xFF);
  }
  csn_deassert();
  return status;
}

// Выполняет команду cmd, и передаёт count байт параметров 
// из буфера buf, возвращает регистр статуса
uint8_t radio_write_buf(uint8_t cmd, uint8_t * buf, uint8_t count) 
{
    csn_assert();
    uint8_t status = spi_send_recv(cmd);
    while (count--) {
        spi_send_recv(*(buf++));
    }
    csn_deassert();
    return status;
}

// Читает значение однобайтового регистра reg (от 0 до 31) 
// и возвращает его
uint8_t radio_readreg(uint8_t reg) 
{
    csn_assert();
    spi_send_recv((reg & 31) | R_REGISTER);
    uint8_t answ = spi_send_recv(0xFF);
    csn_deassert();
    return answ;
}

// Записывает значение однобайтового регистра reg (от 0 до 31), 
// возвращает регистр статуса
uint8_t radio_writereg(uint8_t reg, uint8_t val) 
{
  csn_assert();
  uint8_t status = spi_send_recv((reg & 31) | W_REGISTER);
  spi_send_recv(val);
  csn_deassert();
  return status;
}

// Читает count байт многобайтового регистра reg (от 0 до 31) и сохраняет его в буфер buf,
// возвращает регистр статуса
uint8_t radio_readreg_buf(uint8_t reg, uint8_t * buf, uint8_t count) {
  return radio_read_buf((reg & 31) | R_REGISTER, buf, count);
}

// Записывает count байт из буфера buf 
// в многобайтовый регистр reg (от 0 до 31), возвращает регистр статуса
uint8_t radio_writereg_buf(uint8_t reg, uint8_t * buf, uint8_t count) {
  return radio_write_buf((reg & 31) | W_REGISTER, buf, count);
}

// Возвращает размер данных в начале FIFO очереди приёмника
uint8_t radio_read_rx_payload_width() 
{
  csn_assert();
  spi_send_recv(R_RX_PL_WID);
  uint8_t answ = spi_send_recv(0xFF);
  csn_deassert();
  return answ;
}

// Выполняет команду. Возвращает регистр статуса
uint8_t radio_cmd(uint8_t cmd) 
{
  csn_assert();
  uint8_t status = spi_send_recv(cmd);
  csn_deassert();
  return status;
}

// Возвращает 1, если на линии IRQ активный (низкий) уровень.
uint8_t radio_is_interrupt() 
{
  return (RADIO_PIN & RADIO_IRQ) ? 0 : 1;
}

/*
uint8_t radio_is_interrupt() {
// использовать этот вариант только в крайних случаях!!!
  return (radio_cmd(NOP) & ((1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT))) ? 1 : 0;
}
*/

// Функция производит первоначальную настройку устройства. 
// Возвращает 1, в случае успеха, 0 в случае ошибки
uint8_t radio_start() 
{
    uint8_t self_addr[] = {0xD0, 0xD0, 0xD0, 0xD0, 0xD0}; // Собственный адрес
    uint8_t remote_addr[] = {0xA0, 0xA0, 0xA0, 0xA0, 0xA0}; // Адрес удалённой стороны
    uint8_t chan = 35; // Номер радио-канала (в диапазоне 0 - 125)

    radio_deassert_ce();
    for(uint8_t cnt = 100;;) 
    {
        radio_writereg(CONFIG, (1 << EN_CRC) | (1 << CRCO) | (1 << PRIM_RX)); // Выключение питания
        if (radio_readreg(CONFIG) == ((1 << EN_CRC) | (1 << CRCO) | (1 << PRIM_RX))) 
            break;
        // Если прочитано не то что записано,
        //  то значит либо радио-чип ещё инициализируется, либо не работает.
        if (!cnt--)
            return 0; // Если после 100 попыток не удалось записать что нужно, то выходим с ошибкой
        _delay_ms(1);
    }

    radio_writereg(EN_AA, (1 << ENAA_P0) ); // включение автоподтверждения только по каналу 0
    radio_writereg(EN_RXADDR, (1 << ERX_P0)|(1 << ERX_P1) ); // включение каналов 0 и 1
    radio_writereg(SETUP_AW, SETUP_AW_5BYTES_ADDRESS); // выбор длины адреса 5 байт
    radio_writereg(SETUP_RETR, SETUP_RETR_DELAY_1000MKS | SETUP_RETR_UP_TO_5_RETRANSMIT); 
    radio_writereg(RF_CH, chan); // Выбор частотного канала
    radio_writereg(RF_SETUP, RF_SETUP_1MBPS | RF_SETUP_0DBM); // выбор скорости 1 Мбит/с и мощности 0dBm

    radio_writereg_buf(RX_ADDR_P0, &remote_addr[0], 5); // Подтверждения приходят на канал 0 
    radio_writereg_buf(TX_ADDR, &remote_addr[0], 5);

    radio_writereg_buf(RX_ADDR_P1, &self_addr[0], 5);

    radio_writereg(RX_PW_P0, 32);
    //radio_writereg(RX_PW_P1, 0); 
    radio_writereg(DYNPD, (1 << DPL_P0)|(1 << DPL_P1)); // включение произвольной длины для каналов 0 и 1
    radio_writereg(FEATURE, 0x04); // разрешение произвольной длины пакета данных

    radio_writereg(CONFIG, (1 << EN_CRC) | (1 << CRCO) | (1 << PWR_UP) | (1 << PRIM_RX)); // Включение питания
    return (radio_readreg(CONFIG) == ((1 << EN_CRC) | (1 << CRCO) | (1 << PWR_UP) | (1 << PRIM_RX))) ? 1 : 0;
}

// Вызывается, когда превышено число попыток отправки, 
// а подтверждение так и не было получено.
void on_send_error() 
{
 // TODO здесь можно описать обработчик неудачной отправки
#ifdef MDEBUG    
    uart_puts("send failed...\n");
#endif
}


// Помещает пакет в очередь отправки. 
// buf - буфер с данными, size - длина данных (от 1 до 32)
uint8_t send_data(uint8_t * buf, uint8_t size) 
{
    radio_deassert_ce(); // Если в режиме приёма, то выключаем его 
    uint8_t conf = radio_readreg(CONFIG);
    if (!(conf & (1 << PWR_UP))) // Если питание по какой-то причине отключено, возвращаемся с ошибкой
    {

#ifdef MDEBUG    
        uart_puts("power is down..\n");
#endif
        return 0; 
    }
    uint8_t status = radio_writereg(CONFIG, conf & ~(1 << PRIM_RX)); // Сбрасываем бит PRIM_RX
    if (status & (1 << TX_FULL_STATUS))  // Если очередь передатчика заполнена, возвращаемся с ошибкой
    {

#ifdef MDEBUG    
        uart_puts("queue is full..\n");
#endif
        return 0;
    }

#ifdef MDEBUG    
    uart_puts("try to send..\n");
#endif
    radio_write_buf(W_TX_PAYLOAD, buf, size); // Запись данных на отправку
    radio_assert_ce(); // Импульс на линии CE приведёт к началу передачи
    _delay_us(15); // Нужно минимум 10мкс, возьмём с запасом
    radio_deassert_ce();
    return 1;
}

volatile unsigned char BLOCKM_STAT = 0;
volatile uint8_t radio_delay = 0;

// Вызывается при получении нового пакета по каналу 1 от удалённой стороны.
// buf - буфер с данными, size - длина данных (от 1 до 32)
void on_packet(uint8_t * buf, uint8_t size) 
{
    // TODO здесь нужно написать обработчик принятого пакета
    RELAY_PORT |= (1<<BLINK_PIN);
    radio_delay = 0;

#ifdef MDEBUG    
    char buff[100];
    char *mm = buff;
    for(int i =0 ; i < size; i++)
    {
        sprintf(mm, "%04X ", buf[i]);
        mm += 4;
    }
    buff[size*4] = '\0';
    uart_puts(buff);
    uart_puts("\r\n");
#endif

    

    if (buf[1] & (1<<0))
    {
        BLOCKM_STAT |= (1<<0);
        BLOCKM_STAT &=~ (1<<1);
        BLOCKM_STAT &=~ (1<<2);
        BLOCKM_STAT &=~ (1<<3);

        if ( buf[1] & (1<<3) )
        {
            RELAY_PORT &=~ (1<<RELAY_L_PIN); // ON
            RELAY_PORT &=~ (1<<RELAY_R_PIN); // ON
            BLOCKM_STAT |= (1<<3);
        }
        else if( buf[1] & (1<<1) )
        {
            RELAY_PORT &=~ (1<<RELAY_L_PIN); // ON
            RELAY_PORT |= (1<<RELAY_R_PIN); // OFF
            BLOCKM_STAT |= (1<<1);
        }
        else if ( buf[1] & (1<<2) )
        {
            RELAY_PORT &=~ (1<<RELAY_R_PIN); // ON 
            RELAY_PORT |= (1<<RELAY_L_PIN); // OFF
            BLOCKM_STAT |= (1<<2);
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
        BLOCKM_STAT &=~ (1<<0);
        BLOCKM_STAT &=~ (1<<1);
        BLOCKM_STAT &=~ (1<<2);
    }
    // Если предполагается немедленная отправка ответа, то необходимо 
    // обеспечить задержку ,
    // во время которой чип отправит подтверждение о приёме 
    // чтобы с момента приёма пакета до перевода в режим PTX прошло:
    // 130мкс + ((длина_адреса + длина_CRC + длина_данных_подтверждения) * 8 + 17) / скорость_обмена
    // При типичных условиях и частоте МК 8 мГц достаточно 
    // дополнительной задержки 100мкс
    _delay_us(100);    
    char payload_send[32] = {
                                BLOCKM_STAT,0x00,0x00,0x00
                                ,0x00,0x00,0x00,0x00
                                ,0x00,0x00,0x00,0x00
                                ,0x00,0x00,0x00,0x00
                                ,0x00,0x00,0x00,0x00
                                ,0x00,0x00,0x00,0x00
                                ,0x00,0x00,0x00,0x00
                                ,0x00,0x00,0x00,0x00
                            };
    send_data(payload_send, 32);
}

void check_radio() 
{
    RELAY_PORT &=~ (1<<BLINK_PIN);

    if (!radio_is_interrupt()) // Если прерывания нет, то не задерживаемся
        return;

    uint8_t status = radio_cmd(NOP);
    radio_writereg(STATUS, status); // Просто запишем регистр обратно, тем самым сбросив биты прерываний
  
    if (status & ((1 << TX_DS) | (1 << MAX_RT))) 
    { // Завершена передача успехом, или нет,
        if (status & (1 << MAX_RT)) 
        { // Если достигнуто максимальное число попыток
            radio_cmd(FLUSH_TX); // Удалим последний пакет из очереди
            on_send_error(); // Вызовем обработчик
        } 
        
        if (!(radio_readreg(FIFO_STATUS) & (1 << TX_EMPTY))) 
        { // Если в очереди передатчика есть что передавать
            radio_assert_ce(); // Импульс на линии CE приведёт к началу передачи
            _delay_us(15); // Нужно минимум 10мкс, возьмём с запасом
            radio_deassert_ce();
        } 
        else 
        {
            uint8_t conf = radio_readreg(CONFIG);
            radio_writereg(CONFIG, conf | (1 << PRIM_RX)); // Устанавливаем бит PRIM_RX: приём
            radio_assert_ce(); // Высокий уровень на линии CE переводит радио-чип в режим приёма
        }
    }
    
    uint8_t protect = 4; // В очереди FIFO не должно быть более 3 пакетов. Если больше, значит что-то не так
    while (((status & (7 << RX_P_NO)) != (7 << RX_P_NO)) && protect--) 
    { // Пока в очереди есть принятый пакет
        uint8_t l = radio_read_rx_payload_width(); // Узнаём длину пакета

#ifdef MDEBUG    
        char buff[100];
        sprintf(buff, "%d\n", l );  
        uart_puts(buff);
#endif        
        if (l > 32) 
        { // Ошибка. Такой пакет нужно сбросить
            radio_cmd(FLUSH_RX); 
        } 
        else 
        { 
            uint8_t buf[32]; // буфер для принятого пакета
            radio_read_buf(R_RX_PAYLOAD, &buf[0], l); // начитывается пакет
            if ((status & (7 << RX_P_NO)) == (0 << RX_P_NO)) 
            { // если datapipe 1 
                on_packet(&buf[0], l); // вызываем обработчик полученного пакета
            }
        }         
        status = radio_cmd(NOP);
    }
}


ISR(TIMER1_COMPA_vect)
{
    radio_delay++;
}


int main(void) 
{
    DDRA |= 1<<BLINK_PIN;
    DDRA |= (1<<RELAY_L_PIN)|(1<<RELAY_R_PIN);
    RELAY_PORT &=~ 1<<BLINK_PIN;
    RELAY_PORT &=~ (1<<RELAY_L_PIN);
    RELAY_PORT &=~ (1<<RELAY_R_PIN);

    TCCR1B |= (1 << WGM12); // configure timer1 for CTC mode
    TIMSK |= (1 << OCIE1A); // enable the CTC interrupt

    // timer0 code
    //TIMSK |= 1<<TOIE0;
    // OCR0 = 0xFF;
    // TCNT0 = 0x00; // 7812 // two times in seconds // 30 times of 256
    // TCCR0 |= (1<<CS02) | (1<<CS00);
    // timer0 end code

    sei();
    OCR1A = 15625; // 16 000 000 / 1024 //This flag is set in the timer clock cycle after the counter 
    TCCR1B |= ((1 << CS12) | (1 << CS10)); // start the timer at 16MHz/1024

    
#ifdef MDEBUG    
    uart_init(UART_BAUDRATE);
#endif

    radio_init();
    while (!radio_start()) 
    { 
        _delay_ms(1000);
    }
    // Перед включением питания чипа и сигналом CE 
    // должно пройти время достаточное для начала работы осциллятора
    // Для типичных резонаторов с эквивалентной индуктивностью 
    //не более 30мГн достаточно 1.5 мс
    _delay_ms(2); 

    radio_assert_ce();

    for(;;) 
    {
        check_radio();
  
        if(radio_delay >= 70)
        {
            radio_delay = 0;
            RELAY_PORT &=~ (1<<RELAY_L_PIN);
            RELAY_PORT &=~ (1<<RELAY_R_PIN);
        }
    }
}
