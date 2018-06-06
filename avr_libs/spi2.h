#define USART_DDR DDRD

#define USART_XCK 4

// Инициализация интерфейса USART0 в режиме SPI-master
void spi_init() {
  UBRR0 = 0; 
  USART_DDR |= (1 << USART_XCK);
  UCSR0C = (1 << UMSEL01) | (1 << UMSEL00); // выбор режима SPI-master
  UCSR0B = (1 << RXEN0) | (1 << TXEN0); // Включение приёмника и передатчика
  UBRR0 = 1;  // Выбор частоты интерфейса 1/4 от частоты ЦП
}

// Передаёт и принимает 1 байт по SPI, возвращает полученное значение
uint8_t spi_send_recv(uint8_t data) {
  UDR0 = data;
  while (!(UCSR0A & (1 << RXC0)));
  return UDR0; 
}