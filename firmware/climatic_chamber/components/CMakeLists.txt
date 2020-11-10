idf_component_register(
    SRCS "chamber_server.c" "wifi_connection.c" "temperature.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_http_server 
)