void logika(){ // Определение функции logika()
  Pol_UD = polog_UD*0.001; // Пересчет накопленного положения UD в условные единицы
  Pol_LR = polog_LR*0.001; // Пересчет накопленного положения LR в условные единицы
  
   
  if (parkovka==true) {backligh--; if (backligh<0){backligh=0;}} 
  
    time_tic_sek++; 
    time_tic_min = time_tic_sek / 60; // Реальные минуты
    timerab = time_tic_min;
    // ... остальной код
  timer_cor = 15-abs(timerab - time_last_corr);
  if (timer_cor<=0){timer_cor=0;}


  // 2. Обработка для направлений (Верх/Низ и Лево/Право)
  sumTop = sensorValues[0] + sensorValues[1]; // Сумма показаний верхних датчиков
  sumBottom = sensorValues[2] + sensorValues[3]; // Сумма показаний нижних датчиков
  diffUpDown = (sumTop - sumBottom) - offsetUpDown; // Разница верх/низ с учетом смещения
  sumLeft = sensorValues[0] + sensorValues[3]; // Сумма показаний левых датчиков
  sumRight = sensorValues[1] + sensorValues[2]; // Сумма показаний правых датчиков
  diffLeftRight = (sumLeft - sumRight) - offsetLeftRight; // Разница лево/право с учетом смещением

  normalizedUpDown = diffUpDown; // Нормализованная разница для UD
  normalizedLeftRight = diffLeftRight; // Нормализованная разница для LR


}
