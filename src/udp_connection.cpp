#include "header.h"

uint16_t board_port = DEFAULT_BOARD_PORT;
WiFiUDP SERVER;
uint8_t test_array[] = {'h', 'e', 'l', 'l', 'o'};
uint8_t udp_rx_buffer[UDP_TX_SIZE + 1];

// отправка строки по udp 
void send_string(String out) {
    uint8_t * array = new uint8_t[out.length()] {};
    for (int i = 0; i < out.length(); i++) array[i] = out[i];
    if (server_send(array, out.length(), ground, ground_port))
    {
        Serial.println("udp message sended");
        blink_async(100, 1, false);
    }
    delete [] array;
}
// отправка массива данных по udp
int server_send(uint8_t *out, int size, IPAddress ip, uint32_t port)
{
    SERVER.beginPacket(ip, port);
    SERVER.write(out, size);
    return SERVER.endPacket();
    //delete[] out;
}
// приём данных по udp
bool server_read(uint8_t* buffer, int8_t* size, uint32_t timeout) {
    *size = -1;
    Serial.printf("waiting for messages at local port %i\n", board_port);
    uint32_t start_time = millis();
    while (true) 
    {
        int packetSize = SERVER.parsePacket();
        if (packetSize) {
            // Serial.printf("Received packet of size %d from %s:%d\n    (to %s:%d, free heap = %d B)\n",
            //             packetSize,
            //             SERVER.remoteIP().toString().c_str(), SERVER.remotePort(),
            //             SERVER.destinationIP().toString().c_str(), SERVER.localPort(),
            //             ESP.getFreeHeap());
            *size = SERVER.read(buffer, UDP_TX_SIZE);
            buffer[UDP_TX_SIZE] = 0;
            Serial.printf("recieved %i bytes, message: ", *size);
            return true;
        }
        if (millis() - start_time > timeout) return false;
    }
}



