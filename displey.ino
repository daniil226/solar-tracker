void  displey() {

  // Если режим парковки активен, очищаем дисплей и выходим
  if (parkovka == true && backligh==0) {
    oled.clear();
    oled.update();
    return;
  }

  // Если vflag изменился, рисуем статические элементы
  if (vflag != last_vflag) {
    oled.clear();
    drawStatic();
    last_vflag = vflag;
  }

  // Рисуем динамические элементы
  drawDynamic();

  // Отправляем все изменения на дисплей
  oled.update();
}

// --- Статическая часть ---
// Эта функция вызывается только один раз при смене экрана
void drawStatic() {
  if (vflag == 0) { // Главный экран
    oled.rect (16, 19, 112, 54, OLED_STROKE); //обводка
    oled.setCursor(0, 1); oled.print(F("Ток:"));
    oled.setCursor(64, 1); oled.print(F("Ветер:"));
    // Добавьте сюда любой другой неизменный текст или рамки
  }

  if (vflag == 1) { // Экран настроек 1
    oled.setCursor(7, 0); oled.print(F("Ручное движ поворот")); 
    oled.setCursor(7, 1); oled.print(F("Ручное движ наклон ")); 
    oled.setCursor(7, 2); oled.print(F("Порог облачности   "));
    oled.setCursor(7, 3); oled.print(F("Максимальный ветер "));
    oled.setCursor(7, 4); oled.print(F("Макс ток поворот   "));  
    oled.setCursor(7, 5); oled.print(F("Макс ток наклон    "));
  }

  // Если vflag == 2, статика рисуется внутри динамического switch
  if (vflag == 2) {
    oled.setCursor(0, 0); 
    switch (fs) {
      case 1: oled.print(F("Ручное движ поворот"));    oled.rect (16, 19, 112, 54, OLED_STROKE); //обводка break;
      case 2: oled.print(F("Ручное движ наклон "));    oled.rect (16, 19, 112, 54, OLED_STROKE); //обводка break;
      case 3: oled.print(F("Порог облачности   ")); break;
      case 4: oled.print(F("Максимальный ветер ")); break;
      case 5: oled.print(F("Макс ток поворот   ")); break;
      case 6: oled.print(F("Макс ток наклон    ")); break;
    }
    // Добавьте статические надписи для vflag=2 сюда
    if (fs == 1 || fs == 2) {
       oled.setCursor(0, 1); oled.print(F("Ток:"));
      //oled.setCursor(0, 3); oled.print(F("I:")); // Выводим метку I для обоих случаев
    }
  }
}

// --- Динамическая часть ---
// Эта функция вызывается в каждом цикле loop()
void drawDynamic() {
    int pointX = map(normalizedLeftRight, -300, 300, 16, 112);
    int pointY = map(normalizedUpDown, -300, 300, 19, 54);
    pointX = constrain(pointX, 16, 112);
    pointY = constrain(pointY, 19, 54);
  // Главный экран
  if (vflag == 0) {
    // Рисуем динамическую точку

        oled.clear(17, 20, 111, 53);
        oled.circle(pointX, pointY, 2, OLED_FILL);
        oled.rect(16, 19, 112, 54, OLED_STROKE);
    
    // Обновляем показания
    oled.setCursor(26, 1); oled.print(F("     " ));
    oled.setCursor(26, 1); oled.print(I_dvig, 2);
    oled.setCursor(100, 1); oled.print(F("  " ));
    oled.setCursor(100, 1); oled.print(rotationsPerSecond);

    // Обновляем состояние, 

     // oled.setCursor(0, 0); oled.print(F("                    "));
      oled.setCursor(0, 0);
      switch (Sost) {
        case 0:  oled.print(F("НАЧАЛЬНОЕ           ")); break;// // Начальное состояние или сброс после ошибки
        case 10: oled.print(F("ВЕТЕР ЛОЖИМСЯ       ")); break;// // Защита от ветра : Наклон на ноль
        case 11: oled.print(F("ВЕТЕР ОЖИДАЕМ       ")); break;// // Ожидание после защиты от ветра
        case 15: oled.print(F("ОБЛАЧНО ЗАМЕРЛИ     ")); break;// // Облачно: Замереть
        case 16: oled.print(F("ОБЛАЧНО ЛОЖИМСЯ     ")); break;// // Низкая освещенность (15 минут): Наклон в плоскость
        case 17: oled.print(F("ОБЛАЧНО ОЖИДАЕМ     ")); break;// // Низкая освещенность: Ожидание в плоском положении
        case 21: oled.print(F("ЯСНО ПОВОРОТ        ")); break;// // Нормальный режим: Горизонтальная коррекция
        case 22: oled.print(F("ЯСНО НАКЛОН         ")); break;// // Нормальный режим: Вертикальная коррекция
        case 23: oled.print(F("ЯСНО ОЖИДАЕМ        ")); break;// // Нормальный режим: Скорректировано, ожидание
        case 31: oled.print(F("ПАРКОВКА НАКЛОН     ")); break;// // Парковка: Наклон на ноль (ось UD)
        case 32: oled.print(F("ПАРКОВКА ПОВОРОТ    ")); break;// // Парковка: Поворот на ноль (ось LR)
        case 33: oled.print(F("ПАРКОВКА ЖДЕМ       ")); break;// // Парковка: Стоим в нулевом положении
        case 50: oled.print(F("ОШИБКА 50           ")); break;// Ошибка: Общая
        case 51: oled.print(F("ОШИБКА 51           ")); break;// Ошибка: Общая
        case 52: oled.print(F("СКАЧЕК ТОК ПОВОРОТА ")); break;// Ошибка: Перегрузка по току LR
        case 53: oled.print(F("СКАЧЕК ТОК НАКЛОНА  ")); break;// Ошибка: Перегрузка по току UD
        case 54: oled.print(F("РОСТ ТОК ПОВОРОТА   ")); break;// Ошибка: Перегрузка по току LR тренд
        case 55: oled.print(F("РОСТ ТОК НАКЛОНА    ")); break;// Ошибка: Перегрузка по току LR тренд
        case 57: oled.print(F("ТАЙМАУТ ПОВОРОТА    ")); break;//// Состояние ошибки: таймаут движения ( LR)           
        case 58: oled.print(F("ТАЙМАУТ НАКЛОНА     ")); break;//// Состояние ошибки: таймаут движения (UD)           
        case 56: oled.print(F("ОШИБКА РЕСТАРТА     ")); break;/// Окончательная ошибка, все попытки исчерпаны
      }

    
    // Обновляем данные датчиков (последняя строка)
    oled.setCursor(0, 7);oled.print(F("                    "));oled.setCursor(0, 7);
    for (int i = 0; i < numSensors; i++){
      oled.print(sensorValues[i]);
      if (i < numSensors - 1) {
        oled.print(F(" "));
      }
     }
     oled.setCursor(102, 7); 
     switch (Sost) {
      case 23: oled.print(timer_cor); break;
      case 11: oled.print(veter_on_tick); break;
      case 15: oled.print(start_cloudy); break;
      case 50:
      case 51:
      case 52:
      case 53: 
      case 54:
      case 55:
      case 56:
      case 57:
      case 58:
      case 59:
      case 60:oled.print(start_time_error-timerab);  break;
     }
  }

  // Экран настроек 1
  if (vflag == 1) {
      // Обновляем "курсор" (стрелку >)
      for (int i = 1; i <= 6; i++) {
          oled.setCursor(0, i - 1);
          oled.print((fs == i) ? F(">") : F(" "));
      }
  }

  // Экран настроек 2
  if (vflag == 2) {
    // Обновляем значения в зависимости от fs
    switch (fs) {
      case 1: 
        oled.setCursor(0, 7); oled.print((normalizedLeftRight > 0) ? F("свет слева ") : (normalizedLeftRight < 0) ? F("свет справа"): F(" "));
        oled.print(abs(normalizedLeftRight));
        oled.clear(17, 20, 111, 53);
        oled.circle(pointX, pointY, 2, OLED_FILL);
        oled.rect(16, 19, 112, 54, OLED_STROKE);
        oled.setCursor(26, 1); oled.print(I_dvig, 2);

        break;
      case 2: 
        oled.setCursor(0, 7); oled.print((normalizedUpDown > 0) ? F("свет сверху") : (normalizedUpDown < 0) ? F("свет снизу ") : F(" "));
        oled.print(abs(normalizedUpDown));
        oled.setCursor(30, 1); oled.print(I_dvig, 2);
        oled.clear(17, 20, 111, 53);
        oled.circle(pointX, pointY, 2, OLED_FILL);
        oled.rect(16, 19, 112, 54, OLED_STROKE);
        break;
      case 3: oled.setCursor(0, 1); oled.print(min_ligh);   oled.print(F(" едениц")); break;
      case 4: oled.setCursor(0, 2); oled.print(anemom_max); oled.print(F(" об/сек")); break;
      case 5: oled.setCursor(0, 3); oled.print(I_max_LR, 2);oled.print(F("Ампер")); break;
      case 6: oled.setCursor(0, 4); oled.print(I_max_UD, 2);oled.print(F("Ампер")); break;
    }
  }
}
