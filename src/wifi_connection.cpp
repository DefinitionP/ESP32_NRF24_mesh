#include "header.h"

const uint8_t network_info_count = 4;
String network_info[] = {
    "Node", "password",
    "Baza", "77145055"};

bool wifi_connect() {
    Serial.println("WiFi scan started...");
    int16_t network_count = WiFi.scanNetworks();
    if (network_count == 0)
    {
        Serial.println("no networsk found");
        return false;
    }
    int8_t *networks = new int8_t[network_count];
    for (int8_t i = 0; i < network_count; i++)
    {
        networks[i] = i;
    }
    // сортировка по возрастанию уровня сигнала
    for (int8_t i = 1; i < network_count; i++)
    {
        int8_t current = networks[i], j = i - 1;
        for (; j >= 0 && (long)(WiFi.RSSI(current) > WiFi.RSSI(networks[j])); j--)
        {
            networks[j + 1] = networks[j];
        }
        networks[j + 1] = current;
    }
    bool connected = false;
    for (uint8_t i = 0; connected == false && i < network_info_count; i = i + 2)
    {
        for (uint8_t j = 0; connected == false && j < network_count; j++)
        {
            if (WiFi.SSID(networks[j]) == network_info[i])
            {
                connected = wifi_try_connect(CONNECTION_THRESHOLD, network_info[i], network_info[i + 1]);
            }
        }
    }
    delete[] networks;
    if (connected)
    {
        Serial.printf("successfully connected to \"%s\", IP: %s\n", WiFi.SSID(), WiFi.localIP().toString());
        // Serial.println(get_internal_port());
        return true;
    }
    else
    {
        Serial.printf("couldn't connect to any network\n");
        return false;
    }
}

bool wifi_try_connect(uint16_t ms, String ssid, String psk)
{
    Serial.printf("trying to connect \"%s\", psk is \"%s\"", ssid, psk);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, psk);
    uint16_t connection_start = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(150);
        Serial.print('.');
        if (millis() - connection_start > ms)
        {
            Serial.println("\ncouldn't connect");
            return false;
        }
    }
    Serial.println("\nconnected!");
    return true;
}

void wifi_connection_check() {
    if (WiFi.status() != WL_CONNECTED) {
        //Serial.println("internet connection is lost");
        blink_async(300, 2, true);
        while(true) {};
        //while (!wifi_connect()) {};
        poweron_msg("connection is resumed!");
    }
}