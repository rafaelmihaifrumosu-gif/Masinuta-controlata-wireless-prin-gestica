#include "alarm.h"
#include "bsp/uno.h"
#include "drivers/timer/timer0.h"
#include "drivers/ultrasonic/ultrasonic.h"
#include "drivers/gpio/gpio.h"

// Definim starile posibile ale alarmei
typedef enum {
    ALARM_OFF = 0,
    ALARM_ARMED,
    ALARM_TRIGGERED
} AlarmState_t;

static AlarmState_t stare_alarma = ALARM_OFF;
static uint32_t last_scan_time = 0;
static uint32_t last_toggle_time = 0;
static uint8_t buzzer_state = 0;

void ALARM_Init(void) {
    stare_alarma = ALARM_OFF;
    buzzer_state = 0;
}

void ALARM_Arm(void) {
    stare_alarma = ALARM_ARMED;
    last_scan_time = Millis();
}

void ALARM_Disarm(void) {
    stare_alarma = ALARM_OFF;
    buzzer_state = 0;
    // Fortam oprirea buzzer-ului activ pe pinul D3
    GPIO_Write(UNO_D3, GPIO_LOW); 
}

uint8_t ALARM_IsTriggered(void) {
    return (stare_alarma == ALARM_TRIGGERED) ? 1 : 0;
}

void ALARM_Update(void) {
    if (stare_alarma == ALARM_OFF) return;

    uint32_t timp_curent = Millis();

    // STAREA 1: MOD DE VEGHE
    if (stare_alarma == ALARM_ARMED) {
        // Scanam senzorii de 10 ori pe secunda (100ms)
        if (timp_curent - last_scan_time >= 100) {
            last_scan_time = timp_curent;

            uint16_t d_fata = ULTRASONIC_GetDistance(0);
            uint16_t d_spate = ULTRASONIC_GetDistance(1);
            uint16_t d_dreapta = ULTRASONIC_GetDistance(2);

            // Intruziune la sub 5 cm
            if ((d_fata > 0 && d_fata <= 5) || 
                (d_spate > 0 && d_spate <= 5) || 
                (d_dreapta > 0 && d_dreapta <= 5)) {
                
                stare_alarma = ALARM_TRIGGERED;
                last_toggle_time = timp_curent;
            }
        }
    }

    // STAREA 2: ALARMA DECLANSATA
    if (stare_alarma == ALARM_TRIGGERED) {
        // Clipim buzzerul activ la intervale de 150ms
        if (timp_curent - last_toggle_time >= 150) {
            last_toggle_time = timp_curent;
            buzzer_state = !buzzer_state;
            
            if (buzzer_state) {
                GPIO_Write(UNO_D3, GPIO_HIGH);
            } else {
                GPIO_Write(UNO_D3, GPIO_LOW);
            }
        }
    }
}