#include "wifi_connection.h"

#define WIFI_SSID "Apto 305 d"
#define WIFI_PASSWORD "Henrique1951"

static const char *TAG = "wifi_connection";
static esp_netif_t *s_example_esp_netif = NULL;


static esp_netif_t* wifi_start(void)
{
    esp_netif_ip_info_t my_ip;
    esp_ip4_addr_t ip4addr, mask, gateway;
    esp_netif_t *my_sta = esp_netif_create_default_wifi_sta();

    esp_netif_dhcpc_stop(my_sta);

    ip4addr.addr = ESP_IP4TOUINT32(45, 15, 168, 192);
    mask.addr = ESP_IP4TOUINT32(0,255,255,255);
    gateway.addr = ESP_IP4TOUINT32(254, 1, 168, 192);

    my_ip.ip = ip4addr;
    my_ip.netmask = mask;
    my_ip.gw = gateway;

    esp_netif_set_ip_info(my_sta, &my_ip);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_wifi_set_default_wifi_sta_handlers();


    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
        },
    };
    ESP_LOGI(TAG, "Connecting to %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());
    return my_sta;
}


static void wifi_stop(void)
{
    esp_netif_t *wifi_netif = get_example_netif_from_desc("sta");

    esp_err_t err = esp_wifi_stop();
    if (err == ESP_ERR_WIFI_NOT_INIT) {
        return;
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(esp_wifi_deinit());
    ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(wifi_netif));
    esp_netif_destroy(wifi_netif);
}

esp_err_t wifi_connect(void)
{
    s_example_esp_netif = wifi_start();
    ESP_ERROR_CHECK(esp_register_shutdown_handler(&wifi_stop));
    ESP_LOGI(TAG, "Waiting for IP(s)");

    return ESP_OK;
}

esp_netif_t *get_example_netif_from_desc(const char *desc)
{
    esp_netif_t *netif = NULL;
    char *expected_desc;
    asprintf(&expected_desc, "%s: %s", TAG, desc);
    while ((netif = esp_netif_next(netif)) != NULL) {
        if (strcmp(esp_netif_get_desc(netif), expected_desc) == 0) {
            free(expected_desc);
            return netif;
        }
    }
    free(expected_desc);
    return netif;
}