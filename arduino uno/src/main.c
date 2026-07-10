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
                            SERVO_SetAngle(SERVO_CH_A, 90);  // Se inchide la loc (mijloc)
                            usa_stanga_deschisa = 0; 
                        } else { 
                            SERVO_SetAngle(SERVO_CH_A, 0); // Se deschide spre exterior
                            usa_stanga_deschisa = 1; 
                        }
                        break;
                        
                    case 'I': // Usa Dreapta (Servo B)
                        if (usa_dreapta_deschisa) { 
                            SERVO_SetAngle(SERVO_CH_B, 90);  // Se inchide la loc (mijloc)
                            usa_dreapta_deschisa = 0; 
                        } else { 
                            SERVO_SetAngle(SERVO_CH_B, 180);   // Se deschide spre exterior (invers)
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

void Executa_Parcare_Fata(void) {
    static uint32_t last_scan = 0;
    uint32_t timp_curent = Millis();
    if (timp_curent - last_scan < 60) return; 
    last_scan = timp_curent;

    uint16_t distanta_fata = ULTRASONIC_GetDistance(0); 
    if (distanta_fata == 999 || distanta_fata > 15) {
        MOTOR_Drive(DIR_FATA, 120); 
    } 
    else if (distanta_fata > 0 && distanta_fata <= 15) {
        MOTOR_Drive(DIR_STOP, 0); 
        mod_parcare_activ = '0'; 
    }
}

void Executa_Parcare_Spate(void) {
    static uint8_t pas = 0; 
    static uint32_t timp_start = 0;
    static uint32_t last_scan = 0;
    uint32_t timp_curent = Millis();
    
    if (timp_curent - last_scan < 60) return;
    last_scan = timp_curent;

    uint16_t d_spate = ULTRASONIC_GetDistance(1);
    uint16_t d_dreapta = ULTRASONIC_GetDistance(2);

    switch (pas) {
        case 0: 
            MOTOR_Drive(DIR_FATA, 120); 
            if (d_dreapta > 35 && d_dreapta != 999) { pas = 1; timp_start = Millis(); }
            break;
        case 1: 
            MOTOR_Drive(DIR_FATA, 120);
            if (Millis() - timp_start > 800) { MOTOR_Drive(DIR_STOP, 0); pas = 2; timp_start = Millis(); }
            break;
        case 2: 
            MOTOR_Drive(DIR_DREAPTA, 180); 
            if (Millis() - timp_start > 650) { MOTOR_Drive(DIR_STOP, 0); pas = 3; }
            break;
        case 3: 
            if (d_spate == 999 || d_spate > 12) { MOTOR_Drive(DIR_SPATE, 120); } 
            else if (d_spate > 0 && d_spate <= 12) { MOTOR_Drive(DIR_STOP, 0); pas = 0; mod_parcare_activ = '0'; }
            break;
    }
}

void Executa_Parcare_Lateral_Stanga(void) {}

void Executa_Parcare_Lateral_Dreapta(void) {
    static uint8_t pas = 0; 
    static uint32_t timp_start = 0;
    uint16_t d_fata = ULTRASONIC_GetDistance(0);
    uint16_t d_dreapta = ULTRASONIC_GetDistance(2);

    switch (pas) {
        case 0: MOTOR_Drive(DIR_FATA, 120); if (d_dreapta > 35 && d_dreapta != 999) { pas = 1; timp_start = Millis(); } break;
        case 1: MOTOR_Drive(DIR_FATA, 120); if (Millis() - timp_start > 400) { MOTOR_Drive(DIR_STOP, 0); pas = 2; timp_start = Millis(); } break;
        case 2: MOTOR_Drive(DIR_DREAPTA, 180); if (Millis() - timp_start > 650) { MOTOR_Drive(DIR_STOP, 0); pas = 3; } break;
        case 3: if (d_fata > 12) { MOTOR_Drive(DIR_FATA, 120); } else { MOTOR_Drive(DIR_STOP, 0); pas = 4; timp_start = Millis(); } break;
        case 4: MOTOR_Drive(DIR_STANGA, 180); if (Millis() - timp_start > 650) { MOTOR_Drive(DIR_STOP, 0); pas = 0; mod_parcare_activ = '0'; } break;
    }
}