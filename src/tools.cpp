#include "header.h"

TaskHandle_t blink_task = NULL;
uint32_t blink_delay = 0;
bool led_state = true;
bool task_contitious = false;
bool default_led_state = HIGH;
uint8_t blink_count = 0;

// асинхронное управление светодиодом
void blink_async(uint32_t duration, uint8_t count, bool continious)
{
    pinMode(2, OUTPUT);
    blink_stop();
    task_contitious = continious;
    blink_delay = duration;
    blink_count = count * 2;
    xTaskCreate(blink, "blink", 1024, NULL, 1, &blink_task);
}
void blink(void *params)
{
    do
    {
        digitalWrite(2, (led_state = !led_state));
        vTaskDelay(blink_delay);
        if (blink_count != 0) blink_count --;
    } while (task_contitious || blink_count);
    digitalWrite(2, default_led_state);
    led_state = default_led_state;
    blink_task = NULL;
    vTaskDelete(NULL);
}
void blink_stop()
{
    if (blink_task != NULL)
    {
        //Serial.println("deleting task");
        Serial.flush();
        vTaskDelete(blink_task);
    }
    digitalWrite(2, default_led_state);
    led_state = default_led_state;
    blink_task = NULL;
}
// сообщение при подключении узла к сети
void poweron_msg(String msg) {
    String out = msg + " Node ";
    out += myID;
    out += " is ready. target desktop ep: ";
    out += ground.toString();
    out += ":";
    out += ground_port;
    send_string(out);

}
// остановка работы устройства
void halt(const char * msg) {
    Serial.println(msg);
    Serial.flush();
    esp_deep_sleep_start();
}
