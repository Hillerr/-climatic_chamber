#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/ledc.h"
#include "esp_err.h"

#define SSR_TIMER          LEDC_TIMER_1
#define SSR_MODE           LEDC_LOW_SPEED_MODE
#define SSR_GPIO       2
#define SSR_CHANNEL    LEDC_CHANNEL_0

typedef ledc_timer_config_t ssr_timer_config_t;

typedef ledc_channel_config_t ssr_channel_config_t;

esp_err_t ssr_init(void);

void ssr_set_duty(uint32_t duty);



