void sensorread() {

  /// расчет скорости ветра
  rotationsPerSecond=rotationCount; // Обновляем обороты в секунду
  rotationCount=0; // Сбрасываем счетчик для следующей секунды
    // Защита от ветра если скорость больше максимальной переходит в состояние 10 в котором ждет пока защита отлипнет через 30 минут
     if (rotationsPerSecond > anemom_max) {
        isWindy = true;
        veter_on_tick = 3600; // Обновляем счетчик каждый раз, когда ветер превышает порог
        Sost = 10;
        stop_error = false;
    }

  // 1. Считываем показания всех датчиков освещения
//  for (int i = 0; i < numSensors; i++) {
//    sensorValues[i] = analogRead(sensorPins[i]);
//    //if (sensorValues[i] <5){Esensor =1;} // Если показание очень низкое, возможно, ошибка датчика
//  }
for (int i = 0; i < numSensors; i++) {
        int currentValue = analogRead(sensorPins[i]);
        int medianValue = getMedianHistoryValue(i);
        
        // Проверка на аномальное показание
        if (abs(currentValue - medianValue) > MAX_DEVIATION) {
            // Если показание - выброс, добавляем в буфер медиану, а не новое значение

            count_err[i]++;
         if (count_err[i]<5){sensorHistory[i][historyIndex[i]] = medianValue;} else {sensorHistory[i][historyIndex[i]] = currentValue;}

            
        } else {
            // Если показание в пределах нормы, добавляем его в буфер
            sensorHistory[i][historyIndex[i]] = currentValue;
            count_err[i]=0;
        }
        
        historyIndex[i] = (historyIndex[i] + 1) % HISTORY_SIZE;
    }

sensorValues[0]=getMedianHistoryValue(0);
sensorValues[1]=getMedianHistoryValue(1);
sensorValues[2]=getMedianHistoryValue(2);
sensorValues[3]=getMedianHistoryValue(3);
  //if (sensorValues[0]>5&&sensorValues[1]>5&&sensorValues[2]>5&&sensorValues[3]>5){Esensor =0;} // Если все датчики показывают нормальное значение, сбрасываем ошибку
    // Защита от туч и низкой освещенности
updateCloudyStatus();

}

// --- Программа обработки прерывания (ISR) ---
// Эта функция вызывается автоматически при изменении состояния геркона (переходе в LOW)
void ISR_anemom() {
  // Простая подавление дребезга внутри ISR
  // Регистрируем новое срабатывание только если прошло достаточно времени с последнего
  if (millis() - lastDebounceTime > debounceDelay) {
    rotationCount++; // Увеличиваем счетчик
    lastDebounceTime = millis(); // Обновляем время последнего дребезга
  }
}
void autoCalibrate() {
   oled.clear();
   oled.setCursor(0, 0); oled.print(F("калибровка"));
  // 1. Считываем показания со всех датчиков
  for (int i = 0; i < numSensors; i++) {
    sensorValues[i] = analogRead(sensorPins[i]);
  }

  // 2. Рассчитываем среднее значение
  float averageValue = 0;
  for (int i = 0; i < numSensors; i++) {
    averageValue += sensorValues[i];
  }
  averageValue /= numSensors;

  // 3. Рассчитываем и применяем коэффициенты
  // К каждому датчику применяется свой коэффициент
  for (int i = 0; i < numSensors; i++) {
    // Избегаем деления на ноль, если показание равно 0
    if (sensorValues[i] != 0) {
      calibrationFactor[i] = averageValue / sensorValues[i];
      sensorValues[i] = sensorValues[i] * calibrationFactor[i];

    }
  }
    oled.setCursor(0, 1); oled.print(calibrationFactor[0]);
    oled.setCursor(0, 2); oled.print(calibrationFactor[1]);
    oled.setCursor(0, 3); oled.print(calibrationFactor[2]);
    oled.setCursor(0, 4); oled.print(calibrationFactor[3]);
    
     EEPROM.put(30, calibrationFactor[0]);
     EEPROM.put(35, calibrationFactor[1]);
     EEPROM.put(40, calibrationFactor[2]);
     EEPROM.put(45, calibrationFactor[3]);
     oled.setCursor(0, 5); oled.print(F("OK"));
     delay (2000);
  
}
void readCurrentSensor() {
  int raw_analog_value = 0;
  // Цикл для усреднения 20 измерений
  for (int i = 0; i < 20; i++) {
    raw_analog_value += analogRead(analogInPin);
  }
  // Расчет тока (I_dvig)
  I_dvig = (float)raw_analog_value * koef_shunt * 0.1;
}
int getMedianHistoryValue(int sensorIndex) {
    int tempArray[HISTORY_SIZE];
    for (int i = 0; i < HISTORY_SIZE; i++) {
        tempArray[i] = sensorHistory[sensorIndex][i];
    }
    // Сортировка массива для нахождения медианы
    for (int i = 0; i < HISTORY_SIZE - 1; i++) {
        for (int j = 0; j < HISTORY_SIZE - i - 1; j++) {
            if (tempArray[j] > tempArray[j + 1]) {
                int temp = tempArray[j];
                tempArray[j] = tempArray[j + 1];
                tempArray[j + 1] = temp;
            }
        }
    }
    return tempArray[HISTORY_SIZE / 2];
}
