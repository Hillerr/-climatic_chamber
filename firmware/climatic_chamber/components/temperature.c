#include "temperature.h"

#define TEMP_SENSOR_HOST  HSPI_HOST
#define DMA_CHAN     2
#define PIN_NUM_MISO 18
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  19
#define ACTUAL_TEMP_PIN_CS   15
#define ROOM_TEMP_PIN_CS  4
#define GPIO_OUTPUT_PIN_MASK (1<<ACTUAL_TEMP_PIN_CS) | (1<<ROOM_TEMP_PIN_CS)
#define HIGH         1
#define LOW          0

static const char *TAG = "temperature";

SemaphoreHandle_t temp_semaphore = NULL;

static spi_device_handle_t spi_handle;
static spi_bus_config_t bus_config;
static spi_device_interface_config_t dev_config;
static spi_transaction_t trans_word;

static temp_data_t temperatures = {
    .actual = {
        .integer = 0,
        .decimal = 0,
        .raw_value =0
    },
    .target = {
        .integer = 80,
        .decimal = 0,
        .raw_value =0
    },
    .room = {
        .integer = 0,
        .decimal = 0,
        .raw_value =0
    },
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

    gpio_set_level(ROOM_TEMP_PIN_CS, HIGH);
    gpio_set_level(ACTUAL_TEMP_PIN_CS, HIGH);
    
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

void read_sensor(uint8_t cs_pin, sensor_read_t *sensor_read)
{
    uint16_t data, rawtemp, temp = 0;

    gpio_set_level(cs_pin, LOW); // MAX6675_CS prepare

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

	ESP_ERROR_CHECK(spi_device_transmit(spi_handle, &trans_word));

	gpio_set_level(cs_pin, HIGH); // MAX6675_CS prepare

    temp = ((((rawtemp & 0x00FF) << 8) | ((rawtemp & 0xFF00) >> 8))>>3)*25;

    sensor_read->integer = temp/100;
    sensor_read->decimal = temp%100;
    sensor_read->raw_value = rawtemp;
}

void read_actual_sensor(void)
{
    sensor_read_t temp;

    read_sensor(ACTUAL_TEMP_PIN_CS, &temp);
    set_actual_temp(temp);

	ESP_LOGI(TAG, "Actual temperature spiReadWord=%x temp=%d.%d",temp.raw_value, temp.integer, temp.decimal);

}

void read_room_sensor(void)
{
    sensor_read_t temp;

    read_sensor(ROOM_TEMP_PIN_CS, &temp);
    set_room_temp(temp);

	ESP_LOGI(TAG, "Room temperature spiReadWord=%x temp=%d.%d",temp.raw_value, temp.integer, temp.decimal);
}

sensor_read_t get_actual_temp(void)
{
    sensor_read_t temp;

    if (xSemaphoreTake(temp_semaphore, portMAX_DELAY) == pdTRUE)
	{
		temp = temperatures.actual;
		xSemaphoreGive(temp_semaphore);
	}

    return temp;
}

sensor_read_t get_target_temp(void)
{
    sensor_read_t temp;

    if (xSemaphoreTake(temp_semaphore, portMAX_DELAY) == pdTRUE)
	{
		temp = temperatures.target;
		xSemaphoreGive(temp_semaphore);
	}

    return temp;
}

sensor_read_t get_room_temp(void)
{
    sensor_read_t temp;

    if (xSemaphoreTake(temp_semaphore, portMAX_DELAY) == pdTRUE)
	{
		temp = temperatures.room;
		xSemaphoreGive(temp_semaphore);
	}

    return temp;
}

esp_err_t set_room_temp(sensor_read_t value)
{
    if (xSemaphoreTake(temp_semaphore, portMAX_DELAY) == pdTRUE)
	{
		temperatures.room = value;
		xSemaphoreGive(temp_semaphore);
	}

    return ESP_OK;

}

esp_err_t set_actual_temp(sensor_read_t value)
{
    if (xSemaphoreTake(temp_semaphore, portMAX_DELAY) == pdTRUE)
	{
		temperatures.actual = value;
		xSemaphoreGive(temp_semaphore);
	}

    return ESP_OK;

}

esp_err_t set_target_temp(sensor_read_t value)
{
    if (value.integer > CONFIG_MAX_TEMP){
        ESP_LOGE(TAG, "Cannot set a higher temperature than the CONFIG_MAX_TEMP.");
        return ESP_ERR_INVALID_ARG;
    }

    if(value.integer < temperatures.room.integer){
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

