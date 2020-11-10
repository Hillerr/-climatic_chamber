#ifndef CHAMBER_SERVER
#define CHAMBER_SERVER

#include <string.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "wifi_connection.h"
#include "temperature.h"

#include <esp_http_server.h>

void http_server_start(void);

#endif