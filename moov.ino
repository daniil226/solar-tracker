void moov() {
  unsigned long now = millis(); // Используем локальную переменную для текущего времени

  // Расчет тока потребления двигателя (всегда в начале функции)
  


  // Обновляем предыдущие состояния флагов движения в начале функции для точного определения изменения состояния
  bool prev_dvig_left  = dvig_left_prevsost;
  bool prev_dvig_right = right_prevsost;
  bool prev_dvig_up    = dvig_up_prevsost;
  bool prev_dvig_down  = dvig_down_prevsost;

  // --- Логика для горизонтального движения (Влево/Вправо) ---
  if (dvig_left) {
      digitalWrite(pin_d1_lt, LOW);
      digitalWrite(pin_d1_rt, HIGH);
      digitalWrite(pin_d2_up, HIGH);
      digitalWrite(pin_d2_dn, HIGH);

      if (!prev_dvig_left) { // Движение только началось (из выключенного в включенное состояние)
          start_time_left_motor = now;
          last_polog_update_left = now; // Инициализируем для непрерывного обновления
      }
      if ((now - start_time_left_motor) > INRUSH_CURRENT_BYPASS_MILLIS) {timer_dvig_left = true;}else{timer_dvig_left = false;}
      // Накапливаем положение только если ток выше порога и период пускового тока прошел
      if (I_dvig >= MIN_MOTOR_CURRENT_THRESHOLD && (now - start_time_left_motor) > INRUSH_CURRENT_BYPASS_MILLIS) {
          long delta_time = now - last_polog_update_left;
          polog_LR -= delta_time;
          real_dvig_left = true;
          stop_dvig_left = false; // ИСПРАВЛЕНО: Сбрасываем флаг "потенциальной остановки", так как двигатель движется
      }else{ 
          if (stop_dvig_left == false) {stop_dvig_left = true; stop_time_left_motor = now;} 
          if((now - stop_time_left_motor)>SOME_STOP_CONFIRM_DELAY){real_dvig_left = false;}
      }
      
      last_polog_update_left = now; // Всегда обновляем last_polog_update_left, пока двигатель включен

  } else if (dvig_right) {
      digitalWrite(pin_d1_rt, LOW);
      digitalWrite(pin_d1_lt, HIGH);
      digitalWrite(pin_d2_up, HIGH);
      digitalWrite(pin_d2_dn, HIGH);

      if (!prev_dvig_right) { // Движение только началось
          start_time_right_motor = now;
          last_polog_update_right = now;
      }
      
       if ((now - start_time_right_motor) > INRUSH_CURRENT_BYPASS_MILLIS) {timer_dvig_right = true;}else{timer_dvig_right = false;}
       
      if (I_dvig >= MIN_MOTOR_CURRENT_THRESHOLD && (now - start_time_right_motor) > INRUSH_CURRENT_BYPASS_MILLIS) {
          long delta_time = now - last_polog_update_right;
          polog_LR += delta_time;
          real_dvig_right = true;
          stop_dvig_right = false; // ИСПРАВЛЕНО: Сбрасываем флаг "потенциальной остановки", так как двигатель движется
      }else{
          if (stop_dvig_right == false) {stop_dvig_right = true; stop_time_right_motor = now;} 
          if((now - stop_time_right_motor)>SOME_STOP_CONFIRM_DELAY){real_dvig_right = false;}
      }
      
      last_polog_update_right = now;

  } else if (dvig_up) {
      digitalWrite(pin_d2_up, LOW);
      digitalWrite(pin_d1_lt, HIGH);
      digitalWrite(pin_d1_rt, HIGH);
      digitalWrite(pin_d2_dn, HIGH);

      if (!prev_dvig_up) { // Движение только началось
          start_time_up_motor = now;
          last_polog_update_up = now;
      }
      
       if ((now - start_time_up_motor) > INRUSH_CURRENT_BYPASS_MILLIS) {timer_dvig_up = true;}else{timer_dvig_up = false;}
       
      if (I_dvig >= MIN_MOTOR_CURRENT_THRESHOLD && (now - start_time_up_motor) > INRUSH_CURRENT_BYPASS_MILLIS) {
          long delta_time = now - last_polog_update_up;
          polog_UD -= delta_time;
          real_dvig_up = true;
          stop_dvig_up = false;
      }else{
          if (stop_dvig_up == false) {stop_dvig_up = true; stop_time_up_motor = now;} 
          if((now - stop_time_up_motor)>SOME_STOP_CONFIRM_DELAY){real_dvig_up = false;}
      }
      
      last_polog_update_up = now;

  } else if (dvig_down) {
      digitalWrite(pin_d2_dn, LOW);
      digitalWrite(pin_d1_lt, HIGH);
      digitalWrite(pin_d1_rt, HIGH);
      digitalWrite(pin_d2_up, HIGH);

      if (!prev_dvig_down) { // Движение только началось
          start_time_down_motor = now;
          last_polog_update_down = now;
      }

     if ((now - start_time_down_motor) > INRUSH_CURRENT_BYPASS_MILLIS) {timer_dvig_down = true;}else{timer_dvig_down = false;}
      
      if (I_dvig >= MIN_MOTOR_CURRENT_THRESHOLD && (now - start_time_down_motor) > INRUSH_CURRENT_BYPASS_MILLIS) {
          long delta_time = now - last_polog_update_down;
          polog_UD += delta_time;
          real_dvig_down = true;
          stop_dvig_down = false; // ИСПРАВЛЕНО: Сбрасываем флаг "потенциальной остановки", так как двигатель движется
      }else{
          if (stop_dvig_down == false) {stop_dvig_down = true; stop_time_down_motor = now;} // ИСПРАВЛЕНО: stop_dvig_right -> stop_dvig_down
          if((now - stop_time_down_motor)>SOME_STOP_CONFIRM_DELAY){real_dvig_down = false;}
      }
      last_polog_update_down = now;

  } else { // Двигатели не активны (все dvig_X = false)
      digitalWrite(pin_d1_lt, HIGH);
      digitalWrite(pin_d1_rt, HIGH);
      digitalWrite(pin_d2_up, HIGH);
      digitalWrite(pin_d2_dn, HIGH);

      // Когда двигатели выключены, сбрасываем last_polog_update_X на текущее время, чтобы предотвратить
      // накопление большого delta_time при следующем запуске двигателя после длительного простоя.
      last_polog_update_left = now;
      last_polog_update_right = now;
      last_polog_update_up = now;
      last_polog_update_down = now;

      // ИСПРАВЛЕНО: Сбрасываем все флаги таймеров и реального движения, когда двигатели неактивны
      timer_dvig_up = false;
      real_dvig_up = false;
      stop_dvig_up = false;
      stop_time_up_motor = 0;

      timer_dvig_down = false;
      real_dvig_down = false;
      stop_dvig_down = false;
      stop_time_down_motor = 0;

      timer_dvig_left = false;
      real_dvig_left = false;
      stop_dvig_left = false;
      stop_time_left_motor = 0;

      timer_dvig_right = false;
      real_dvig_right = false;
      stop_dvig_right = false;
      stop_time_right_motor = 0;
  }

  // Убедимся, что polog_LR и polog_UD не уходят в отрицательные значения
  if (polog_LR < 0) polog_LR = 0;
  if (polog_UD < 0) polog_UD = 0;

  // Обновляем индикаторы на дисплее на основе накопленного положения
  ind_LR = polog_LR * 0.001;
  ind_UD = polog_UD * 0.001;

//////////////////////////////////////////////////////////////////////////////////
// блок защит и проверок по времени и току
//////////////////////////////////////////////////////////////////////////////////
 
 
// тут будет защита по току 
if ((now - start_time_left_motor) > 1000) { // Проверяем задержку для LR
    is_motor_started = true;
}

      if ((dvig_left && timer_dvig_left) || (dvig_right && timer_dvig_right) || (dvig_up && timer_dvig_up) || (dvig_down && timer_dvig_down)) {
        
    check_I();}


 /////////////////////////////////
 /// тут будет защита по времени корекции
checkCorrectionTimeouts();
  // Управление встроенным светодиодом
  if (pin_indic) {
      digitalWrite(LED_BUILTIN, HIGH);
  } else {
      digitalWrite(LED_BUILTIN, LOW);
  }

  // Обновляем предыдущие состояния флагов в конце функции для следующей итерации цикла
  dvig_left_prevsost = dvig_left;
  right_prevsost = dvig_right;
  dvig_up_prevsost = dvig_up;
  dvig_down_prevsost = dvig_down;


// Динамически меняем интервал чтения тока
if (dvig_left || dvig_right || dvig_up || dvig_down) {
    // Если хоть один двигатель включен, читаем ток 5 раз в секунду
    currentInterval = 200; 
} else {
    // Иначе, читаем 1 раз в секунду
    currentInterval = 1000;
}
}
