// --- ФУНКЦИЯ ОБРАБОТКИ ОШИБОК И ПЕРЕЗАПУСКА ---
// Управляет повторными попытками перезапуска трекера после возникновения ошибки.
void handleErrorRecovery() {
    unsigned long now = millis(); // Получаем текущее время
    if (error_retry_count==0){start_time_error=timerab;}
    if ((timerab - start_time_error) > 30) {
    wdt_enable(WDTO_1S); // Включаем таймер на 1 секунду
    while(1); // Зависаем в этом цикле, ожидая сброса
     }

    // Если мы уже находимся в состоянии окончательной ошибки (Sost 56), ничего не делаем
    if (Sost == 56) {
        return;
    }

    // Если счетчик попыток еще не достиг максимума
    if (error_retry_count < MAX_ERROR_RETRIES) {
        // Если это первая попытка или прошло достаточно времени с последней попытки
        // now - last_error_retry_time >= ERROR_RETRY_INTERVAL_MS
        // При первой ошибке last_error_retry_time = 0, поэтому условие будет now >= ERROR_RETRY_INTERVAL_MS
        // Это означает, что первая попытка произойдет только после истечения ERROR_RETRY_INTERVAL_MS
        if ( (now - last_error_retry_time >= ERROR_RETRY_INTERVAL_MS)) {
            
           if (error_retry_count>=1){
            stop_error = false; // Сбрасываем флаг общей ошибки, чтобы разрешить перезапуск
            // Переводим трекер в начальное состояние или в состояние начала коррекции
            Sost = 0; // Или Sost = 21 для начала обычной коррекции
           }
            error_retry_count++; // Увеличиваем счетчик попыток
            last_error_retry_time = now; // Запоминаем время текущей попытки
            // Опционально: можно добавить логирование попытки
            // Serial.print("Error retry attempt: ");
            // Serial.println(error_retry_count);
        }
    } else {
        // Если все попытки исчерпаны, переходим в окончательное состояние ошибки
        Sost = 56; // Окончательная ошибка, требуется ручное вмешательство
        stop_error = true; // Устанавливаем флаг ошибки, чтобы оставаться в этом состоянии
    }
}
// --- ФУНКЦИЯ ПРОВЕРКИ ТАЙМАУТОВ КОРРЕКЦИИ ---
// Эта функция проверяет, не превысило ли время движения по осям LR или UD
// максимально допустимое время коррекции.
void checkCorrectionTimeouts() {
    unsigned long now = millis(); // Получаем текущее время

    // Проверка таймаута для горизонтальной коррекции (LR)
    // start_time_LR_correction должен быть установлен в 0, когда нет активного движения LR,
    // и установлен в currentMillis, когда начинается движение LR.
    if ((dvig_left || dvig_right) && start_time_LR_correction == 0) {
        start_time_LR_correction = now;
    } else if (!(dvig_left || dvig_right)) { // Если двигатели LR не активны
        start_time_LR_correction = 0; // Сбрасываем таймер
    }

    if (start_time_LR_correction != 0 && (now - start_time_LR_correction > T_max_LR_CORRECTION_MS)) {
        stop_error = true; // Устанавливаем флаг общей ошибки
        Sost = 57;         // Состояние ошибки: таймаут движения (UD или LR)
        // Дополнительно можно выключить двигатели здесь, если они еще работают
        dvig_left = false;
        dvig_right = false;
        start_time_LR_correction = 0; // Сбрасываем таймер, чтобы не срабатывал повторно
    }

    // Проверка таймаута для вертикальной коррекции (UD)
    // start_time_UD_correction должен быть установлен в 0, когда нет активного движения UD,
    // и установлен в currentMillis, когда начинается движение UD.
    if ((dvig_up || dvig_down) && start_time_UD_correction == 0) {
        start_time_UD_correction = now;
    } else if (!(dvig_up || dvig_down)) { // Если двигатели UD не активны
        start_time_UD_correction = 0; // Сбрасываем таймер
    }

    if (start_time_UD_correction != 0 && (now - start_time_UD_correction > T_max_UD_CORRECTION_MS)) {
        stop_error = true; // Устанавливаем флаг общей ошибки
        Sost = 58;         // Состояние ошибки: таймаут движения (UD или LR)
        // Дополнительно можно выключить двигатели здесь, если они еще работают
        dvig_up = false;
        dvig_down = false;
        start_time_UD_correction = 0; // Сбрасываем таймер, чтобы не срабатывал повторно
    }
}
// --- Функция сохранения настроек в EEPROM ---
void saveSettings() {
  if (settingsChanged) {
    EEPROM.put(EEPROM_ADDR_UPDOWN, offsetUpDown);
    EEPROM.put(EEPROM_ADDR_LEFTRIGHT, offsetLeftRight);
    EEPROM.put(EEPROM_ADDR_MIN_LIGH, min_ligh);
    EEPROM.put(EEPROM_ADDR_ANEMOM_MAX, anemom_max);
    EEPROM.put(EEPROM_ADDR_I_MAX_LR, I_max_LR);
    EEPROM.put(EEPROM_ADDR_I_MAX_UD, I_max_UD);
    EEPROM.put(EEPROM_ADDR_T_MAX_UD, T_max_UD);
    EEPROM.put(EEPROM_ADDR_T_MAX_LR, T_max_LR);
    settingsChanged = false; // Сбрасываем флаг после сохранения
    // Serial.println("Settings saved to EEPROM.");
  }
}
