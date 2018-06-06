// Помещает пакет в очередь отправки. 
// buf - буфер с данными, size - длина данных (от 1 до 32)
uint8_t send_data(uint8_t * buf, uint8_t size) {
  radio_deassert_ce(); // Если в режиме приёма, то выключаем его 
  uint8_t conf = radio_readreg(CONFIG);
  // Сбрасываем бит PRIM_RX, и включаем питание установкой PWR_UP
  uint8_t status = radio_writereg(CONFIG, (conf & ~(1 << PRIM_RX)) | (1 << PWR_UP)); 
  if (status & (1 << TX_FULL_STATUS))  // Если очередь передатчика заполнена, возвращаемся с ошибкой
    return 0;
  if (!(conf & (1 << PWR_UP))) // Если питание не было включено, то ждём, пока запустится осциллятор
    _delay_ms(2); 
  radio_write_buf(W_TX_PAYLOAD, buf, size); // Запись данных на отправку
  radio_assert_ce(); // Импульс на линии CE приведёт к началу передачи
  _delay_us(15); // Нужно минимум 10мкс, возьмём с запасом
  radio_deassert_ce();
  return 1;
}

void check_radio() {
  if (!radio_is_interrupt()) // Если прерывания нет, то не задерживаемся
    return;
  uint8_t status = radio_cmd(NOP);
  radio_writereg(STATUS, status); // Просто запишем регистр обратно, тем самым сбросив биты прерываний
  
  if (status & ((1 << TX_DS) | (1 << MAX_RT))) { // Завершена передача успехом, или нет,
    if (status & (1 << MAX_RT)) { // Если достигнуто максимальное число попыток
      radio_cmd(FLUSH_TX); // Удалим последний пакет из очереди
      on_send_error(); // Вызовем обработчик
    } 
    if (!(radio_readreg(FIFO_STATUS) & (1 << TX_EMPTY))) { // Если в очереди передатчика есть что передавать
      radio_assert_ce(); // Импульс на линии CE приведёт к началу передачи
      _delay_us(15); // Нужно минимум 10мкс, возьмём с запасом
      radio_deassert_ce();
    } else {
      uint8_t conf = radio_readreg(CONFIG);
      radio_writereg(CONFIG, conf & ~(1 << PWR_UP)); // Если пусто, отключаем питание
    }
  }
  uint8_t protect = 4; // В очереди FIFO не должно быть более 3 пакетов. Если больше, значит что-то не так
  while (((status & (7 << RX_P_NO)) != (7 << RX_P_NO)) && protect--) { // Пока в очереди есть принятый пакет
    radio_cmd(FLUSH_RX); // во всех случаях выкидываем пришедший пакет.
    status = radio_cmd(NOP);
  }
}