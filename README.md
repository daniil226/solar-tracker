# ☀️ Dual-Axis Solar Tracker with OLED & Wind Protection
# ☀️ Двоосьовий сонячний трекер з OLED та захистом від вітру

---

## 🇺🇸 English Description

This project is an automated solar panel orientation system. It tracks the sun using 4 photoresistors (LDRs) and features built-in safety logic: if high wind is detected (via anemometer) or during heavy cloud cover, the panel moves to a safe parking position.

### 🛠 Project Structure
* **Firmware (Arduino/ESP):** Modular code including OLED display logic (GyverOLED), sensor processing, and motor control.
* **Hardware:** Includes a full electrical schematic (`Schematic.pdf`) and Gerber files for PCB manufacturing (`Gerber_project.zip`).
* **3D Printing:** Models for the supporting frame, anemometer parts, and sensor mounts.

### ⚙️ Key Features
- **Automatic Tracking:** Finds the point of maximum brightness.
- **Wind Protection:** Folds the tracker when critical wind speeds are detected via reed switch (Pin 2).
- **Cloud Mode:** Freezes movement during low light to save power.
- **OLED Status:** Real-time display of system state (Sost), light levels, and wind speed.

---

## 🇺🇦 Опис українською

Проєкт автоматизованої системи орієнтації сонячних панелей. Система відстежує положення сонця за допомогою 4-х фоторезисторів та має вбудовану логіку захисту: при сильному вітрі (дані з анемометра) або значній хмарності панель переходить у безпечне паркувальне положення.

### 🛠 Склад проєкту
* **Прошивка (Arduino/ESP):** Модульний код, що включає логіку OLED-дисплея (GyverOLED), обробку датчиків та керування двигунами.
* **Апаратна частина:** Повна електрична схема (`Schematic.pdf`) та Gerber-файли для замовлення друкованої плати (`Gerber_project.zip`).
* **3D-друк:** Моделі опорної рами, деталей анемометра та кріплень для датчиків.

### ⚙️ Основні функції
- **Автоматичне стеження:** Пошук точки максимальної яскравості.
- **Захист від вітру:** При досягненні критичних обертів анемометра (геркон на Pin 2) трекер автоматично складається.
- **Режим хмарності:** Якщо сонця довго немає, система завмирає для економії енергії.
- **Інфо-панель:** Відображення поточного стану (Sost), освітленості та швидкості вітру на OLED-екрані.

---
**Developed by: Daniil Guzenko**
