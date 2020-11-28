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

static void temp_read_task(void *pvParameters) 
{
    while(1){
        read_actual_sensor();
        read_room_sensor();
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

static void ssr_control_task(void *pvParameters)
{
    ssr_init();

    while(1){
        ssr_set_duty(23831);
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
