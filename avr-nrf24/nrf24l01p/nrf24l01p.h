#define RADIO_PORT PORTD
#define RADIO_DDR DDRD
#define RADIO_PIN PIND

#define RADIO_CSN 1
#define RADIO_CE 2
#define RADIO_IRQ 3

// Выбирает активное состояние (высокий уровень) на линии CE
inline void radio_assert_ce() {
  RADIO_PORT |= (1 << RADIO_CE); // Установка высокого уровня на линии CE
}

// Выбирает неактивное состояние (низкий уровень) на линии CE
inline void radio_deassert_ce() {
  RADIO_PORT &= ~(1 << RADIO_CE); // Установка низкого уровня на линии CE  
}

// Поскольку функции для работы с csn не предполагается использовать в иных файлах, их можно объявить static

// Выбирает активное состояние (низкий уровень) на линии CSN
inline static void csn_assert() {
  RADIO_PORT &= ~(1 << RADIO_CSN); // Установка низкого уровня на линии CSN
}

// Выбирает неактивное состояние (высокий уровень) на линии CSN
inline static void csn_deassert() {
  RADIO_PORT |= (1 << RADIO_CSN); // Установка высокого уровня на линии CSN
}

// Инициализирует порты 
void radio_init() {
  RADIO_DDR |= (1 << RADIO_CSN) | (1 << RADIO_CE); // Ножки CSN и CE на выход
  RADIO_DDR &= ~(1 < RADIO_IRQ); // IRQ - на вход
  csn_deassert();
  radio_deassert_ce();
  spi_init();
}

// Выполняет команду cmd, и читает count байт ответа, помещая их в буфер buf, возвращает регистр статуса
uint8_t radio_read_buf(uint8_t cmd, uint8_t * buf, uint8_t count) {
  csn_assert();
  uint8_t status = spi_send_recv(cmd);
  while (count--) {
    *(buf++) = spi_send_recv(0xFF);
  }
  csn_deassert();
  return status;
}

// Выполняет команду cmd, и передаёт count байт параметров из буфера buf, возвращает регистр статуса
uint8_t radio_write_buf(uint8_t cmd, uint8_t * buf, uint8_t count) {
  csn_assert();
  uint8_t status = spi_send_recv(cmd);
  while (count--) {
    spi_send_recv(*(buf++));
  }
  csn_deassert();
  return status;
}

// Читает значение однобайтового регистра reg (от 0 до 31) и возвращает его
uint8_t radio_readreg(uint8_t reg) {
  csn_assert();
  spi_send_recv((reg & 31) | R_REGISTER);
  uint8_t answ = spi_send_recv(0xFF);
  csn_deassert();
  return answ;
}

// Записывает значение однобайтового регистра reg (от 0 до 31), возвращает регистр статуса
uint8_t radio_writereg(uint8_t reg, uint8_t val) {
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

// Записывает count байт из буфера buf в многобайтовый регистр reg (от 0 до 31), возвращает регистр статуса
uint8_t radio_writereg_buf(uint8_t reg, uint8_t * buf, uint8_t count) {
  return radio_write_buf((reg & 31) | W_REGISTER, buf, count);
}

// Возвращает размер данных в начале FIFO очереди приёмника
uint8_t radio_read_rx_payload_width() {
  csn_assert();
  spi_send_recv(R_RX_PL_WID);
  uint8_t answ = spi_send_recv(0xFF);
  csn_deassert();
  return answ;
}

// Выполняет команду. Возвращает регистр статуса
uint8_t radio_cmd(uint8_t cmd) {
  csn_assert();
  uint8_t status = spi_send_recv(cmd);
  csn_deassert();
  return status;
}

// Возвращает 1, если на линии IRQ активный (низкий) уровень.
uint8_t radio_is_interrupt() {
  return (RADIO_PIN & RADIO_IRQ) ? 0 : 1;
}

/*
uint8_t radio_is_interrupt() {
// использовать этот вариант только в крайних случаях!!!
  return (radio_cmd(NOP) & ((1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT))) ? 1 : 0;
}
*/

// Функция производит первоначальную настройку устройства. Возвращает 1, в случае успеха, 0 в случае ошибки
uint8_t radio_start() {
  uint8_t self_addr[] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7}; // Собственный адрес
  uint8_t remote_addr[] = {0xC2, 0xC2, 0xC2, 0xC2, 0xC2}; // Адрес удалённой стороны
  uint8_t chan = 3; // Номер радио-канала (в диапазоне 0 - 125)

  radio_deassert_ce();
  for(uint8_t cnt = 100;;) {
    radio_writereg(CONFIG, (1 << EN_CRC) | (1 << CRCO) | (1 << PRIM_RX)); // Выключение питания
    if (radio_readreg(CONFIG) == ((1 << EN_CRC) | (1 << CRCO) | (1 << PRIM_RX))) 
      break;
    // Если прочитано не то что записано, то значит либо радио-чип ещё инициализируется, либо не работает.
    if (!cnt--)
      return 0; // Если после 100 попыток не удалось записать что нужно, то выходим с ошибкой
    _delay_ms(1);
  }

  radio_writereg(EN_AA, (1 << ENAA_P1)); // включение автоподтверждения только по каналу 1
  radio_writereg(EN_RXADDR, (1 << ERX_P0) | (1 << ERX_P1)); // включение каналов 0 и 1
  radio_writereg(SETUP_AW, SETUP_AW_5BYTES_ADDRESS); // выбор длины адреса 5 байт
  radio_writereg(SETUP_RETR, SETUP_RETR_DELAY_250MKS | SETUP_RETR_UP_TO_2_RETRANSMIT); 
  radio_writereg(RF_CH, chan); // Выбор частотного канала
  radio_writereg(RF_SETUP, RF_SETUP_1MBPS | RF_SETUP_0DBM); // выбор скорости 1 Мбит/с и мощности 0dBm
  
  radio_writereg_buf(RX_ADDR_P0, &remote_addr[0], 5); // Подтверждения приходят на канал 0 
  radio_writereg_buf(TX_ADDR, &remote_addr[0], 5);

  radio_writereg_buf(RX_ADDR_P1, &self_addr[0], 5);
  
  radio_writereg(RX_PW_P0, 0);
  radio_writereg(RX_PW_P1, 32); 
  radio_writereg(DYNPD, (1 << DPL_P0) | (1 << DPL_P1)); // включение произвольной длины для каналов 0 и 1
  radio_writereg(FEATURE, 0x04); // разрешение произвольной длины пакета данных

  radio_writereg(CONFIG, (1 << EN_CRC) | (1 << CRCO) | (1 << PWR_UP) | (1 << PRIM_RX)); // Включение питания
  return (radio_readreg(CONFIG) == ((1 << EN_CRC) | (1 << CRCO) | (1 << PWR_UP) | (1 << PRIM_RX))) ? 1 : 0;
}