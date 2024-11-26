#include "rf24g.h"
#include "RF24.h"
#ifndef ROUTER_H_
#define ROUTER_H_
#include "Arduino.h"

// число узлов в сети + 1
const int DRONE_CNT = 6;

class Router
{
public:
  Router(uint8_t myID, RF24_G &radio);
  uint8_t getMyID();
  void generateExtraPayload(uint8_t receiver);
  void setupPacket(uint8_t receiver);
  bool checkIfNewMessage(uint8_t senderID, uint16_t numberOfPacket);
  bool checkIfIAmReceiver(uint8_t extraPayload);
  void routeMessage(uint8_t packetToRoute[], String info);
  void smartSender(uint8_t receiver);
  bool radioAvailable();
  void changeSendRetriesCount(uint8_t);
  void clearPacket();
  // массив данных пакета для отправки
  uint8_t packett[30];
  // id текущего узла
  uint8_t myID;
  // массив счётчиков пакетов узлов
  uint16_t lastNumbersOfPackets[DRONE_CNT];
private:
  RF24_G radio;
  // число попыток отправить пакет получателю напрямую
  uint8_t smartSendRetriesCount;
  // задержка между попытками
  uint16_t smartSendDelayRetries;
};
#endif