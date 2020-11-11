#include "temperature.h"

#define TEMP_SENSOR_HOST  HSPI_HOST
#define DMA_CHAN     2
#define PIN_NUM_MISO 18
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  19
#define PIN_NUM_CS   15
#define GPIO_OUTPUT_PIN_MASK (1<<PIN_NUM_CS)
#define HIGH         1
#define LOW          0


static const char *TAG = "temperature";

SemaphoreHandle_t temp_semaphore = NULL;

static spi_device_handle_t spi_handle;
static spi_bus_config_t bus_config;
static spi_device_interface_config_t dev_config;
static spi_transaction_t trans_word;

static temp_data_t temperatures = {
    .actual = 20,
    .target = 50,
    .room = 24
};

static bool temp_sys_status = false;

static void init_gpio() 
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_PIN_INTR_DISABLE, //disable interrupt
        .mode = GPIO_MODE_OUTPUT, //set as output mode
        .pin_bit_mask = GPIO_OUTPUT_PIN_MASK, //bit mask of the pins
        .pull_down_en = GPIO_PULLDOWN_DISABLE, //disable pull-down mode
        .pull_up_en = GPIO_PULLUP_DISABLE   //disable pull-up mode
    };

    ESP_LOGI(TAG, "gpio_config");
    gpio_config(&io_conf); //configure GPIO with the given settings
}

void init_temp(void)
{
    vSemaphoreCreateBinary(temp_semaphore);

    init_gpio();

    gpio_set_level(PIN_NUM_CS, HIGH);
    
    ESP_LOGI(TAG, "Initializing bus SPI%d...", TEMP_SENSOR_HOST+1);

    bus_config.miso_io_num = PIN_NUM_MISO;
    bus_config.mosi_io_num = -1;
    bus_config.sclk_io_num = PIN_NUM_CLK;
    bus_config.quadwp_io_num = -1;
    bus_config.quadhd_io_num = -1;
    bus_config.max_transfer_sz = 16;

    //Initialize the SPI bus
    ESP_ERROR_CHECK(spi_bus_initialize(TEMP_SENSOR_HOST, &bus_config, DMA_CHAN));
    
    // Configure MAX6675 spi device
    dev_config.address_bits     = 0;
    dev_config.command_bits     = 0;
    dev_config.dummy_bits       = 0;
    dev_config.mode             = 0; // SPI_MODE0
    dev_config.duty_cycle_pos   = 0;
    dev_config.cs_ena_posttrans = 0;
    dev_config.cs_ena_pretrans  = 0;
    dev_config.clock_speed_hz   = 10000;  // 10 kHz
    dev_config.spics_io_num     = -1; // CS External
    dev_config.flags            = 0; // SPI_MSBFIRST
    dev_config.queue_size       = 100;
    dev_config.pre_cb           = NULL,
    dev_config.post_cb          = NULL;

    ESP_LOGI(TAG, "spi_bus_add_device");
	ESP_ERROR_CHECK(spi_bus_add_device(TEMP_SENSOR_HOST, &dev_config, &spi_handle));
    
    temp_sys_status = true;

}

void read_actual_sensor(void)
{
    uint16_t data, rawtemp, temp=0;

    gpio_set_level(PIN_NUM_CS, LOW); // MAX6675_CS prepare
    vTaskDelay(20 / portTICK_RATE_MS);  // see MAX6675 datasheet

    rawtemp = 0x000;
    data = 0x000;  // write dummy

    trans_word.addr      = 0;
    trans_word.cmd       = 0;
    trans_word.flags     = 0;
    trans_word.length    = 8 * 2; // Total data length, in bits NOT number of bytes.
    trans_word.rxlength  = 0; // (0 defaults this to the value of ``length``)
    trans_word.tx_buffer = &data;
    trans_word.rx_buffer = &rawtemp;

    ESP_LOGI(TAG, "spi_device_transmit");
	ESP_ERROR_CHECK(spi_device_transmit(spi_handle, &trans_word));

	gpio_set_level(PIN_NUM_CS, HIGH); // MAX6675_CS prepare

    temp = ((((rawtemp & 0x00FF) << 8) | ((rawtemp & 0xFF00) >> 8))>>3)*25;

	ESP_LOGI(TAG, "readMax6675 spiReadWord=%x temp=%d.%d",rawtemp,temp/100,temp%100);

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

