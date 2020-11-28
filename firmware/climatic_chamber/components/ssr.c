#include "ssr.h"

static ssr_timer_config_t ssr_timer;
static ssr_channel_config_t ssr_channel;

SemaphoreHandle_t ssr_semaphore = NULL;

esp_err_t ssr_init(void)
{
    vSemaphoreCreateBinary(ssr_semaphore);

    esp_err_t ret;

    ssr_timer.duty_resolution = LEDC_TIMER_16_BIT; 
    ssr_timer.freq_hz = 1;                      
    ssr_timer.speed_mode = SSR_MODE;           
    ssr_timer.timer_num = SSR_TIMER;            
    ssr_timer.clk_cfg = LEDC_AUTO_CLK;              

    ret = ledc_timer_config(&ssr_timer);

    if(ret != ESP_OK){
        return ret;
    }

    ssr_channel.channel    = SSR_CHANNEL;
    ssr_channel.duty       = 0;
    ssr_channel.gpio_num   = SSR_GPIO;
    ssr_channel.speed_mode = SSR_MODE;
    ssr_channel.hpoint     = 0;
    ssr_channel.timer_sel  = SSR_TIMER;

    ret = ledc_channel_config(&ssr_channel);

    return ret;
}

void ssr_set_duty(uint32_t duty)
{
    if (xSemaphoreTake(ssr_semaphore, portMAX_DELAY) == pdTRUE)
	{
		ledc_set_duty(SSR_MODE, ssr_channel.channel, duty);
        ledc_update_duty(SSR_MODE, ssr_channel.channel);

		xSemaphoreGive(ssr_semaphore);
	}
}