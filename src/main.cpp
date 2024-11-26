#include "header.h"

void setup()
{
	blink_async(25, 2, true);
	Serial.begin(115200);
	if (!wifi_connect()) halt("poweroff");
	blink_stop();
	poweron_msg("hello!");

	// radio_scan();
	// radio_tx_test();
	// radio_rx_test();
#ifndef PING_MODE
#ifdef SEND_MODE
	mesh_sender();
#else
	mesh_router_test();
#endif

#else
	mesh_ping();
#endif
}

void loop()
{
}
