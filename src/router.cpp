#include "router.h"
#include "header.h"

// конструктор
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

// возврат id
uint8_t Router::getMyID()
{
	return myID;
}

// подготовка полей со служебной информацией пакета
void Router::generateExtraPayload(uint8_t receiver)
{
	packett[0] = myID << 4 | receiver; // номер отправителя
	packett[1] = lastNumbersOfPackets[myID] >> 8; // номер пакета
	packett[2] = lastNumbersOfPackets[myID] & 0x00ff;
}

// очистка пакета
void Router::clearPacket()
{
	for (int i = 0; i < 30; i++)
	{
		packett[i] = 0;
	}
}

// подготовка полей со служебной информацией пакета
void Router::setupPacket(uint8_t receiver)
{
	clearPacket();
	generateExtraPayload(receiver);
}

// проверка, является ли сообщение новым
bool Router::checkIfNewMessage(uint8_t senderID, uint16_t numberOfPacket)
{
	bool result = ((lastNumbersOfPackets[senderID] < numberOfPacket) || (abs(numberOfPacket - lastNumbersOfPackets[senderID]) > 30000) || (numberOfPacket < 5));

	if (result)
	{
		lastNumbersOfPackets[senderID] = numberOfPacket;
	}
	return result;
}

// проверка на то, что текущий узел является получаталем пакета
bool Router::checkIfIAmReceiver(uint8_t extraPayload)
{
	return (extraPayload & 0x0f) == myID;
}

// перенаправление сообщения. вызывается в случае, если текущий узел не является конечным получателем пакета
void Router::routeMessage(uint8_t packetToRoute[], String info)
{
	// получение номеров отрпавителя и получателя из служебных полей пакета
	uint8_t receiver = packetToRoute[0] & 0x0f;
	uint8_t firstSender = packetToRoute[0] >> 4;
	// подготовка строки с информацией о перенаправлении 
	String temp = info;
	temp += " is redirected to ~> ";
	// отправка пакета всем узлам в сети, кроме отправителя
	for (int i = 1; i < DRONE_CNT; i++)
	{
		if ((i != myID) && (i != firstSender))
		{
			
			packet sender; // создание пакета для отправки
			sender.setAddress(i); // установка адреса получателя
			sender.addPayload(packetToRoute, 30); // копируем данные из полученного пакета
			radio.write(&sender); // отправляем пакет
			// отправляем информационное сообщение по udp
			send_string(temp + i);
			Serial.println(temp + i);
		}
	}
}

// отправка сообщения
void Router::smartSender(uint8_t receiver)
{
	// создание пакета
	packet sender;
	sender.setAddress(receiver);
	sender.addPayload(packett, 30);
	uint8_t type = sender.buffer[5];
	uint16_t number = (sender.buffer[1] << 8) | sender.buffer[2];
	// попытка отправить сразу получателю
	bool ok = radio.write(&sender); 
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
	// если получатель недоступен, отправляем пакет на все остальные узлы
	if (!ok)
	{	
		for (int i = 1; i < DRONE_CNT; i++)
		{
			if ((i != myID) && (i != receiver))
			{
				sender.setAddress(i);
				radio.write(&sender);
				send_string(info + " is redirected to ~> " + i);
			}
		}
	}
	else
	{
		// если получатель доступен, выводим информационное сообщение
		info += " sended to receiver -> ";
		info += receiver;
		send_string(info);
	}
	// прибавление счётчика отрпавленных пакетов
	lastNumbersOfPackets[myID]++;
}

bool Router::radioAvailable()
{
	return radio.available();
}