void dvig() {
    // Получаем текущее время один раз для использования в функции
    const unsigned long currentMillis = millis();

    if (isWindy) {
        // Если мы НЕ в режиме ожидания после перегрузки И количество попыток < 20
        if (wind_overload_waiting == false && veter_int_tick < 20) {
            regim = 4; // Работаем в штатном режиме защиты от ветра (едем в 0)
        } else {
            // Если была перегрузка (waiting) ИЛИ превышен лимит попыток
            regim = 7;             // Переходим в режим ошибки (двигатели стоп)
            veter_overload_tick++; // Начинаем отсчет времени паузы
        }

        // Логика выхода из паузы: если отсчитали 10 тиков
        if (veter_overload_tick >= 10) {
            wind_overload_waiting = false; // Разрешаем движение снова
            veter_overload_tick = 0;       // Сбрасываем таймер паузы
            veter_int_tick++;              // Увеличиваем общий счетчик инцидентов
        }
    } else {
        if (stop_error == true) {
            regim = 7; // Режим ошибки имеет наивысший приоритет
        } else {
            // --- Логика парковки по датчикам освещенности ---
            if (parkovka == false) { 
                if (sensorValues[0] < PARK_ENTER_THRESHOLD &&
                    sensorValues[1] < PARK_ENTER_THRESHOLD &&
                    sensorValues[2] < PARK_ENTER_THRESHOLD &&
                    sensorValues[3] < PARK_ENTER_THRESHOLD) 
                {
                    if (parkovkaTimer == 0) parkovkaTimer = currentMillis;
                    if (currentMillis - parkovkaTimer >= PARK_DELAY_MS) {
                        parkovka = true;
                        parkovkaTimer = 0;
                    }
                } else {
                    parkovkaTimer = 0;
                }
            } else { 
                if (sensorValues[0] > PARK_EXIT_THRESHOLD ||
                    sensorValues[1] > PARK_EXIT_THRESHOLD ||
                    sensorValues[2] > PARK_EXIT_THRESHOLD ||
                    sensorValues[3] > PARK_EXIT_THRESHOLD) 
                {
                    if (parkovkaTimer == 0) parkovkaTimer = currentMillis;
                    if (currentMillis - parkovkaTimer >= PARK_DELAY_MS) {
                        parkovka = false;
                        parkovkaTimer = 0;
                    }
                } else {
                    parkovkaTimer = 0;
                }
            }

            // --- Определение основного режима работы (regim) ---
            if (hand_dvig == true) {
                regim = 6; 
            } else {
                if (parkovka == true) {
                    regim = 0; 
                } else {
                    if (isCloudy) {
                        regim = (start_cloudy >= 900) ? 3 : 2;
                    } else {
                        regim = 1; 
                    }
                }
            }
        } 
    } // Конец блока isWindy/else

    // --- Изначально выключаем все двигатели ---
    if (regim != 6) { 
        dvig_up = false;
        dvig_down = false;
        dvig_left = false;
        dvig_right = false;
    }

    // --- Выполнение действий на основе regim ---
    switch (regim) {
        case 0: if(Sost != 31 && Sost != 32 && Sost != 33) Sost = 31; break;
        case 1: if(Sost != 21 && Sost != 22 && Sost != 23) Sost = 23; break;
        case 2: if(Sost != 15) Sost = 15; break;
        case 3: if(Sost != 16 && Sost != 17) Sost = 16; break;
        case 4: if(Sost != 10 && Sost != 11) Sost = 10; break;
        case 6: break; // Ручной
        case 7: if(Sost < 50) Sost = 51; break;
    }

    // --- Машина состояний (Sost) ---
    switch (Sost) {
        case 10: 
            dvig_down = true;
            if (timer_dvig_down && !real_dvig_down) Sost = 11;
            break;

        case 11:
            veter_int_tick = 0;
            veter_overload_tick = 0;
            wind_overload_waiting = false;
            if (veter_on_tick <= 1) {
                isWindy = false;
                Sost = 0;
            }
            break;

        case 15: break; 

        case 16:
            dvig_down = true;
            if (timer_dvig_down && !real_dvig_down) Sost = 17;
            break;

        case 17: break;

        case 21: // Горизонтальная коррекция
            if (abs(normalizedLeftRight) > 20) {
                if (normalizedLeftRight < 0) dvig_left = true; else dvig_right = true;
                if ((dvig_left && timer_dvig_left && !real_dvig_left) ||
                    (dvig_right && timer_dvig_right && !real_dvig_right)) {
                    Sost = 22;
                }
            } else {
                Sost = 22;
            }
            break;

        case 22: // Вертикальная коррекция
            if (abs(normalizedUpDown) > 20) {
                if (normalizedUpDown > 0) dvig_down = true; else dvig_up = true;
                if ((dvig_up && timer_dvig_up && !real_dvig_up) ||
                    (dvig_down && timer_dvig_down && !real_dvig_down)) {
                    correction_pass_count++;
                    Sost = (correction_pass_count < 2) ? 21 : 23;
                    if (Sost == 23) { correction_pass_count = 0; time_last_corr = timerab; }
                }
            } else {
                correction_pass_count++;
                Sost = (correction_pass_count < 2) ? 21 : 23;
                if (Sost == 23) { correction_pass_count = 0; time_last_corr = timerab; }
            }
            break;

        case 23:
            if (timerab - time_last_corr > 14 || time_tic_sek < 30) {
                Sost = 21;
                correction_pass_count = 0;
                time_last_corr = timerab;
            }
            break;

        case 31:
            dvig_down = true;
            if (timer_dvig_down && !real_dvig_down) Sost = 32;
            break;

        case 32:
            dvig_left = true;
            if (timer_dvig_left && !real_dvig_left) Sost = 33;
            break;

        case 33: break; // Парковка: Стоим в нулевом положении
        case 50: break;// Ошибка: Общая
        case 51: break;// Ошибка: Общая
        case 52: break;// Ошибка: Перегрузка по току LR
        case 53: break;// Ошибка: Перегрузка по току UD
        case 54: break;// Ошибка: Перегрузка по току LR тренд
        case 55:  break;// Ошибка: Перегрузка по току LR тренд
        case 57:  break;//// Состояние ошибки: таймаут движения ( LR)           
        case 58:  break;//// Состояние ошибки: таймаут движения (UD)           
        case 56: break; // Окончательная ошибка, все попытки исчерпаны
        case 59: break; // Окончательная ошибка, все попытки исчерпаны
        // Трекер остается в этом состоянии до ручного вмешательства.
        // Двигатели должны быть выключены.
       

        default: break; // Ошибки 50-58
    }
}
