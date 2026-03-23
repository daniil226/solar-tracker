

void check_I() {
  unsigned long now = millis();

  // Защита по току должна работать только если двигатель включен
  if (dvig_right || dvig_left || dvig_up || dvig_down) {
    // Если это первый замер тока в начале движения, инициализируем фильтр
    if (!is_moving) {
      is_moving = true;
      I_filtered = I_dvig; // Инициализируем плавающее базовое значение
    }

    if (now - smart_time_I >= I_max_delay) {
      smart_time_I = now; // Обновляем таймер

      // Плавно обновляем базовое значение. Оно будет адаптироваться к нормальным, медленным изменениям.
      I_filtered = (I_dvig * ALPHA) + (I_filtered * (1.0 - ALPHA));

       // --- СПЕЦ-ЗАЩИТА ПРИ ВЕТРЕ ---
      if (isWindy && I_dvig >= 10.0) {
        stop_error = true;           // Останавливаем движение
        Sost = 59;                   // Состояние: Пауза по перегрузке (ветер)
        wind_overload_waiting = true; // ВКЛЮЧАЕМ режим ожидания (паузу)
        return;                      // Выходим, чтобы не сработали другие ошибки
      }
      // 1. Защита по абсолютному максимальному току
      if ((dvig_right || dvig_left) && (I_dvig >= I_max_LR)) {
        stop_error = true;
        Sost = 52;
        return;
      }
       if (!isWindy && (dvig_up || dvig_down) && (I_dvig >= I_max_UD)) {
        stop_error = true;
        Sost = 53;
        return;
      }

      // 2. Защита по относительному скачку тока
      // Теперь сравниваем текущий ток с плавающим базовым значением
      if (!isWindy && I_dvig > I_filtered * CURRENT_SPIKE_FACTOR) {
        y++;
      } else {
        y = 0;
      }

      if (y >= CONSECUTIVE_SPIKE_COUNT) {
        stop_error = true;
        if (dvig_right || dvig_left) {
          Sost = 54; // Ошибка: Скачок тока LR
        } else {
          Sost = 55; // Ошибка: Скачок тока UD
        }
        return;
      }
    
  }
  }
  else {
    // Если двигатель не движется, сбрасываем флаги и счетчик для нового движения
    is_moving = false;
    y = 0;
    // Можно также сбросить I_filtered, чтобы избежать скачка при следующем запуске
    // I_filtered = 0.0;
  }
}
