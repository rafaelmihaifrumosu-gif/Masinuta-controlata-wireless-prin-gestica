/*
 * Proiect: Rover Controlat prin Gesturi (Modul ESP32-C6 Master)
 * Autor principal: Rafael Mihai
 * Descriere: Preia comenzi prin BLE GATT, controleaza farurile/semnalizarile
 * local non-blocant si trimite restul comenzilor catre Arduino Uno prin I2C.
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/i2c.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_defs.h"
#include "esp_gatt_common_api.h"

static const char *TAG = "ROVER_BLE";

// --- Configuratie I2C ---
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_SDA_IO 6  // Pin sigur (dreapta)
#define I2C_MASTER_SCL_IO 7  // Pin sigur (dreapta)
#define I2C_MASTER_FREQ_HZ 100000
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0
#define I2C_SLAVE_ADDR 0x08

// --- Configuratie Actuatori Caroserie ---
#define PIN_FARURI 18        // Pin sigur (stanga)
#define PIN_SEMNAL_STG 19    // Pin sigur (stanga)
#define PIN_SEMNAL_DR 20     // Pin sigur (stanga)

#define PWM_CHANNEL 0
#define PWM_FREQ 5000
#define PWM_RESOLUTION LEDC_TIMER_8_BIT

// --- Configuratie BLE (GATT Server) ---
#define GATT_SERVICE_UUID 0xFF00
#define GATT_CHAR_UUID 0xFF01
#define GATT_MAX_VALUE_LEN 20

static int stare_faruri = 0;
static char stare_semnalizare = 'O';
static bool led_semnal_aprins = false;
static TickType_t last_blink_time;
static const TickType_t blink_interval = pdMS_TO_TICKS(500);

static uint16_t g_service_handle = 0;
static uint16_t g_char_handle = 0;
static esp_gatt_if_t g_gatts_if = ESP_GATT_IF_NONE;
static uint16_t g_conn_id = 0;
static bool g_connected = false;

static void set_lights_off(void)
{
    gpio_set_level(PIN_SEMNAL_STG, 0);
    gpio_set_level(PIN_SEMNAL_DR, 0);
}

static void i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode,
                                      I2C_MASTER_RX_BUF_DISABLE,
                                      I2C_MASTER_TX_BUF_DISABLE, 0));
}

static esp_err_t send_i2c_command(uint8_t command)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t err = i2c_master_start(cmd);
    if (err != ESP_OK) goto cleanup;
    err = i2c_master_write_byte(cmd, (I2C_SLAVE_ADDR << 1) | I2C_MASTER_WRITE, true);
    if (err != ESP_OK) goto cleanup;
    err = i2c_master_write_byte(cmd, command, true);
    if (err != ESP_OK) goto cleanup;
    err = i2c_master_stop(cmd);
    if (err != ESP_OK) goto cleanup;
    err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
cleanup:
    i2c_cmd_link_delete(cmd);
    return err;
}

static void process_command(char cmd)
{
    ESP_LOGI(TAG, "Comanda primita: %c", cmd);

    if (cmd == 'H') {
        stare_faruri++;
        if (stare_faruri > 2) {
            stare_faruri = 0;
        }

        if (stare_faruri == 0) {
            ledc_set_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL, 0);
        } else if (stare_faruri == 1) {
            ledc_set_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL, 64);
        } else {
            ledc_set_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL, 255);
        }
        ledc_update_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL);
    } else if (cmd == 'L' || cmd == 'R' || cmd == 'Z') {
        if (stare_semnalizare == cmd) {
            stare_semnalizare = 'O';
            set_lights_off();
        } else {
            stare_semnalizare = cmd;
        }
        led_semnal_aprins = false;
        set_lights_off();
        last_blink_time = xTaskGetTickCount();
    } else if (cmd == 'S') {
        stare_semnalizare = 'O';
        set_lights_off();
        esp_err_t err = send_i2c_command((uint8_t)cmd);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Eroare trimitere I2C: %s", esp_err_to_name(err));
        }
    } else {
        esp_err_t err = send_i2c_command((uint8_t)cmd);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Eroare trimitere I2C: %s", esp_err_to_name(err));
        }
    }
}

static void update_signaling(void)
{
    if (stare_semnalizare == 'O') {
        set_lights_off();
        return;
    }

    if (xTaskGetTickCount() - last_blink_time >= blink_interval) {
        last_blink_time = xTaskGetTickCount();
        led_semnal_aprins = !led_semnal_aprins;

        if (stare_semnalizare == 'Z') {
            gpio_set_level(PIN_SEMNAL_STG, led_semnal_aprins);
            gpio_set_level(PIN_SEMNAL_DR, led_semnal_aprins);
        } else if (stare_semnalizare == 'L') {
            gpio_set_level(PIN_SEMNAL_STG, led_semnal_aprins);
            gpio_set_level(PIN_SEMNAL_DR, 0);
        } else if (stare_semnalizare == 'R') {
            gpio_set_level(PIN_SEMNAL_STG, 0);
            gpio_set_level(PIN_SEMNAL_DR, led_semnal_aprins);
        }
    }
}

static void start_advertising(void)
{
    esp_ble_adv_data_t adv_data = {
        .set_scan_rsp = false,
        .include_name = true,
        .include_txpower = false,
        .min_interval = 0x20,
        .max_interval = 0x40,
        .appearance = 0x0,
        .manufacturer_len = 0,
        .p_manufacturer_data = NULL,
        .service_data_len = 0,
        .p_service_data = NULL,
        .service_uuid_len = 0,
        .p_service_uuid = NULL,
        .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
    };

    esp_ble_adv_params_t adv_params = {
        .adv_int_min = 0x20,
        .adv_int_max = 0x40,
        .adv_type = ADV_TYPE_IND,
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .channel_map = ADV_CHNL_ALL,
        .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    };

    esp_ble_gap_config_adv_data(&adv_data);
    esp_ble_gap_start_advertising(&adv_params);
}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        break;
    default:
        break;
    }
}

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event) {
    case ESP_GATTS_REG_EVT:
        if (param->reg.status == ESP_GATT_OK) {
            g_gatts_if = gatts_if;
            ESP_LOGI(TAG, "GATT app registered");

            esp_gatt_srvc_id_t service_id = {
                .id = {
                    .uuid = {
                        .len = ESP_UUID_LEN_16,
                        .uuid.uuid16 = GATT_SERVICE_UUID,
                    },
                    .inst_id = 0,
                },
                .is_primary = true,
            };
            esp_bt_uuid_t char_uuid = {
                .len = ESP_UUID_LEN_16,
                .uuid.uuid16 = GATT_CHAR_UUID,
            };
            esp_attr_value_t char_val = {
                .attr_max_len = GATT_MAX_VALUE_LEN,
                .attr_len = 0,
                .attr_value = NULL,
            };
            esp_attr_control_t control = {
                .auto_rsp = ESP_GATT_AUTO_RSP,
            };

            esp_err_t err = esp_ble_gatts_create_service(gatts_if, &service_id, 4);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Create service failed: %s", esp_err_to_name(err));
                break;
            }

            err = esp_ble_gatts_add_char(g_service_handle, &char_uuid,
                                         ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                         ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
                                         &char_val, &control);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Add characteristic failed: %s", esp_err_to_name(err));
            }
        }
        break;

    case ESP_GATTS_CREATE_EVT:
        if (param->create.status == ESP_GATT_OK) {
            g_service_handle = param->create.service_handle;
            ESP_LOGI(TAG, "Service created");
        }
        break;

    case ESP_GATTS_ADD_CHAR_EVT:
        if (param->add_char.status == ESP_GATT_OK) {
            g_char_handle = param->add_char.attr_handle;
            ESP_LOGI(TAG, "Characteristic added");
            esp_ble_gatts_start_service(g_service_handle);
        }
        break;

    case ESP_GATTS_START_EVT:
        if (param->start.status == ESP_GATT_OK) {
            ESP_LOGI(TAG, "GATT service started");
            esp_ble_gap_set_device_name("Rover_ESP32C6");
            start_advertising();
        }
        break;

    case ESP_GATTS_WRITE_EVT:
        if (param->write.handle == g_char_handle && param->write.len > 0) {
            for (int i = 0; i < param->write.len; i++) {
                process_command((char)param->write.value[i]);
            }
        }
        esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id,
                                     ESP_GATT_OK, NULL);
        break;

    case ESP_GATTS_CONNECT_EVT:
        g_connected = true;
        g_conn_id = param->connect.conn_id;
        ESP_LOGI(TAG, "Client connected");
        break;

    case ESP_GATTS_DISCONNECT_EVT:
        g_connected = false;
        ESP_LOGI(TAG, "Client disconnected");
        start_advertising();
        break;

    default:
        break;
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    ESP_ERROR_CHECK(esp_ble_gap_register_callback(gap_event_handler));
    ESP_ERROR_CHECK(esp_ble_gatts_register_callback(gatts_event_handler));
    ESP_ERROR_CHECK(esp_ble_gatts_app_register(0));

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PIN_SEMNAL_STG) | (1ULL << PIN_SEMNAL_DR),
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    set_lights_off();

    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = PWM_RESOLUTION,
        .freq_hz = PWM_FREQ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_conf));

    ledc_channel_config_t channel_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = PIN_FARURI,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel_conf));

    i2c_master_init();
    last_blink_time = xTaskGetTickCount();

    ESP_LOGI(TAG, "Rover BLE GATT started. Waiting for commands.");

    while (1) {
        update_signaling();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
