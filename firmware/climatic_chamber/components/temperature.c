#include "temperature.h"

static const char *TAG = "temperature";

SemaphoreHandle_t temp_semaphore = NULL;

static temp_data_t temperatures = {
    .actual = 20,
    .target = 50,
    .room = 24
};

static bool temp_sys_status = false;

void init_temp(void)
{
    vSemaphoreCreateBinary(temp_semaphore);

    temp_sys_status = true;
}


uint32_t get_actual_temp(void)
{
    uint32_t temp = 0;

    if (xSemaphoreTake(temp_semaphore, portMAX_DELAY) == pdTRUE)
	{
		temp = temperatures.actual;
		xSemaphoreGive(temp_semaphore);
	}

    return temp;
}

uint32_t get_target_temp(void)
{
    uint32_t temp = 0;

    if (xSemaphoreTake(temp_semaphore, portMAX_DELAY) == pdTRUE)
	{
		temp = temperatures.target;
		xSemaphoreGive(temp_semaphore);
	}

    return temp;
}

uint32_t get_room_temp(void)
{
    uint32_t temp = 0;

    if (xSemaphoreTake(temp_semaphore, portMAX_DELAY) == pdTRUE)
	{
		temp = temperatures.room;
		xSemaphoreGive(temp_semaphore);
	}

    return temp;
}

static void set_room_temp(uint32_t value)
{
    if (xSemaphoreTake(temp_semaphore, portMAX_DELAY) == pdTRUE)
	{
		temperatures.room = value;
		xSemaphoreGive(temp_semaphore);
	}

}

static void set_actual_temp(uint32_t value)
{
    if (xSemaphoreTake(temp_semaphore, portMAX_DELAY) == pdTRUE)
	{
		temperatures.actual = value;
		xSemaphoreGive(temp_semaphore);
	}

}

esp_err_t set_target_temp(uint32_t value)
{
    if (value > CONFIG_MAX_TEMP){
        ESP_LOGE(TAG, "Cannot set a higher temperature than the CONFIG_MAX_TEMP.");
        return ESP_ERR_INVALID_ARG;
    }

    if(value < temperatures.room){
        ESP_LOGE(TAG, "Cannot set a lower temperature than the actual room temperature.");
        return ESP_ERR_INVALID_ARG;
    }

    if (xSemaphoreTake(temp_semaphore, portMAX_DELAY) == pdTRUE)
	{
		temperatures.target = value;
		xSemaphoreGive(temp_semaphore);
	}

    return ESP_OK;
}

