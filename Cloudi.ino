void updateCloudyStatus() {
    // Вычисляем среднее значение со всех датчиков для более стабильного показания
    int averageSensorValue = (sensorValues[0] + sensorValues[1] + sensorValues[2] + sensorValues[3]) / 4;

    // Определяем "потенциальные" состояния на основе гистерезиса
    bool potentialCloudyNow = (averageSensorValue < (min_ligh + CLOUD_ENTER_OFFSET));
    
    // НОВАЯ ЛОГИКА: Потенциально ясно, если среднее значение выше порога ИЛИ хотя бы один датчик показывает высокий уровень
    bool potentialClearNow = (averageSensorValue > (min_ligh + CLOUD_EXIT_OFFSET)) ||
                             (sensorValues[0] > min_ligh ||
                              sensorValues[1] > min_ligh ||
                              sensorValues[2] > min_ligh ||
                              sensorValues[3] > min_ligh);

    // Логика обновления isCloudy с использованием start_cloudy как счетчика стабильности
    if (isCloudy == false) { // Если текущее стабильное состояние - "Ясно"
        if (potentialCloudyNow) {
            // Если сейчас потенциально облачно, начинаем/продолжаем отсчет секунд этого состояния
            start_cloudy++;
            // Если потенциально облачное состояние сохраняется достаточно долго, переходим в стабильное "Облачно"
            if (start_cloudy >= CLOUDY_DEBOUNCE_SECONDS) {
                isCloudy = true;
                // Сбрасываем start_cloudy, чтобы он начал отсчет длительности *стабильной* облачности (до 900 секунд)
                start_cloudy = 0; 
            }
        } else {
            // Если потенциально ясно (или мерцает), сбрасываем счетчик
            start_cloudy = 0;
        }
    } else { // Если текущее стабильное состояние - "Облачно"
        if (potentialClearNow) {
            // Если сейчас потенциально ясно, и мы в облачном состоянии, то сразу переходим в ясное.
            // Примечание: Эта логика не дебаунсит выход из облачности, только вход.
            // Если требуется дебаунсинг выхода, нужна отдельная переменная-таймер.
            isCloudy = false;
            start_cloudy = 0; // Сбрасываем счетчик
        } else {
            // Если не потенциально ясно, продолжаем отсчет длительности стабильной облачности
            start_cloudy++;
            // Ограничиваем счетчик на 900 (для regim = 3)
            if (start_cloudy >= 900) {
                start_cloudy = 900;
            }
        }
    }
}
