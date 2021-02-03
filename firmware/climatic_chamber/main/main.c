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
    double Kp = 1.267;
    double Ki = 0.0038;
    double error = 0, curr_decimal, target_decimal;

    double proportional, integral = 0, pi, derivative = 0;
    int fs = 1;
    int duty;

    sensor_read_t curr, target, last_target;

    last_target.integer = 0;

    ssr_init();

    while(1){

        curr = get_actual_temp();
        target = get_target_temp();

        if(target.integer){

            target_decimal = ((double) target.decimal)/100;
            curr_decimal = ((double) curr.decimal)/100;

            if (target.integer < last_target.integer){
                if (curr.integer < target.integer)
                    integral = 0;

                last_target = target;
            }

            error = (double) target.integer - (double) curr.integer;
            error = error + (target_decimal - curr_decimal);

            proportional = Kp * error;
            integral += Ki * error;

        }
        else{
            proportional = 0;
            integral = 0;
        }

        last_target = target;

        pi = proportional + integral + derivative;

        duty = (pi * 16/62);

        if (duty > 64) duty = 64;
        if (duty < 0) duty = 0;

        duty = duty * 256 * 4;
        
        //duty = 6554;

        ESP_LOGI(TAG, "Current: %d.%d\tTarget: %d.%d\tError: %.2f\tControl: %.2f\tK: %.2f\tI: %.2f\tD: %.2f\tDuty: %d", 
                curr.integer, curr.decimal, 
                target.integer, target.decimal, 
                error, pi, proportional, integral, derivative, duty);


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
