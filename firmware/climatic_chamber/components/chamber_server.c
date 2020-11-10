#include "chamber_server.h"


static const char *TAG = "http_server";



static esp_err_t actual_get_handler(httpd_req_t *req)
{
    char actual_temp[4];
    uint32_t actual = get_actual_temp();

    sprintf(actual_temp, "%u", actual);

    httpd_resp_send(req, (const char*) actual_temp, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

static esp_err_t target_get_handler(httpd_req_t *req)
{
    char target_temp[4];
    uint32_t target = get_target_temp();

    sprintf(target_temp, "%u", target);

    httpd_resp_send(req, (const char*) target_temp, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

static esp_err_t target_post_handler(httpd_req_t *req)
{
    char content[3];

    /* Truncate if content length larger than the buffer */
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);

    if (ret <= 0) {  
        /* 0 return value indicates connection closed */
        /* Check if timeout occurred */
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {

            httpd_resp_send_408(req);
        }

        return ESP_FAIL;
    }

    uint32_t temp = atoi(content);
    esp_err_t res = set_target_temp(temp);

    /* Log data received */
    ESP_LOGI(TAG, "Updating target temperature to %s", content);

    if (res != ESP_OK){
        const char resp[] = "Failed";
        httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    }
    else {
        const char resp[] = "Updated";
        httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    }

    return ESP_OK;
}

static const httpd_uri_t actual = {
    .uri       = "/actual",
    .method    = HTTP_GET,
    .handler   = actual_get_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t target_get = {
    .uri       = "/target",
    .method    = HTTP_GET,
    .handler   = target_get_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t target_post = {
    .uri       = "/target",
    .method    = HTTP_POST,
    .handler   = target_post_handler,
    .user_ctx  = NULL
};

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &actual);
        httpd_register_uri_handler(server, &target_get);
        httpd_register_uri_handler(server, &target_post);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

void http_server_start(void)
{
    static httpd_handle_t server = NULL;

    ESP_ERROR_CHECK(wifi_connect());
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));

    server = start_webserver();
}