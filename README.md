# Mașinuță controlată wireless prin gestică

Acest repository documentează dezvoltarea unei **mașinuțe controlate prin gesturi**, bazată pe o arhitectură **master–slave** între două microcontrolere:

* **ESP32-C6** — controller principal pentru comunicație și controlul funcțiilor auxiliare
* **Arduino Uno** — controller pentru acționarea motoarelor, citirea senzorilor și controlul unor actuatoare

Proiectul explorează o metodă de interacțiune naturală om–mașină, în care comenzile sunt transmise prin **gestică**, fără joystick sau telecomandă clasică.

---

## Descărcare software pentru control

[![Download](https://img.shields.io/badge/⬇-Download_Software-success?style=for-the-badge)](https://github.com/rafaelmihaifrumosu-gif/Masinuta-controlata-wireless-prin-gestica/releases/tag/0.1)


---

# Cuprins

* [Descriere generală](#descriere-generală)
* [Arhitectura sistemului](#arhitectura-sistemului)
* [Funcționalități](#funcționalități)
* [Componente hardware](#componente-hardware)
* [Pinaj / conexiuni hardware](#pinaj--conexiuni-hardware)
* [Structura repository-ului](#structura-repository-ului)
* [Build & Flash](#build--flash)
* [Cum funcționează](#cum-funcționează)


---

# Descriere generală

Proiectul constă într-o mașinuță robotică echipată cu:

* **4 motoare TT** pentru deplasare
* **2 punți H MX1508** pentru controlul motoarelor
* **senzor ultrasonic HC-SR04** pentru detecția obstacolelor
* **servomotoare SG90** pentru acționarea unor elemente mecanice
* **LED-uri** pentru faruri și semnalizare
* **buzzer activ** pentru feedback sonor
* **ESP32-C6** pentru comunicație și controlul funcțiilor auxiliare
* **Arduino Uno** pentru controlul motoarelor și al senzorilor

Sistemul este alimentat din acumulatori Li-Ion, iar tensiunile necesare sunt adaptate prin convertoare **LM2596** și **MT3608**.

---

# Arhitectura sistemului

Arhitectura proiectului este de tip **master–slave**:

* **ESP32-C6 (master)**

  * gestionează comunicația și logica de control de nivel superior
  * primește sau interpretează comenzile de gestică
  * controlează funcții auxiliare precum farurile și semnalizarea
  * comunică cu Arduino Uno prin interfață **I2C**

* **Arduino Uno (slave)**

  * controlează cele două punți H pentru motoarele de tracțiune
  * citește senzorul ultrasonic
  * acționează servomotoarele
  * execută comenzile primite de la ESP32-C6

## Flux logic al sistemului

```text
[Gest utilizator / comandă]
          ↓
      [ESP32-C6]
  (master / comunicație)
          ↓
         I2C
          ↓
     [Arduino Uno]
 (motoare / senzori / servo)
          ↓
 [deplasare + actuatoare + feedback]
```

---

# Funcționalități

* control wireless al mașinuței
* control prin gestică
* acționare independentă a celor 4 motoare prin 2 punți H
* detecție de obstacole cu senzor ultrasonic
* control pentru:

  * faruri
  * semnal stânga
  * semnal dreapta
  * buzzer
* controlul a două servomotoare

---

# Componente hardware

## ⚙️ Motoare & mecanică

| Cantitate | Componentă                |
| --------- | ------------------------- |
| x4        | Motor reductor TT         |
| x4        | Roată plastic cu cauciuc  |
| x1        | Senzor ultrasonic HC-SR04 |
| x2        | Servomotor SG90           |

## 🔋 Alimentare

| Cantitate | Componentă                      |
| --------- | ------------------------------- |
| x2        | Acumulator Li-Ion               |
| x2        | Modul ridicător tensiune MT3608 |
| x1        | Modul coborâtor tensiune LM2596 |

## 🤖 Control & electronică

| Cantitate | Componentă           |
| --------- | -------------------- |
| x1        | Arduino Uno          |
| x1        | ESP32-C6             |
| x2        | Punte H dublă MX1508 |
| x1        | Buzzer activ         |
| x1        | LED faruri           |
| x1        | LED semnal stânga    |
| x1        | LED semnal dreapta   |

---

# Pinaj / conexiuni hardware

## 1) Alimentare

### Modul LM2596

| Pin modul | Conexiune Arduino Uno | Conexiune ESP32-C6 |
| --------- | --------------------: | -----------------: |
| OUT+      |                    5V |                VIN |
| OUT-      |                   GND |                GND |

### Baterii

| Pin | Conexiune        |
| --- | ---------------- |
| GND | GND comun sistem |


---

## 2) Comunicație între ESP32-C6 și Arduino Uno

### Magistrală I2C

| Semnal | Arduino Uno |    ESP32-C6 |
| ------ | ----------: | ----------: |
| SDA    |          A4 | GPIO 4 |
| SCL    |          A5 | GPIO 5 |


Exemplu final recomandat (după verificare):

* SDA → **GPIO X**
* SCL → **GPIO Y**

---

## 3) Iluminare și semnalizare

| Componentă         | Pin componentă | Conectare       |
| ------------------ | -------------- | --------------- |
| LED faruri         | GND            | GND             |
| LED faruri         | VCC            | ESP32-C6 GPIO18 |
| LED semnal stânga  | VCC            | ESP32-C6 GPIO19 |
| LED semnal dreapta | VCC            | ESP32-C6 GPIO20 |


---

## 4) Senzor ultrasonic HC-SR04

| Pin HC-SR04 | Arduino Uno |
| ----------- | ----------: |
| TRIG        |          A1 |
| ECHO        |          A2 |
| VCC         |         VCC |
| GND         |         GND |

---

## 5) Punți H pentru motoare

### Punte H1 — motoare stânga

| Conexiune |           Arduino Uno |
| --------- | --------------------: |
| Motor A   |           Stânga față |
| Motor B   |          Stânga spate |
| IN1 + IN3 |   D9 *(mișcare față)* |
| IN2 + IN4 | D10 *(mișcare spate)* |
| +         |                   VCC |
| -         |                   GND |

### Punte H2 — motoare dreapta

| Conexiune |          Arduino Uno |
| --------- | -------------------: |
| Motor A   |         Dreapta față |
| Motor B   |        Dreapta spate |
| IN1 + IN3 | D11 *(mișcare față)* |
| IN2 + IN4 | D3 *(mișcare spate)* |
| +         |                  VCC |
| -         |                  GND |

---

## 6) Servomotoare

### Servomotor ușă dreapta

| Pin servomotor | Arduino Uno |
| -------------- | ----------: |
| Semnal         |         D10 |
| VCC            |         VCC |
| GND            |         GND |

### Servomotor ușă stânga

| Pin servomotor | Arduino Uno |
| -------------- | ----------: |
| Semnal         |          D9 |
| VCC            |         VCC |
| GND            |         GND |

---

## 7) Buzzer activ

 
| Pin buzzer | Microcontroler    | Pin       |
| ---------- | ----------------- | --------- |
| Semnal     | Arduino sau ESP32 | GPIO |
| VCC        | VCC               |           |
| GND        | GND               |           |

---

# Structura repository-ului

O structură recomandată pentru repo ar putea fi:

```text
.
├── arduino-uno/
    ├── bin/
    ├── bsp/
    ├── drivers/
        ├── adc/
        ├── button/
        ├── buzzer/
        ├── eeprom/
        ├── gpio/
        ├── i2c/
        ├── intrerrupt/
        ├── led/
        ├── motor/
        ├── pwm/
        ├── servo/
        ├── timer/
        ├── ultrasonic/
        ├── usart/
    ├── img/
    ├── obj
        ├── drivers/
            ├── adc/
            ├── button/
            ├── buzzer/
            ├── eeprom/
            ├── gpio/
            ├── i2c/
            ├── intrerrupt/
            ├── led/
            ├── motor/
            ├── pwm/
            ├── servo/
            ├── timer/
            ├── ultrasonic/
            ├── usart/
        ├── src/
        └── utils/
    ├── src/
    ├── test/
    ├── utils/
    ├── LICENSE
    ├── Makefile
    ├── PrezentareProiect_HTML
    └── README.md
├── esp/
    ├── .devcontainer/
    ├── .vscode/
    ├── main
    ├── .clangd
    ├── .gitingore
    ├── CMakeLists.txt
    ├── build.log.txt
    └── build_output.txt 
├── control_python/
     └── control_led.py
├── Project Sentinel.py
└── README.md
```


---

# Build & Flash

## Prerequisites

### Pentru Arduino Uno

* `avr-gcc`
* `avrdude`
* `make`

### Pentru ESP32-C6

* **ESP-IDF**
* `idf.py`

---

## Build & flash — Arduino Uno

În directorul firmware-ului pentru Arduino Uno:

| Comandă      | Descriere                            |
| ------------ | ------------------------------------ |
| `make all`   | Compilează proiectul                 |
| `make flash` | Scrie firmware-ul pe placa conectată |
| `make clean` | Șterge fișierele generate            |

---

## Build & flash — ESP32-C6

În directorul firmware-ului pentru ESP32-C6:

| Comandă                | Descriere                            |
| ---------------------- | ------------------------------------ |
| `idf.py build`         | Compilează proiectul                 |
| `idf.py -p PORT flash` | Scrie firmware-ul pe placa conectată |
| `idf.py clean`         | Șterge fișierele generate            |

Exemplu:

```bash
idf.py -p COM5 flash
```

sau pe Linux:

```bash
idf.py -p /dev/ttyUSB0 flash
```

---

# Cum funcționează

## 1. Alimentare

Sistemul este alimentat din acumulatori Li-Ion. Tensiunea este adaptată prin convertoare DC-DC pentru a furniza nivelele necesare către:

* Arduino Uno
* ESP32-C6
* punțile H și motoarele DC
* servomotoare și senzori

## 2. Control

ESP32-C6 primește comenzi de control și le transmite către Arduino Uno prin I2C. Arduino execută comenzile și controlează:

* motoarele stânga/dreapta
* servomotoarele
* senzorul ultrasonic
* alte acțiuni de bază necesare deplasării

## 3. Acționare

Motoarele sunt împărțite în două grupuri:

* **grup stânga** → controlat prin Puntea H1
* **grup dreapta** → controlat prin Puntea H2

Prin combinarea semnalelor pentru punțile H, mașinuța poate executa:

* deplasare înainte
* deplasare înapoi
* viraj stânga
* viraj dreapta
* oprire

---
