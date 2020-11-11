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

struct sensor_read{
    uint32_t integer;
    uint32_t decimal;
    uint32_t raw_value;
};

typedef struct sensor_read sensor_read_t;

struct temp_data{
    sensor_read_t actual;
    sensor_read_t target;
    sensor_read_t room;
};

typedef struct temp_data temp_data_t;

void init_temp(void);

void read_actual_sensor(void);

void read_room_sensor(void);

sensor_read_t get_actual_temp(void);

sensor_read_t get_target_temp(void);

esp_err_t set_target_temp(uint32_t value);

sensor_read_t get_room_temp(void);

#endif