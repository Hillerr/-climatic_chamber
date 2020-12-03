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

static int calc_last_measures(int *last_measures, int size){
    int i, sum = 0;

    for (i = 0; i < size; i++){
        sum += last_measures[i];
    }

    return sum/size;
}

static void init_last_measures(int *last_measures, int size){
    int i;

    for (i = 0; i < size; i++){
        last_measures[i] = 0;
    }
}

static void ssr_control_task(void *pvParameters)
{
    double Kp = 2.65;
    double Ki = 0.0116;
    double Kd = 0;
    int error = 0, last_measure = 0, last_curr = 0;
    int samples[3], samples_index = 0, size_of_samples = 3;

    init_last_measures(samples, size_of_samples);

    double proportional, integral = 0, pi, derivative;
    int fs = 1;
    int duty;

    sensor_read_t curr, target;

    ssr_init();

    while(1){

        curr = get_actual_temp();
        target = get_target_temp();

        error = target.integer - curr.integer;
        last_measure = calc_last_measures(samples, size_of_samples);

        samples[samples_index] = error;

        samples_index ++;
        samples_index %= size_of_samples;

        last_curr = last_measure - calc_last_measures(samples, size_of_samples);

        proportional = Kp * error;
        integral += Ki * error;
        derivative = (last_curr)*Kd;

        pi = proportional + integral + derivative;

        duty = (pi * 64/160);

        if (duty > 64) duty = 64;
        if (duty < 0) duty = 0;

        duty = duty * 256 * 4;

        ESP_LOGI(TAG, "Current: %d.%d\tTarget: %d.%d\tError: %d\tControl: %f\tDuty: %d", 
                curr.integer, curr.decimal, 
                target.integer, target.decimal, 
                error, pi, duty);

        ESP_LOGI(TAG, "Last: %d\tLast_Curr: %d",  last_measure, last_curr);
        

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
