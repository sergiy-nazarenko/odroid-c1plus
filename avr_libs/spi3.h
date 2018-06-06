#define SWSPI_DDR DDRB
#define SWSPI_PORT PORTB
#define SWSPI_PIN PINB

#define SWSPI_MOSI 6
#define SWSPI_MISO 7
#define SWSPI_SCK 8

// Инициализация программного интерфейса SPI
void spi_init() {
  SWSPI_PORT &= ~((1 << SWSPI_MOSI) | (1 << SWSPI_SCK));
  SWSPI_DDR |= (1 << SWSPI_MOSI) | (1 << SWSPI_SCK);
  SWSPI_DDR &= ~ (1 << SWSPI_MISO);
  SWSPI_PORT |= (1 << SWSPI_MISO); // подтяжка на линии MISO
}

// Передаёт и принимает 1 байт по SPI, возвращает полученное значение
uint8_t spi_send_recv(uint8_t data) {
  for (uint8_t i = 8; i > 0; i--) {
    if (data & 0x80) 
      SWSPI_PORT |= (1 << SWSPI_MOSI); // передача единички
    else
      SWSPI_PORT &= ~(1 << SWSPI_MOSI); // передача нулика
    SWSPI_PORT |= (1 << SWSPI_SCK);
    data <<= 1;
    if (SWSPI_PIN & (1 << SWSPI_MISO)) // Чтение бита на линии MISO
      data |= 1; 
    SWSPI_PORT &= ~(1 << SWSPI_SCK);
  }
  return data;
}