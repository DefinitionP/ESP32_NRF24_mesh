#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <nRF24L01.h>
#include "RF24.h"

#include "config.h"
#include "router.h"

#define CONNECTION_THRESHOLD 3000
#define AP_SSID "node"
#define AP_PSK "password"
#define UDP_TX_SIZE 32
#define DEFAULT_BOARD_PORT 4444
#define PING_TIMEOUT 1000

// тип сообщения
typedef enum : uint8_t {
    MESH_DATA = 0, MESH_PING_REQUEST = 1, MESH_PING_RESPONCE = 2
} mesh_message_type;


// mesh_node.cpp основные функции, режимы работы узлов
void mesh_setup();
void mesh_router_test();
void mesh_sender();
void mesh_send_packet(uint8_t receiver, mesh_message_type type);
void mesh_ping();
String packet_info(uint16_t number, uint8_t senderID, uint8_t intermediateID, uint8_t targetID, uint8_t type);
bool wait_responce(uint8_t t_node, uint8_t t_type, uint32_t timeout);

// radio_test.cpp проверки работоспособности радиомодулей
void radio_scan();
void radio_tx_test();
void radio_rx_test();

// tools.cpp вспомогательные функции
void halt(const char * msg);
void blink_async(uint32_t duration, uint8_t count, bool continious);
void poweron_msg(String msg);
void blink_stop();

// udp_connection функции для работы с udp
int server_send(uint8_t *out, int size, IPAddress ip, uint32_t port);
bool server_read(uint8_t* buffer, int8_t* size, uint32_t timeout);
void send_string(String out);

// wifi_connection функции для подключения к wifi
bool wifi_connect();
bool wifi_try_connect(uint16_t ms, String ssid, String psk);
void wifi_connection_check();


extern uint32_t ground_port;
extern IPAddress ground;
extern uint8_t myID;