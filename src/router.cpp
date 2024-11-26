#include "router.h"
#include "header.h"

// Конструктор
Router::Router(uint8_t myID, RF24_G &radio)
{
	this->radio = radio;
	this->myID = myID;
	// this->radio = RF24_G(myID, 9, 10);
	smartSendRetriesCount = 1;
	smartSendDelayRetries = 200;

	for (int i = 0; i < DRONE_CNT; i++)
	{
		lastNumbersOfPackets[i] = 0;
	}
}

// Узнать записанный ID
uint8_t Router::getMyID()
{
	return myID;
}

// Так как мы собираемся прокидывать траффик между разными модулями
// необходимо запомнить адресата и номер посылки
void Router::generateExtraPayload(uint8_t receiver)
{
	packett[0] = myID << 4 | receiver;
	packett[1] = lastNumbersOfPackets[myID] >> 8;
	packett[2] = lastNumbersOfPackets[myID] & 0x00ff; // откусил лишнее
}

// очистка пакета
void Router::clearPacket()
{
	for (int i = 0; i < 30; i++)
	{
		packett[i] = 0;
	}
}

// Настройка пакета
void Router::setupPacket(uint8_t receiver)
{
	clearPacket();
	generateExtraPayload(receiver);
}

// Проверка, является ли сообщение новым
bool Router::checkIfNewMessage(uint8_t senderID, uint16_t numberOfPacket)
{
	bool result = ((lastNumbersOfPackets[senderID] < numberOfPacket) || (abs(numberOfPacket - lastNumbersOfPackets[senderID]) > 30000) || (numberOfPacket < 5));

	if (result)
	{
		lastNumbersOfPackets[senderID] = numberOfPacket;
	}

	return result;
}

// проверка на то, что является приёмником
bool Router::checkIfIAmReceiver(uint8_t extraPayload)
{
	return (extraPayload & 0x0f) == myID; // откусываем лишнее
}

// Перенаправить сообщение
void Router::routeMessage(uint8_t packetToRoute[], String info)
{
	uint8_t receiver = packetToRoute[0] & 0x0f;
	uint8_t firstSender = packetToRoute[0] >> 4; // мы не хотим возвращать отправителю его же пакет

	String temp = info;
	temp += " is redirected to ~> ";
	for (int i = 1; i < DRONE_CNT; i++)
	{
		if ((i != myID) && (i != firstSender))
		{
			// packetToRoute[3] = myID;
			packet sender;
			sender.setAddress(i);
			sender.addPayload(packetToRoute, 30); // перенаправляем копию как есть
			// sender.addPayload(packetToRoute, sizeof(packetToRoute)); //перенаправляем копию как есть
			// Serial.println(packetToRoute[3]);
			radio.write(&sender);

			send_string(temp + i);
			Serial.println(temp + i);
		}
	}
}

// Отправка, которая автоматически сделает редирект
void Router::smartSender(uint8_t receiver)
{
	packet sender;
	sender.setAddress(receiver);
	sender.addPayload(packett, 30);
	uint8_t type = sender.buffer[5];
	uint16_t number = (sender.buffer[1] << 8) | sender.buffer[2];
	// Serial.printf("a: %i\n", sender.buffer[0]);
	bool ok = radio.write(&sender); // отправили

	// если не получилось отправить, то мы должны передать остальным, что что-то пошло не так
	// TODO ОБЕРУНТЬ В ФУНКЦИЮ
	uint32_t timer2 = millis();
	if (!ok)
	{
		for (int i = 0; i < smartSendRetriesCount; i++)
		{
			while (millis() - timer2 < smartSendDelayRetries)
			{
			}

			timer2 = millis();
			ok = radio.write(&sender);
			if (ok)
			{
				break;
			}
		}
	}

	String info = packet_info(number, myID, myID, receiver, type);

	if (!ok)
	{
		// Serial.println("Редирект");
		
		for (int i = 1; i < DRONE_CNT; i++)
		{
			if ((i != myID) && (i != receiver))
			{
				sender.setAddress(i);
				radio.write(&sender);

				//info += " is redirected to ~> ";
				//info += i;
				send_string(info + " is redirected to ~> " + i);
			}
		}
	}
	else
	{
		info += " sended to receiver -> ";
		info += receiver;
		send_string(info);
	}
	lastNumbersOfPackets[myID]++;
}

// временно неактуальная функция
bool Router::radioAvailable()
{
	return radio.available();
}