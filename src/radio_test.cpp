#include "header.h"

#define CE_PIN 27
#define CS_PIN 15

RF24 radio(CE_PIN, CS_PIN); 
SPIClass mega_spi(2);
SPIClass *pointer_spi = &mega_spi;

const uint8_t num_channels = 128;
uint8_t values[num_channels];
uint8_t out_buffer[num_channels];

// число сканов
const int num_reps = 100;

const uint32_t pipe_addr = 111156789; // адрес рабочей трубы;
byte tx_data;

void radio_scan()
{
    if (!radio.begin())
        Serial.println("radio init error!");
    else
        Serial.println("radio init ok!");

    radio.setAutoAck(false);

    // Get into standby mode
    radio.startListening();
    radio.printDetails();

    int i = 0;
    while (i < num_channels)
    {
        Serial.printf("%x", i >> 4);
        ++i;
    }
    Serial.printf("\n\r");
    i = 0;
    while (i < num_channels)
    {
        Serial.printf("%x", i & 0xf);
        ++i;
    }
    Serial.printf("\n\r");
    while (true)
    {
        // Clear measurement values
        memset(values, 0, sizeof(values));
        // Scan all channels num_reps times
        int rep_counter = num_reps;
        while (rep_counter--)
        {
            int i = num_channels;
            while (i--)
            {
                // Select this channel
                radio.setChannel(i);

                // Listen for a little
                radio.startListening();
                delayMicroseconds(512);
                radio.stopListening();

                // Did we get a carrier?
                if (radio.testCarrier())
                    ++values[i];
            }
        }

        // Print out channel measurements, clamped to a single hex digit
        int i = 0;

        String out = "";
        while (i < num_channels)
        {
            // Serial.printf("%x", min(0xf, values[i] & 0xf));
            out += ("%x", min(0xf, values[i] & 0xf));
            ++i;
        }
        // Serial.printf("\n\r");
        Serial.println(out);
        send_string(out);
    }
}


void radio_tx_test()
{
    if (!radio.begin())
        Serial.println("radio init error!");
    else
        Serial.println("radio init ok!");

    radio.setDataRate(RF24_1MBPS);    // скорость обмена данными RF24_1MBPS или RF24_2MBPS
    radio.setCRCLength(RF24_CRC_8);   // размер контрольной суммы 8 bit или 16 bit
    radio.setPALevel(RF24_PA_MAX);    // уровень питания усилителя RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX
    radio.setChannel(0x6f);           // установка канала
    radio.setAutoAck(false);          // автоответ
    radio.powerUp();                  // включение или пониженное потребление powerDown - powerUp
    radio.stopListening();            // радиоэфир не слушаем, только передача
    radio.openWritingPipe(pipe_addr); // открыть трубу на отправку

    while (true)
    {
        tx_data = 109;
        radio.write(&tx_data, 1);
        Serial.println("data= " + String(tx_data));
    }
}

byte rx_data[1];
int scn = 0; // счетчик циклов прослушивания эфира
int sg = 0;  // счетчик числа принятых пакетов с передатчика
void radio_rx_test()
{
    if (!radio.begin())
        Serial.println("radio init error!");
    else
        Serial.println("radio init ok!");

    radio.setDataRate(RF24_1MBPS);       // скорость обмена данными RF24_1MBPS или RF24_2MBPS
    radio.setCRCLength(RF24_CRC_8);      // размер контрольной суммы 8 bit или 16 bit
    radio.setChannel(0x6f);              // установка канала
    radio.setAutoAck(false);             // автоответ
    radio.openReadingPipe(1, pipe_addr); // открыть трубу на приём
    radio.startListening();              // приём

    while (true)
    {
        if (scn < 1000)
        { // прослушивание эфира
            if (radio.available())
            {
                radio.read(rx_data, 1);
                //scn++;
                if (rx_data[0] == 109)
                {
                    sg++;
                }
            }
        }
        else
        { 
            // всего принято
            {
                String out = "recieved packets: ";
                out += sg;
                Serial.println(out);
                send_string(out);
                sg = 0;
            }
            scn = 0;
        }
        scn++;
        delay(2);

        if (scn >= 1000) scn = 1000;
    }
}

