#ifndef TEMPERATURE
#define TEMPERATURE

#include <stdlib.h>
#include <stdint.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

struct temp_data{
    uint32_t actual;
    uint32_t target;
    uint32_t room;
};

typedef struct temp_data temp_data_t;

void init_temp(void);

void read_actual_sensor(void);

uint32_t get_actual_temp(void);

uint32_t get_target_temp(void);

esp_err_t set_target_temp(uint32_t value);

uint32_t get_room_temp(void);

#endif