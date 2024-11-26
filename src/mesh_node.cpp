#include "header.h"

#ifdef NODE_1
uint32_t ground_port = 8888;
uint8_t myID = 1;
#endif
#ifdef NODE_2
uint32_t ground_port = 8889;
uint8_t myID = 2;
#endif
#ifdef NODE_3
uint32_t ground_port = 8890;
uint8_t myID = 3;
#endif
#ifdef NODE_4
uint32_t ground_port = 8891;
uint8_t myID = 4;
#endif
#ifdef NODE_5
uint32_t ground_port = 8892;
uint8_t myID = 5;
#endif

// ip-адрес устройства, на которое приходят сообщения по udp
IPAddress ground GROUND_IPADDR;
Router * router;
RF24_G * radio_;
// номер получателя сообщений по умолчанию
uint8_t default_receiver = DEFAULT_RECIEVER;
// передаваемое сообщение по умолчанию
const String mesh_data = "sample message";

void mesh_setup() {
    radio_ = new RF24_G(myID, 27, 15);
    RF24_G rad = *radio_;
    router = new Router(myID, rad);
}

// работа узла в режиме приёма и перенаправления пакетов
void mesh_router_test()
{
    mesh_setup();
    while (true)
    {
        // проверка соединения с wifi
        wifi_connection_check();
        if (radio_->available() == true)
        {
            // получение массива данных пакета
            packet receiver;
            uint8_t actual[30];
            radio_->read(&receiver);
            receiver.readPayload(actual, 30);
            // получение служебной информаци
            uint8_t senderID = actual[0] >> 4; // первоначальный отправитель пакета
            uint8_t intermediateID = actual[3]; // последний отправитель пакета
            uint8_t targetID = actual[0] & 0x0f; // получатель пакета
            uint16_t number = (actual[1] << 8) | actual[2]; // номер пакета
            uint8_t type = actual[5]; // тип сообщения
            uint8_t payload_len = actual[4]; // длина полезной нагрузки

            // если сообщение является новым
            if (router->checkIfNewMessage(senderID, number))
            {
                // получение строки информации о пакете для отправки по udp
                String info = packet_info(number, senderID, intermediateID, targetID, type);
                // если текущий узел является получаталем пакета
                if (router->checkIfIAmReceiver(actual[0]))
                {
                    // сообщение принято
                    info += " is recieved [";
                    info += payload_len;
                    info += "] #: ";
                    for (uint8_t i = 0; i < payload_len; i++) {
                        info += (char)(actual[i + 6]);
                    }

                    Serial.println(info); 
                    send_string(info);
                    blink_async(80, 3, false);
                    // если тип сообщения - запрос пинг, отправляем ответ
                    if (type == MESH_PING_REQUEST) {
                        mesh_send_packet(senderID, MESH_PING_RESPONCE);
                    }
                }
                else
                {
                    // если текущий узел не является получателем, пакет перенаправляется 
                    actual[3] = myID;
                    router->routeMessage(actual, info);
                    blink_async(100, 2, false);
                }
            }
        }
    }
}

// работа узла в режиме отправки сообщений по нажатию кнопки
void mesh_sender() {
    pinMode(16, INPUT_PULLUP);
    mesh_setup();
    // переменные для обработки нажатия кнопки
    uint32_t press_timer = 0;
    bool pressed_flag = false;
    while (true)
    {
        wifi_connection_check();
        if (!digitalRead(16)) {
            if (!pressed_flag) {
                press_timer = millis() + 500;
                pressed_flag = true;
                // отправка пакета на узел, заданный по умолчанию
                mesh_send_packet(default_receiver, MESH_DATA);
                blink_async(120, 1, false);
                
            }
            else if (millis() > press_timer) pressed_flag = false;
        }
    }
}

// отправка сообщения заданного типа на заданный узел
void mesh_send_packet(uint8_t receiver, mesh_message_type type) {
    // заполнение полей со служебной информацией
    router->setupPacket(receiver);
    router->packett[3] = myID;
    router->packett[5] = type;
    uint8_t payload_len = (uint8_t)mesh_data.length();
    if (payload_len > 24) payload_len = 24;
    router->packett[4] = payload_len;
    // данные берутся из строки, заданной по умолчанию
    for (uint8_t i = 0; i < payload_len; i++) router->packett[i + 6] = mesh_data[i];
    router->smartSender(receiver);
}

// режим работы в виде отправки сообщений типа ping по нажатию кнопки
void mesh_ping() {
    pinMode(16, INPUT_PULLUP);
    mesh_setup();
    // переменные для обработки нажатия кнопки
    uint32_t press_timer = 0;
    bool pressed_flag = false;
    while (true)
    {
        wifi_connection_check();
        if (!digitalRead(16)) {
            
            if (!pressed_flag) {
                press_timer = millis() + 500;
                pressed_flag = true;
                // отправка пакета типа ping_request на заданный узел
                mesh_send_packet(default_receiver, MESH_PING_REQUEST);
                blink_async(50, 4, false);
                // ожидание сообщения типа ping_responce от узла
                if (!wait_responce(default_receiver, MESH_PING_RESPONCE, PING_TIMEOUT)) {
                    // отправка сообщения по udp, если ответа нет
                    String out = "no responce from ";
                    out += default_receiver;
                    out += " during ";
                    out += PING_TIMEOUT;
                    out += " ms";
                    send_string(out);
                }
                // изменение номера получателя сообщения при слеующем нажатии кнопки
                do {
                    default_receiver++;
                    if (default_receiver == DRONE_CNT) default_receiver = 1;
                }
                while (default_receiver == myID);
            }
            else if (millis() > press_timer) pressed_flag = false;
        }
    }
}

// ожижание сообщения заданного типа от заданного узла
bool wait_responce(uint8_t t_node, uint8_t t_type, uint32_t timeout) {
    uint32_t start_listening = millis();
    while ((millis() - start_listening) < timeout)
    {
        wifi_connection_check();
        if (radio_->available() == true)
        {
            // получение информации о пакете
            packet receiver;
            uint8_t actual[30];
            radio_->read(&receiver);
            receiver.readPayload(actual, 30);
            uint8_t senderID = actual[0] >> 4;
            uint8_t intermediateID = actual[3];
            uint8_t targetID = actual[0] & 0x0f;
            uint16_t number = (actual[1] << 8) | actual[2];
            uint8_t type = actual[5];
            uint8_t payload_len = actual[4];
            // если пакет актуален
            if (router->checkIfNewMessage(senderID, number))
            {
                String info = packet_info(number, senderID, intermediateID, targetID, type);
                // если текущий узел является получателем
                if (router->checkIfIAmReceiver(actual[0]))
                {
                    info += " is recieved [";
                    info += payload_len;
                    info += "] #: ";
                    for (uint8_t i = 0; i < payload_len; i++) {
                        info += (char)(actual[i + 6]);
                    }

                    Serial.println(info); 
                    // вывод сообщения в udp
                    send_string(info);
                    blink_async(80, 3, false);
                    // возврат true, если параметры принятого сообщения соответствуют заданным
                    if (type = t_type && senderID == t_node) return true;
                    else return false;
                }
                else
                {
                    // если текущий узел не является получателем, пакет перенаправляется 
                    actual[3] = myID;
                    router->routeMessage(actual, info);
                    blink_async(100, 2, false);
                    return false;
                }
            }
        }
    }
    return false;
}

// возврат строки с представлением информации о пакете
String packet_info(uint16_t number, uint8_t senderID, uint8_t intermediateID, uint8_t targetID, uint8_t type) {
    String info = "packet{";
    info += number;
    info += "} [";
    switch (type)
    {
    case 0:
    {
        info += "data";
        break;
    }
    case 1:
    {
        info += "ping request";
        break;
    }
    case 2:
    {
        info += "ping responce";
        break;
    }
    }
    info += "] ";
    info += senderID;
    info += "|";
    info += intermediateID;
    info += " ---> ";
    info += targetID;

    return info;
}