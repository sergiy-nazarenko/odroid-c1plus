#define SPI_DDR DDRB

#define SPI_SS 2
#define SPI_MOSI 3
#define SPI_SCK 5

// Инициализация интерфейса
void spi_init() {
  SPI_DDR |= (1 << SPI_MOSI) | (1 <<  SPI_SCK) | (1 << SPI_SS);
  SPCR = (1 << SPE) | (1 << MSTR); // режим 0, мастер, частота 1/4 от частоты ЦП
}

// Передаёт и принимает 1 байт по SPI, возвращает полученное значение
uint8_t spi_send_recv(uint8_t data) {
  SPDR = data;
  while (!(SPSR & (1 << SPIF)));
  return SPDR;
}