#include "bsp/uno.h"
#include "drivers/timer/timer0.h"
#include "drivers/i2c/i2c_slave.h"
#include "drivers/motor/motor.h"
#include "drivers/buzzer/buzzer.h"
#include "drivers/servo/servo.h"
#include "drivers/ultrasonic/ultrasonic.h"
#include "drivers/alarm/alarm.h" // <-- Noul driver

#define I2C_SLAVE_ADDR 0x08

static uint8_t usa_stanga_deschisa = 0;
static uint8_t usa_dreapta_deschisa = 0;

static uint8_t secventa_claxon_activa = 0;
static uint8_t pas_claxon = 0;
static uint32_t timp_claxon_anterior = 0;

// Variabila globala pentru parcare
char mod_parcare_activ = '0'; 

void Gestionare_Secventa_Claxon(void) {
    if (!secventa_claxon_activa) return;

    uint32_t timp_curent = Millis();

    switch (pas_claxon) {
        case 0:
            BUZZER_Beep(200); 
            timp_claxon_anterior = timp_curent;
            pas_claxon = 1;
            break;
        case 1:
            if (timp_curent - timp_claxon_anterior >= 400) { 
                BUZZER_Beep(200); 
                timp_claxon_anterior = timp_curent;
                pas_claxon = 2;
            }
            break;
        case 2:
            if (timp_curent - timp_claxon_anterior >= 400) {
                BUZZER_Beep(200); 
                secventa_claxon_activa = 0; 
                pas_claxon = 0;
            }
            break;
    }
}

void Executa_Parcare_Fata(void);
void Executa_Parcare_Spate(void);
void Executa_Parcare_Lateral_Stanga(void);
void Executa_Parcare_Lateral_Dreapta(void);

int main(void) {
    Timer0_Init();
    MOTOR_Init(); 
    BUZZER_Init(); // Active buzzer
    SERVO_Init();
    ULTRASONIC_Init();
    ALARM_Init(); // <-- Initializam masina de stare a alarmei
    i2c_slave_init(I2C_SLAVE_ADDR);

    SERVO_SetAngle(SERVO_CH_B, 0); 
    SERVO_SetAngle(SERVO_CH_A, 0); 

    uint8_t comanda_primita = 0;

    while (1) {
        // --- PROCESE DE FUNDAL ---
        BUZZER_Update();
        ULTRASONIC_Update(); 
        Gestionare_Secventa_Claxon();
        ALARM_Update(); // <-- Proceseaza logica alarmei non-stop

        // --- SISTEME DE PARCARE AUTOMATA ---
        if (mod_parcare_activ == '1') Executa_Parcare_Fata();
        else if (mod_parcare_activ == '2') Executa_Parcare_Spate();
        else if (mod_parcare_activ == '3') Executa_Parcare_Lateral_Stanga();
        else if (mod_parcare_activ == '4') Executa_Parcare_Lateral_Dreapta();

        // --- CITIRE COMENZI I2C ---
        if (i2c_slave_has_data(&comanda_primita)) {
            
            if (comanda_primita == '1' || comanda_primita == '2' || 
                comanda_primita == '3' || comanda_primita == '4') {
                mod_parcare_activ = comanda_primita;
                ALARM_Disarm(); // Dezactivam alarma daca se incepe o parcare
                continue; 
            }
            
            if (comanda_primita == 'S') {
                mod_parcare_activ = '0';
                MOTOR_Drive(DIR_STOP, 0);
                ALARM_Disarm(); // Resetam alarma la oprirea de urgenta
                continue;
            }

            switch (comanda_primita) {
                case 'X': // Mod Securitate Armata
                    mod_parcare_activ = '0';
                    MOTOR_Drive(DIR_STOP, 0); // Asigura-te ca masina sta pe loc
                    ALARM_Arm();
                    // Optional: Un bip scurt de confirmare a armarii
                    BUZZER_Beep(100); 
                    break;

                case 'L': 
                    if (usa_stanga_deschisa) {
                        SERVO_SetAngle(SERVO_CH_B, 0); 
                        usa_stanga_deschisa = 0;
                    } else {
                        SERVO_SetAngle(SERVO_CH_B, 90); 
                        usa_stanga_deschisa = 1;
                    }
                    break;
                    
                case 'R': 
                    if (usa_dreapta_deschisa) {
                        SERVO_SetAngle(SERVO_CH_A, 0); 
                        usa_dreapta_deschisa = 0;
                    } else {
                        SERVO_SetAngle(SERVO_CH_A, 90); 
                        usa_dreapta_deschisa = 1;
                    }
                    break;
                    
                case 'C': 
                    if (!secventa_claxon_activa) {
                        secventa_claxon_activa = 1;
                        pas_claxon = 0;
                    }
                    break;

                default:
                    break;
            }
        }
    }
    return 0;
}

void Executa_Parcare_Fata(void) {
    uint16_t distanta_fata = ULTRASONIC_GetDistance(0); 

    if (distanta_fata == 0 || distanta_fata > 400) {
        return; 
    }

    if (distanta_fata > 15) {
        MOTOR_Drive(DIR_FATA, 150); 
    } else {
        MOTOR_Drive(DIR_STOP, 0); 
        mod_parcare_activ = '0'; 
    }
}

void Executa_Parcare_Spate(void) {
}

void Executa_Parcare_Lateral_Stanga(void) {
}

void Executa_Parcare_Lateral_Dreapta(void) {
    // Variabile statice pentru a "tine minte" starea intre iteratiile buclei while(1)
    static uint8_t pas = 0; 
    static uint32_t timp_start = 0;

    // Citim distantele
    uint16_t d_fata = ULTRASONIC_GetDistance(0);
    uint16_t d_dreapta = ULTRASONIC_GetDistance(2);

    switch (pas) {
        case 0: // PASUL 0: Cautare gol parcare
            MOTOR_Drive(DIR_FATA, 120); // Mergem incet in fata
            
            // Daca distanta laterala creste brusc (peste 35cm inseamna ca am gasit un gol)
            // Ne asiguram ca ignoram 999 (eroare senzor)
            if (d_dreapta > 35 && d_dreapta != 999) {
                pas = 1;
                timp_start = Millis();
            }
            break;

        case 1: // PASUL 1: Aliniere punte spate
            // Mai mergem putin in fata ca masina sa nu loveasca "masina parcata" din spate cand roteste
            MOTOR_Drive(DIR_FATA, 120);
            
            if (Millis() - timp_start > 400) { // Calibrare: aprox 400ms de inaintare
                MOTOR_Drive(DIR_STOP, 0);
                pas = 2;
                timp_start = Millis();
            }
            break;

        case 2: // PASUL 2: Rotire 90 grade Dreapta
            // Motoarele stanga trag in fata, motoarele dreapta trag in spate
            MOTOR_Drive(DIR_DREAPTA, 180); // Putere mai mare pentru rotire pe loc
            
            if (Millis() - timp_start > 650) { // Calibrare: cat timp ii ia sa faca EXACT 90 grade
                MOTOR_Drive(DIR_STOP, 0);
                pas = 3;
            }
            break;

        case 3: // PASUL 3: Intrare in parcare 
            // Acum masina e orientata cu fata spre marginea drumului.
            // Inaintam pana cand senzorul frontal zice ca suntem aproape de bordura (12 cm).
            if (d_fata > 12) {
                MOTOR_Drive(DIR_FATA, 120);
            } else {
                MOTOR_Drive(DIR_STOP, 0);
                pas = 4;
                timp_start = Millis();
            }
            break;

        case 4: // PASUL 4: Indreptare masina (Rotire 90 grade Stanga)
            MOTOR_Drive(DIR_STANGA, 180); 
            
            if (Millis() - timp_start > 650) { // Acelasi timp de pivotare ca la Pasul 2
                MOTOR_Drive(DIR_STOP, 0);
                pas = 0;                 // Resetam starea pentru viitoarele parcari
                mod_parcare_activ = '0'; // Dezactivam modul de parcare automat
            }
            break;
    }
}