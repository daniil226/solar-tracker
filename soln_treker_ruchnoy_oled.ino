#include <Wire.h>
// #include <LiquidCrystal_I2C.h> // УДАЛЯЕМ эту строку
#include <GyverOLED.h> // ДОБАВЛЯЕМ библиотеку GyverOLED
#include <EEPROM.h>
#include "iarduino_RTC.h" // Включаем библиотеку iarduino_RTC
#include <GyverButton.h>
#include <avr/wdt.h> // Библиотека для аппаратного сторожевого таймера
#include <math.h> // Для функции isnan()

// --- Определения OLED и датчиков ---
// LiquidCrystal_I2C lcd(0x27, 20, 4); // УДАЛЯЕМ инициализацию LCD

// ДОБАВЛЯЕМ инициализацию OLED
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled; // Инициализация OLED дисплея 128x64 без буфера

// Адрес LCD 0x27 для дисплея 20x4 // КОММЕНТИРУЕМ или УДАЛЯЕМ эту строку, она больше не нужна

// Предполагаемое расположение датчиков:
// A0 --- A1 (Верх)
// |      |
// |      |
// A3 --- A2 (Низ)
const int sensorPins[] = {A0, A1, A2, A3};
const int numSensors = sizeof(sensorPins) / sizeof(sensorPins[0]);
const int analogInPin = A6; // Пин для датчика тока
float calibrationFactor[]={1,1,1,1};
const int HISTORY_SIZE = 5; // Размер буфера, можно изменить
int sensorHistory[4][HISTORY_SIZE];
int historyIndex[4] = {0, 0, 0, 0};
int sensorValues[numSensors];
const int MAX_DEVIATION = 200; // Максимально допустимое отклонение от медианы. Настройте это значение под свои датчики.
int count_err[4]={0, 0, 0, 0};
// Массив для хранения необработанных показаний датчиков

// --- Переменные калибровки (EEPROM) ---
// Адреса EEPROM для сохранения настроек
const int EEPROM_ADDR_UPDOWN = 0;
const int EEPROM_ADDR_LEFTRIGHT = EEPROM_ADDR_UPDOWN + sizeof(int);
const int EEPROM_ADDR_MIN_LIGH = EEPROM_ADDR_LEFTRIGHT + sizeof(int);
const int EEPROM_ADDR_ANEMOM_MAX = EEPROM_ADDR_MIN_LIGH + sizeof(int);
const int EEPROM_ADDR_I_MAX_LR = EEPROM_ADDR_ANEMOM_MAX + sizeof(int);
const int EEPROM_ADDR_I_MAX_UD = EEPROM_ADDR_I_MAX_LR + sizeof(float);
const int EEPROM_ADDR_T_MAX_UD = EEPROM_ADDR_I_MAX_UD + sizeof(float);
const int EEPROM_ADDR_T_MAX_LR = EEPROM_ADDR_T_MAX_UD + sizeof(float);


int offsetUpDown = 0;    // Смещение калибровки для направления Вверх/Вниз
int offsetLeftRight = 0;
// Смещение калибровки для направления Лево/Право



//////////////////////////////////// порты реле
#define pin_d1_lt    5 // Реле для движения влево
#define pin_d1_rt    6 // Реле для движения вправо
#define pin_d2_up    7 // Реле для движения вверх
#define pin_d2_dn    8 // Реле для движения вниз

// --- Определения кнопок ---
#define pin_left 12  // Пин кнопки "влево"
#define pin_enter 11 // Пин кнопки "ввод"
#define pin_rite 10  // Пин кнопки "вправо"

////////////////////////////////////порты доп устройств

#define pin_anemom  2 // Пин для анемометра (датчика ветра)
#define pin_ind  13 // Пин для индикаторного светодиода (LED_BUILTIN)

//////////////////////////////////////////////менять здесь назначение кнопок
GButton
left  (pin_left );
GButton enter (pin_enter);
GButton rite  (pin_rite );

// --- Определение RTC ---
iarduino_RTC watch(RTC_DS3231);
// Создаем объект RTC для DS1307 (для iarduino_RTC)

unsigned long currentMillis = 0;
unsigned long previousSensorMillis = 0;
const long sensorInterval = 1000; // Интервал для чтения датчиков и логики (1 секунда)

// int tic = 0;   // Неиспользуемая переменная
int ur_osv=0; //
float I_dvig=0;
// Переменная для хранения тока двигателя
// ОБНОВЛЕНО: Коэффициент шунта для 0.5 Ом и делителя 1/2 (22.0 / 1024.0)
const float koef_shunt=  0.075;
// Коэффициент шунта для пересчета в ток (Ампер на единицу АЦП)
//float koef_i=   0;
// Неиспользуемая переменная

// --- Новые константы для минимального тока и таймаута калибровки ---
const float MIN_MOTOR_CURRENT_THRESHOLD = 0.3;
// Минимальный порог тока двигателя, ниже которого считается 0
const unsigned long CALIBRATION_TIMEOUT_MILLIS = 5UL * 60UL * 1000UL;
// 5 минут для таймаута калибровки
const float OVERCURRENT_FACTOR = 1.2;
// Коэффициент для превышения тока (1.2 = 20% превышение)
const unsigned long INRUSH_CURRENT_BYPASS_MILLIS = 1000;
// 500мс для игнорирования пускового тока (пусковой ток может быть высоким)

// Переменные для отслеживания времени начала движения
int start_time_error=0;
static unsigned long start_time_left_motor  = 0;
static unsigned long start_time_right_motor  = 0;
static unsigned long start_time_up_motor  = 0;
static unsigned long start_time_down_motor  = 0;
unsigned long stop_time_left_motor=0;
unsigned long stop_time_right_motor=0;
unsigned long stop_time_up_motor=0;
unsigned long stop_time_down_motor=0;
///// умный контроль тока
unsigned long smart_time_I_left=0;
unsigned long smart_time_I_right=0;
unsigned long smart_time_I_up=0;
unsigned long smart_time_I_down=0;
unsigned long smart_time_I=0;
// Новые константы для таймаутов коррекции (добавьте их глобально в ваш .ino файл)
const unsigned long T_max_LR_CORRECTION_MS = 240 * 1000UL;
// Макс. время для горизонтальной коррекции (30 секунд)
const unsigned long T_max_UD_CORRECTION_MS = 180 * 1000UL; // Макс.
//время для вертикальной коррекции (30 секунд)

// Глобальные переменные для отслеживания времени начала движения в коррекции
unsigned long start_time_LR_correction  = 0;
// Для горизонтальной коррекции
unsigned long start_time_UD_correction  = 0; // Для вертикальной коррекции

// ОБНОВЛЕНО: Переменные для отслеживания времени последнего обновления положения
static unsigned long last_polog_update_left = 0;
static unsigned long last_polog_update_right = 0;
static unsigned long last_polog_update_up = 0;
static unsigned long last_polog_update_down = 0;
// --- Переменная для таймаута калибровки ---
unsigned long calibrationStepStartTime = 0;

unsigned long previousTimeDisplayMillis = 0;
long timeDisplayInterval = 1000;
// Интервал для обновления дисплея (1 секунда)

// Переменные для логики
int normalizedUpDown = 0;
int normalizedLeftRight = 0;
int diffLeftRight=0;
int diffUpDown=0;
int sumBottom=0;
int sumTop=0;
int sumLeft=0;
int sumRight=0;

byte vflag=0;
// Флаг для выбора основного экрана (0 - главный, 1 - настройки 1, 2 - настройки 2)
byte fs=0;
// Флаг для выбора подменю или пункта настройки
byte stat_disp=1; // Флаг для выбора подэкрана на главном дисплее

boolean  hand_dvig=false;
// Флаг состояния парковки
boolean  parkovka=false; // Флаг состояния парковки
boolean  stop_error=false; // Флаг состояния парковки
bool wind_overload_waiting = false;  // Флаг того, что мы сейчас ждем после скачка тока
int veter_overload_tick=0;
int veter_int_tick=0;

int Sost=0;
int ch_sost=0;
int last_vflag = -1; 
// Переменная состояния системы:

int Esensor=0; // Флаг ошибки датчиков освещения

int min_ligh=600; // Порог минимальной освещенности для режима "облачно"
int anemom_max=200;
// Порог максимальной скорости ветра
int T_max_LR = 300; // Максимальное время движения по горизонтали (калиброванное)
int T_max_UD = 300;
// Максимальное время движения по вертикали (калиброванное)
float I_max_LR=3; // Максимальный ток для горизонтального двигателя (калиброванный)
float I_max_UD=3;
float I_trend=0;
int y=0;
// Максимальный ток для вертикального двигателя (калиброванный)
float I_prev=0; // Максимальный ток для горизонтального двигателя (калиброванный)

// НОВЫЕ ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ДЛЯ ПЛАВНОЙ ЗАЩИТЫ ПО ТОКУ
float I_baseline_for_gradual = 0;
unsigned long last_gradual_baseline_update_time = 0;
const unsigned long GRADUAL_BASELINE_UPDATE_INTERVAL = 2000;
// Интервал для обновления базовой линии (в мс)
const float GRADUAL_OVERCURRENT_THRESHOLD_FACTOR = 1.3;
// Порог увеличения для плавного роста (1.3 = 30%)
// НОВЫЕ ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ДЛЯ ЗАЩИТЫ ПО ТОКУ (ТРЕНД И СКАЧОК)
float I_prev_spike = 0;
// Предыдущее значение тока для обнаружения мгновенных скачков
float I_prev_trend = 0;
// Предыдущее значение тока для обнаружения тренда
int consecutive_increase_count = 0; // Счетчик последовательных увеличений для тренда
unsigned long last_current_sample_time = 0;
// Время последней выборки тока для обеих проверок

const float TREND_INCREASE_THRESHOLD = 1.10;
// Порог увеличения для тренда (1.10 = 10% увеличение)
int trend=0;
const int CONSECUTIVE_TREND_COUNT = 4;
// Количество последовательных увеличений для срабатывания тренда
// float I_prev_LR=0; // Неиспользуемая переменная
// float I_prev_UD=0; // Неиспользуемая переменная
unsigned long I_max_delay= 200;
static bool is_moving = false; // Флаг для определения начала движения
static float I_base = 0.0;     // Переменная для хранения начального тока
// Глобальные переменные. Объявите их в начале вашего скетча.
float I_filtered = 0.0; // Плавающее базовое значение тока
const float CURRENT_SPIKE_FACTOR = 1.8; // Увеличьте это значение
const int CONSECUTIVE_SPIKE_COUNT = 8; // Увеличьте это значение
const float ALPHA = 0.3; // Коэффициент сглаживания. Чем меньше значение, тем медленнее адаптируется фильтр.
// Для отсчета времени
int counter = 0;
unsigned long parkovkaTimer = 0;
// Для отсчета времени
const unsigned long PARK_DELAY_MS = 5 * 60 * 1000UL;
// 5 минут задержки перед парковкой/выходом

const int PARK_ENTER_THRESHOLD = 100;
const int PARK_EXIT_THRESHOLD = 300;

// Переменные для нового таймера тока
unsigned long previousCurrentMillis = 0;
unsigned long currentInterval = 1000; // Начальный интервал чтения тока: 1000 мс

int regim=0;
int timerab=0;
// Текущее время в минутах от начала суток
int time_last_corr=0; // Время последней коррекции
int veter_max=0;
int veter_on_tick=0;
int start_cloudy=0;
int backligh=240 ;
// int kalibrovka=0; // Неиспользуемая переменная
float ind_UD=0; // Индикатор текущего положения по вертикали (для отображения)
float ind_LR=0;
// Индикатор текущего положения по горизонтали (для отображения)
long polog_UD=0; // Накопленное положение по вертикали (в миллисекундах работы двигателя)
long polog_LR=0;
// Накопленное положение по горизонтали (в миллисекундах работы двигателя)
float Pol_UD = 0;
// Положение по вертикали в условных единицах (пересчитанное из polog_UD)
float Pol_LR = 0;
// Положение по горизонтали в условных единицах (пересчитанное из polog_LR)
int Pol_UD_max = 3000000;
/// Максимальное калиброванное положение по вертикали (в миллисекундах)
int Pol_LR_max = 3000000;
/// Максимальное калиброванное положение по горизонтали (в миллисекундах)
int timer_cor = 0;

bool isCloudy = false;
// Флаг, указывающий на облачность
const int CLOUD_ENTER_OFFSET = 20; // Например, 20 единиц выше min_ligh
// Порог для выхода из облачности: среднее значение датчиков должно подняться выше (min_ligh + CLOUD_EXIT_OFFSET)
const int CLOUD_EXIT_OFFSET = 20;
// Например, 20 единиц выше min_ligh (должно быть > CLOUD_ENTER_OFFSET)
// Время (в секундах) для подтверждения стабильности состояния (облачно/ясно)
const int CLOUDY_DEBOUNCE_SECONDS = 60;
// Например, 60 секунд (1 минута)

boolean isWindy= false;             // Флаг для состояния ветра
unsigned long windProtectionStartTime = 0;
// Время начала защиты от ветра
unsigned long lowLightStartTime = 0;     // Время начала низкой освещенности
unsigned long sunShiningStartTime = 0;
// Добавлено: Время начала непрерывного солнечного света
unsigned long noNormalSunStartTime = 0;
// Добавлено: Время начала отсутствия нормального солнца
int correction_pass_count = 0;
// Добавлено: Счетчик проходов коррекции для двойного цикла
// unsigned long motorStopDetectionTime = 0;
// Неиспользуемая переменная
const unsigned long SOME_STOP_CONFIRM_DELAY = 500; // 500мс для подтверждения остановки двигателя

long time_tic_sek=0;
/// переменная тик тикают секунды с начала программы
long time_tic_min=0; /// переменная тик тикают минуты с начала программы
// unsigned long cloudyDeactivationStartTime = 0;
// Неиспользуемая переменная
// const unsigned long CLOUDY_DEACTIVATION_DURATION = 5UL * 60UL * 1000UL; // Неиспользуемая переменная
bool is_motor_started = false;
boolean pin_indic = false;
// Флаг для управления встроенным светодиодом
boolean dvig_left = false; // Флаг: двигатель движется влево
boolean dvig_left_prevsost=false;
// Предыдущее состояние флага dvig_left
boolean dvig_right= false; // Флаг: двигатель движется вправо
boolean right_prevsost=false; // Предыдущее состояние флага dvig_right
boolean dvig_up= false;
// Флаг: двигатель движется вверх
boolean dvig_up_prevsost=false; // Предыдущее состояние флага dvig_up
boolean dvig_down = false; // Флаг: двигатель движется вниз
boolean dvig_down_prevsost=false;
// Предыдущее состояние флага dvig_down
boolean real_dvig_left = false;
boolean real_dvig_right = false;
boolean real_dvig_up = false;
boolean real_dvig_down = false;
boolean timer_dvig_left = false;
boolean timer_dvig_right = false;
boolean timer_dvig_up = false;
boolean timer_dvig_down = false;
boolean stop_dvig_left = false;
boolean stop_dvig_right = false;
boolean stop_dvig_up = false;
boolean stop_dvig_down = false;


unsigned long lastDebounceTime = 0;
// Последнее время переключения входного пина (для анемометра)
unsigned long debounceDelay = 50;
// Время подавления дребезга (для анемометра)

// Переменные для подсчета оборотов анемометра
volatile int rotationCount = 0;
// Используем 'volatile', так как переменная изменяется в прерывании (ISR)
unsigned long lastSecondMillis = 0;
// Последнее время обновления оборотов в секунду
int rotationsPerSecond = 0;
// Хранит рассчитанные обороты в секунду (для анемометра)

// Флаг для отслеживания изменений настроек для EEPROM
bool settingsChanged = false;
unsigned long lastSettingsSaveMillis = 0;
//const unsigned long SETTINGS_SAVE_INTERVAL = 60UL * 60UL * 1000UL;
// Сохранять настройки каждые 60 минут

// --- Переменные для обработки ошибок и перезапуска ---
int error_retry_count = 0;
// Счетчик попыток перезапуска после ошибки
unsigned long last_error_retry_time = 0; // Время последней попытки перезапуска
const int MAX_ERROR_RETRIES = 5;
// Максимальное количество попыток перезапуска
const unsigned long ERROR_RETRY_INTERVAL_MS = 5UL * 60UL * 1000UL;
// Интервал между попытками (5 минут)

// --- ПРОТОТИПЫ ФУНКЦИЙ ---
// Объявляем все функции, чтобы компилятор знал о них до их использования.
void moov();
void updateCloudyStatus();
void knopki();
void sensorread();
void logika();
void displey();
void ISR_anemom();
void saveSettings();
// Прототип функции сохранения настроек
void dvig(); // Прототип функции dvig()
void handleErrorRecovery();
// Прототип функции обработки ошибок и перезапуска


// --- НАСТРОЙКА (SETUP) ---
void setup() {
  // Serial.begin(9600);
// Инициализация последовательного порта для отладки
  // Serial.println("Starting Solar Tracker...");
   // Заполняем буфер начальными данными
  for (int i = 0; i < HISTORY_SIZE*2; i++) {
    sensorread();
    delay(100); // Небольшая задержка, чтобы показания были разные
  }
  timerab=0;
// Инициализация timerab, так как watch.Hours и watch.minutes больше не используются
  // ИСПРАВЛЕНИЕ: Инициализация time_last_corr, чтобы коррекция началась сразу
 // time_last_corr = timerab - 16;
// Установка time_last_corr так, чтобы условие timerab - time_last_corr > 15 было сразу истинным


  //watch.begin(&Wire);
// &Wire1, &Wire2 ... // Инициализация RTC (объект watch все еще нужен для RTC)

  // Инициализация LCD // КОММЕНТИРУЕМ или УДАЛЯЕМ
  // lcd.init();
  // lcd.backlight();
  // lcd.clear();
  // lcd.setCursor(0,0);
  // lcd.print("Solar Tracker Init");

  // Инициализация OLED
  oled.init(); // Инициализация дисплея
  oled.clear(); // Очистка дисплея
  oled.home();  // Перемещение курсора в начало (0,0)
  oled.print("загрузка"); // Вывод текста
  // oled.update(); // Для OLED_BUFFERED, но у нас OLED_NO_BUFFER

  // Настройка пинов кнопок с внутренними подтягивающими резисторами
  //////////////////////////////////////////////////////////////////////////////// кнопки
  left.setDebounce(50);
// настройка антидребезга (по умолчанию 80 мс)
  left.setTimeout(1000);        // настройка таймаута на удержание (по умолчанию 500 мс)
  left.setClickTimeout(300);
// настройка таймаута между кликами (по умолчанию 300 мс)
  enter.setDebounce(50);        // настройка антидребезга (по умолчанию 80 мс)
  enter.setTimeout(1000);
// настройка таймаута на удержание (по умолчанию 500 мс)
  enter.setClickTimeout(300);
// настройка таймаута между кликами (по умолчанию 300 мс)
  rite.setDebounce(50);        // настройка антидребезга (по умолчанию 80 мс)
  rite.setTimeout(1000);
// настройка таймаута на удержание (по умолчанию 500 мс)
  rite.setClickTimeout(300);
// настройка таймаута между кликами (по умолчанию 300 мс)

  pinMode(pin_anemom   , INPUT_PULLUP);
// Пин анемометра как вход с подтяжкой
  pinMode(LED_BUILTIN, OUTPUT); // Встроенный светодиод как выход
  pinMode(pin_d1_lt , OUTPUT);// Реле как выходы
  pinMode(pin_d1_rt, OUTPUT);
  pinMode(pin_d2_up, OUTPUT);
  pinMode(pin_d2_dn, OUTPUT);
      digitalWrite(pin_d1_rt, HIGH);
      digitalWrite(pin_d1_lt, HIGH);
      digitalWrite(pin_d2_up, HIGH);
      digitalWrite(pin_d2_dn, HIGH);
  // Загружаем все настройки из EEPROM
  EEPROM.get(EEPROM_ADDR_UPDOWN, offsetUpDown);
EEPROM.get(EEPROM_ADDR_LEFTRIGHT, offsetLeftRight);
  EEPROM.get(EEPROM_ADDR_MIN_LIGH, min_ligh);
  EEPROM.get(EEPROM_ADDR_ANEMOM_MAX, anemom_max);
  EEPROM.get(EEPROM_ADDR_I_MAX_LR, I_max_LR);
  EEPROM.get(EEPROM_ADDR_I_MAX_UD, I_max_UD);
  EEPROM.get(EEPROM_ADDR_T_MAX_UD, T_max_UD);
  EEPROM.get(EEPROM_ADDR_T_MAX_LR, T_max_LR);
    EEPROM.get(30, calibrationFactor[0]);
  EEPROM.get(35, calibrationFactor[1]);
  EEPROM.get(40, calibrationFactor[2]);
  EEPROM.get(45, calibrationFactor[3]);
// Проверка и коррекция значений, загруженных из EEPROM
  if (offsetUpDown < -500 || offsetUpDown > 500) { offsetUpDown = 0;
settingsChanged = true; }
  if (offsetLeftRight < -500 || offsetLeftRight > 500) { offsetLeftRight = 0; settingsChanged = true;
}
  if (min_ligh < 0 || min_ligh > 1023) { min_ligh = 600; settingsChanged = true;
}
  if (anemom_max < 0 || anemom_max > 30) { anemom_max = 20; settingsChanged = true;
}
  
  // Проверка на NaN и диапазон для I_max_LR и I_max_UD
  if (isnan(I_max_LR) || I_max_LR < 0.1 || I_max_LR > 10.0) { I_max_LR = 2.0;
settingsChanged = true; }
  if (isnan(I_max_UD) || I_max_UD < 0.1 || I_max_UD > 10.0) { I_max_UD = 2.0;
settingsChanged = true; }

  if (T_max_UD < 0 || T_max_UD > 600) { T_max_UD = 300; settingsChanged = true;
} // Макс 10 минут
  if (T_max_LR < 0 || T_max_LR > 600) { T_max_LR = 300;
settingsChanged = true; } // Макс 10 минут

  attachInterrupt(digitalPinToInterrupt(pin_anemom), ISR_anemom, FALLING);
// Настройка прерывания для анемометра

  // Инициализация аппаратного сторожевого таймера
  wdt_enable(WDTO_8S);
// Включаем сторожевой таймер с таймаутом 8 секунд (ближайшее к 15с)
  
  // Вызываем logika() для начальной установки timerab и других логических переменных
  // перед тем, как использовать timerab для time_last_corr.
logika();
  // ИСПРАВЛЕНИЕ: Инициализация time_last_corr, чтобы коррекция началась сразу
  // Перенесено сюда, после того как timerab будет установлен в logika()
    time_last_corr = 0;
// Инициализация isCloudy на основе текущих показаний датчиков
    // Сначала считываем датчики, чтобы sensorValues были актуальны
    sensorread();
if ((sensorValues[0] + sensorValues[1] + sensorValues[2] + sensorValues[3]) / 4 < min_ligh) {
        isCloudy = true;
// Если среднее значение ниже min_ligh, считаем, что облачно/темно
    } else {
        isCloudy = false;
// Иначе считаем, что ясно
    }
    start_cloudy = 0;
// Сбрасываем счетчик при старте
}

// --- ЦИКЛ основной ---
void loop() {
  currentMillis = millis();
// Получаем текущее время в миллисекундах
  wdt_reset(); // Сбрасываем сторожевой таймер, чтобы предотвратить сброс Arduino
  rite.tick();
// Обновление состояния кнопок
  enter.tick();
  left.tick();
  moov(); // Функция управления двигателями и измерения тока
  knopki();
// Функцию knopki() лучше вызывать постоянно для отзывчивости
  dvig(); // Добавлено: Вызов функции автоматического управления движением
  if (Sost >= 50) {handleErrorRecovery();}
  
// НОВЫЙ ВЫЗОВ: Обработка ошибок и перезапуск
  // --- Таймер для чтения датчиков и логики ---
  // Этот блок будет выполняться каждые 'sensorInterval' миллисенд
  if (currentMillis - previousSensorMillis >= sensorInterval) {
    previousSensorMillis = currentMillis;
// Обновляем предыдущее время
    //if (claudi_min>0)()
    // Часть 2: Управление длительностью состояния ветра (исправлено, выполняется каждую секунду)
   if (veter_on_tick > 0) { veter_on_tick--;} else  { veter_on_tick=0;}
 
  
    sensorread();
// Считываем датчики
    logika();     // Обрабатываем логику на основе новых показаний
    // if (kalibrovka>=1){kalibrovka++;} // Неиспользуемая логика
  }

  // --- Отдельный таймер для обновления дисплея ---
  // Этот блок будет выполняться каждые 'timeDisplayInterval' миллисекунд
  if (currentMillis - previousTimeDisplayMillis >= timeDisplayInterval) {
    previousTimeDisplayMillis = currentMillis;
// Обновляем предыдущее время

    displey(); // Обновляем только дисплей (считанные датчики уже будут обновлены, если это нужно)
    timeDisplayInterval=1000;
// Сброс интервала для следующего обновления дисплея

  }
  // --- Новый таймер для чтения тока ---
  if (currentMillis - previousCurrentMillis >= currentInterval) {
    previousCurrentMillis = currentMillis;
    readCurrentSensor(); // Вызываем новую функцию для чтения тока
  }

}
