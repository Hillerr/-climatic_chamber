/* Simple HTTP Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "chamber_server.h"
#include "temperature.h"
#include "ssr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

const char *TAG = "main";

static void temp_read_task(void *pvParameters) 
{
    while(1){
        read_actual_sensor();
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

static void ssr_control_task(void *pvParameters)
{
    double Kp = 3.454;
    double Ki = 0.0187;
    double Kd = 106.51;
    int error = 0, last_measure = 0, last_curr = 0;

    double proportional, integral = 0, pi, derivative;
    int fs = 1;
    int duty;

    sensor_read_t curr, target;

    ssr_init();

    while(1){

        curr = get_actual_temp();
        target = get_target_temp();

        error = target.integer - curr.integer;
        last_curr = last_measure - curr.integer;

        proportional = Kp * error;
        integral += Ki * error;
        derivative = (last_curr)*Kd;

        last_measure = curr.integer;

        pi = proportional + integral + derivative;

        duty = (pi * 64/160);

        if (duty > 64) duty = 64;
        if (duty < 0) duty = 0;

        duty = duty * 256 * 4;

        ESP_LOGI(TAG, "Error: %d\tControl: %f\tDuty: %d", error, pi, duty);
        ssr_set_duty(duty);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    init_temp();
    
    http_server_start();

    xTaskCreatePinnedToCore(&temp_read_task, "temp_read_task", 8192, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(&ssr_control_task, "ssr_control_task", 8192, NULL, 5, NULL, 0);
}
