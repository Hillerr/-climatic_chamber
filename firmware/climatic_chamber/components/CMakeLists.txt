idf_component_register(
    SRCS "ssr.c" "chamber_server.c" "wifi_connection.c" "temperature.c" "ssr.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_http_server 
)