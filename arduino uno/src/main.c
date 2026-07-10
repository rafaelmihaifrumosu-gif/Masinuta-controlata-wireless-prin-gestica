#include "bsp/uno.h"
#include "drivers/timer/timer0.h"
#include "drivers/i2c/i2c_slave.h"
#include "drivers/motor/motor.h"
#include "drivers/buzzer/buzzer.h"
#include "drivers/servo/servo.h"
#include "drivers/ultrasonic/ultrasonic.h"
#include "drivers/alarm/alarm.h"

#define I2C_SLAVE_ADDR 0x08

static uint8_t usa_stanga_deschisa = 0;
static uint8_t usa_dreapta_deschisa = 0;
static uint8_t secventa_claxon_activa = 0;
static uint8_t pas_claxon = 0;
static uint32_t timp_claxon_anterior = 0;

char mod_parcare_activ = '0'; 
static uint8_t stare_sistem_alarma = 0;

void Gestionare_Secventa_Claxon(void) {
    if (!secventa_claxon_activa) return;
    uint32_t timp_curent = Millis();
    switch (pas_claxon) {
        case 0: BUZZER_Beep(200); timp_claxon_anterior = timp_curent; pas_claxon = 1; break;
        case 1: if (timp_curent - timp_claxon_anterior >= 400) { BUZZER_Beep(200); timp_claxon_anterior = timp_curent; pas_claxon = 2; } break;
        case 2: if (timp_curent - timp_claxon_anterior >= 400) { BUZZER_Beep(200); secventa_claxon_activa = 0; pas_claxon = 0; } break;
    }
}

void Executa_Parcare_Fata(void);
void Executa_Parcare_Spate(void);
void Executa_Parcare_Lateral_Stanga(void);
void Executa_Parcare_Lateral_Dreapta(void);

int main(void) {
    Timer0_Init();
    MOTOR_Init(); 
    BUZZER_Init(); 
    SERVO_Init();
    ULTRASONIC_Init();
    ALARM_Init(); 
    i2c_slave_init(I2C_SLAVE_ADDR);

    // Initializare Usi: Pozitia INCHIS (Aliniate la 90 de grade)
    SERVO_SetAngle(SERVO_CH_A, 90); // Usa Stanga
    SERVO_SetAngle(SERVO_CH_B, 90); // Usa Dreapta

    uint8_t comanda_primita = 0;

    while (1) {
        BUZZER_Update();
        ULTRASONIC_Update(); 
        Gestionare_Secventa_Claxon();
        ALARM_Update(); 

        if (mod_parcare_activ == '1') Executa_Parcare_Fata();
        else if (mod_parcare_activ == '2') Executa_Parcare_Spate();
        else if (mod_parcare_activ == '3') Executa_Parcare_Lateral_Stanga();
        else if (mod_parcare_activ == '4') Executa_Parcare_Lateral_Dreapta();

        if (i2c_slave_has_data(&comanda_primita)) {
            if (comanda_primita == 'S') {
                mod_parcare_activ = '0';
                stare_sistem_alarma = 0;
                MOTOR_Drive(DIR_STOP, 0);
                ALARM_Disarm();
            } 
            else if (comanda_primita >= '1' && comanda_primita <= '4') {
                mod_parcare_activ = comanda_primita;
                stare_sistem_alarma = 0;
                ALARM_Disarm(); 
            }
            else {
                mod_parcare_activ = '0';
                MOTOR_Drive(DIR_STOP, 0);

                switch (comanda_primita) {
                    case 'X':
                        if (stare_sistem_alarma == 0) {
                            stare_sistem_alarma = 1;
                            ALARM_Arm();
                            BUZZER_Beep(150); 
                        } else {
                            stare_sistem_alarma = 0;
                            ALARM_Disarm();
                            BUZZER_Beep(50); 
                        }
                        break;
                        
                    case 'U': // Usa Stanga (Servo A)
                        if (usa_stanga_deschisa) { 
                            SERVO_SetAngle(SERVO_CH_A, 90);  // Inchis
                            usa_stanga_deschisa = 0; 
                        } else { 
                            SERVO_SetAngle(SERVO_CH_A, 180); // Deschis spre exterior
                            usa_stanga_deschisa = 1; 
                        }
                        break;
                        
                    case 'I': // Usa Dreapta (Servo B)
                        if (usa_dreapta_deschisa) { 
                            SERVO_SetAngle(SERVO_CH_B, 90);  // Inchis
                            usa_dreapta_deschisa = 0; 
                        } else { 
                            SERVO_SetAngle(SERVO_CH_B, 0);   // Deschis spre exterior (invers)
                            usa_dreapta_deschisa = 1; 
                        }
                        break;
                        
                    case 'C': 
                        if (!secventa_claxon_activa) { secventa_claxon_activa = 1; pas_claxon = 0; }
                        break;
                }
            }
        }
    }
    return 0;
}

// --- PARCARE SIMPLA CU FATA ---
void Executa_Parcare_Fata(void) {
    static uint32_t last_scan = 0;
    if (Millis() - last_scan < 50) return; 
    last_scan = Millis();

    uint16_t distanta_fata = ULTRASONIC_GetDistance(0); 
    
    // Merge drept cat timp e drum liber sau distanta e > 10 cm
    if (distanta_fata == 999 || distanta_fata > 10) {
        MOTOR_Drive(DIR_FATA, 120); 
    } 
    // Se opreste fix cand atinge 10 cm (fara erori de 0)
    else if (distanta_fata > 0 && distanta_fata <= 10) {
        MOTOR_Drive(DIR_STOP, 0); 
        mod_parcare_activ = '0'; 
    }
}

// --- PARCARE SIMPLA CU SPATELE ---
// --- PARCARE CU SPATELE (ROTIRE 180 GRADE -> MARSARIER PANA LA ZID) ---
void Executa_Parcare_Spate(void) {
    static uint8_t pas = 0; 
    static uint32_t timp_start = 0;
    static uint32_t last_scan = 0;
    
    if (Millis() - last_scan < 50) return;
    last_scan = Millis();

    uint16_t d_spate = ULTRASONIC_GetDistance(1);

    switch (pas) {
        case 0: // PAS 0: Initializare cronometru pentru rotire
            timp_start = Millis();
            pas = 1;
            break;

        case 1: // PAS 1: INTOARCE 180 DE GRADE
            MOTOR_Drive(DIR_STANGA, 180); // Pivotare pe loc la stanga
            
            // ATENTIE: Daca 650ms inseamna 90 de grade, atunci 1300ms 
            // ar trebui sa insemne 180 de grade. Ajusteaza numarul 
            // asta (1300) in sus sau in jos daca se intoarce prea mult/putin!
            if (Millis() - timp_start > 1300) { 
                MOTOR_Drive(DIR_STOP, 0);
                pas = 2; // Trecem la mersul cu spatele
            }
            break;

        case 2: // PAS 2: DA CU SPATELE PANA LA 10 CM DE ZID
            // Cat timp drumul e liber sau mai mare de 10 cm
            if (d_spate > 10 || d_spate == 999) {
                MOTOR_Drive(DIR_SPATE, 120);
            } 
            // Am atins pragul de 10 cm fata de zidul din spate
            else if (d_spate > 0 && d_spate <= 10) {
                MOTOR_Drive(DIR_STOP, 0);
                
                // Am terminat manevra cu succes!
                pas = 0;                 
                mod_parcare_activ = '0'; 
            }
            break;
    }
}
void Executa_Parcare_Lateral_Stanga(void) {}

// --- PARCARE LATERALA (SECVENTA FRONTALA -> ROTIRE -> SPATE) ---
void Executa_Parcare_Lateral_Dreapta(void) {
    static uint8_t pas = 0; 
    static uint32_t timp_start = 0;
    static uint32_t last_scan = 0;
    
    if (Millis() - last_scan < 50) return;
    last_scan = Millis();

    uint16_t d_fata = ULTRASONIC_GetDistance(0);
    uint16_t d_spate = ULTRASONIC_GetDistance(1);

    switch (pas) {
        case 0: // PAS 1: Mergi in fata pana dai de zid (la 15cm)
            if (d_fata > 15 || d_fata == 999) {
                MOTOR_Drive(DIR_FATA, 120);
            } else if (d_fata > 0 && d_fata <= 15) {
                MOTOR_Drive(DIR_STOP, 0);
                pas = 1;
                timp_start = Millis();
            }
            break;

        case 1: // PAS 2: Intoarce 90 de grade
            MOTOR_Drive(DIR_STANGA, 180); // Modifica DIR_STANGA in DIR_DREAPTA daca vrei sa se roteasca invers
            
            // ATENTIE: Calibreaza 650-ul ca sa faca exact 90 de grade!
            if (Millis() - timp_start > 650) { 
                MOTOR_Drive(DIR_STOP, 0);
                pas = 2;
            }
            break;

        case 2: // PAS 3: Da cu spatele pana la 10 cm de zid
            if (d_spate > 10 || d_spate == 999) {
                MOTOR_Drive(DIR_SPATE, 120);
            } else if (d_spate > 0 && d_spate <= 10) {
                MOTOR_Drive(DIR_STOP, 0);
                
                // Finalizare manevra!
                pas = 0;                 
                mod_parcare_activ = '0'; 
            }
            break;
    }
}
