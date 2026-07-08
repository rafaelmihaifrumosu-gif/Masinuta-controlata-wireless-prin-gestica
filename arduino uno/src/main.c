
#include "bsp/uno.h"
#include "drivers/timer/timer0.h"
#include "drivers/i2c/i2c_slave.h"
#include "drivers/motor/motor.h"
#include "drivers/buzzer/buzzer.h"
#include "drivers/servo/servo.h"
#include "drivers/ultrasonic/ultrasonic.h"

// Adresa I2C alocata placii Arduino
#define I2C_SLAVE_ADDR 0x08

// Stari usi (0 = inchis, 1 = deschis la 90 grade)
static uint8_t usa_stanga_deschisa = 0;
static uint8_t usa_dreapta_deschisa = 0;

// Variabile pentru secventa "Gasire Masina" (Claxon non-blocant)
static uint8_t secventa_claxon_activa = 0;
static uint8_t pas_claxon = 0;
static uint32_t timp_claxon_anterior = 0;

// Functie auxiliara care proceseaza cele 3 bipuri in fundal
void Gestionare_Secventa_Claxon(void) {
    if (!secventa_claxon_activa) return;

    uint32_t timp_curent = Millis();

    switch (pas_claxon) {
        case 0:
            BUZZER_Beep(200); // Primul bip (200ms)
            timp_claxon_anterior = timp_curent;
            pas_claxon = 1;
            break;
        case 1:
            if (timp_curent - timp_claxon_anterior >= 400) { 
                BUZZER_Beep(200); // Al doilea bip
                timp_claxon_anterior = timp_curent;
                pas_claxon = 2;
            }
            break;
        case 2:
            if (timp_curent - timp_claxon_anterior >= 400) {
                BUZZER_Beep(200); // Al treilea bip
                secventa_claxon_activa = 0; // Oprim secventa
                pas_claxon = 0;
            }
            break;
    }
}

// Prototipuri pentru viitorii algoritmi de parcare autonoma
void Executa_Parcare_Fata(void);
void Executa_Parcare_Spate(void);
void Executa_Parcare_Lateral_Stanga(void);
void Executa_Parcare_Lateral_Dreapta(void);

int main(void) {
    // 1. Initializare hardware si drivere
    Timer0_Init();
    MOTOR_Init();
    BUZZER_Init();
    SERVO_Init();
    ULTRASONIC_Init();
    i2c_slave_init(I2C_SLAVE_ADDR);

    // 2. Aducem usile in pozitia initiala (inchis)
    SERVO_SetAngle(SERVO_CH_B, 0); // Usa Stanga (Pin D10)
    SERVO_SetAngle(SERVO_CH_A, 0); // Usa Dreapta (Pin D9)

    uint8_t comanda_primita = 0;
    char mod_parcare_activ = '0'; // '0' inseamna ca parcarea este oprita

    while (1) {
        // --- PROCESE DE FUNDAL ---
        // Aceste functii trebuie rulate continuu pentru siguranta si temporizari
        BUZZER_Update();
        ULTRASONIC_Update(); // Protectia la impact frontal
        Gestionare_Secventa_Claxon();

        // --- SISTEME DE PARCARE AUTOMATA ---
        if (mod_parcare_activ == '1') Executa_Parcare_Fata();
        else if (mod_parcare_activ == '2') Executa_Parcare_Spate();
        else if (mod_parcare_activ == '3') Executa_Parcare_Lateral_Stanga();
        else if (mod_parcare_activ == '4') Executa_Parcare_Lateral_Dreapta();

        // --- CITIRE COMENZI I2C ---
        if (i2c_slave_has_data(&comanda_primita)) {
            
            // Verificam intai daca este o comanda de parcare
            if (comanda_primita == '1' || comanda_primita == '2' || 
                comanda_primita == '3' || comanda_primita == '4') {
                mod_parcare_activ = comanda_primita;
                continue; // Sarim peste restul loop-ului si incepem manevra
            }
            
            // Oprire de Urgenta sau anulare parcare
            if (comanda_primita == 'S') {
                mod_parcare_activ = '0';
                MOTOR_Drive(DIR_STOP, 0);
                continue;
            }

            // Tratare comenzi actionare componente caroserie
            switch (comanda_primita) {
                case 'U': // Usa Stanga
                    if (usa_stanga_deschisa) {
                        SERVO_SetAngle(SERVO_CH_B, 0); 
                        usa_stanga_deschisa = 0;
                    } else {
                        SERVO_SetAngle(SERVO_CH_B, 90); 
                        usa_stanga_deschisa = 1;
                    }
                    break;
                    
                case 'I': // Usa Dreapta
                    if (usa_dreapta_deschisa) {
                        SERVO_SetAngle(SERVO_CH_A, 0); 
                        usa_dreapta_deschisa = 0;
                    } else {
                        SERVO_SetAngle(SERVO_CH_A, 90); 
                        usa_dreapta_deschisa = 1;
                    }
                    break;
                    
                case 'C': // Declansare secventa Gasire Masina
                    if (!secventa_claxon_activa) {
                        secventa_claxon_activa = 1;
                        pas_claxon = 0;
                    }
                    break;

                default:
                    // Ignoram orice alt caracter invalid
                    break;
            }
        }
    }
    return 0;
}

// --- Algoritmi Parcare (Aici vei scrie logica pe baza senzorilor) ---

void Executa_Parcare_Fata(void) {
    /* Exemplu simplu: 
     * uint16_t distanta = ULTRASONIC_GetDistance();
     * if (distanta > 20) MOTOR_Drive(DIR_FATA, 40);
     * else { MOTOR_Drive(DIR_STOP, 0); mod_parcare_activ = '0'; }
     */
}

void Executa_Parcare_Spate(void) {
    // Va necesita citirea noului senzor ultrasonic de pe spate
}

void Executa_Parcare_Lateral_Stanga(void) {
    // Manevre complexe in mai multi pasi
}

void Executa_Parcare_Lateral_Dreapta(void) {
    // Manevre complexe in mai multi pasi
}