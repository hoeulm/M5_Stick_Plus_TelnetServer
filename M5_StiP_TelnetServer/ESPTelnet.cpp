// Original by Lennart Hennigs https://github.com/LennartHennigs/ESPTelnet
#include "ESPTelnet.h"

ESPTelnet::ESPTelnet() { isConnected = false; }

/* ------------------------------------------------- */

bool ESPTelnet::begin()
{
	ip = "";
	if(WiFi.status() == WL_CONNECTED)
			{ server.begin(i_port); server.setNoDelay(true); return true;}
		else{ return false; }
}

/* ------------------------------------------------- */

void ESPTelnet::stop()
{	server.stop(); }

/* ------------------------------------------------- */

bool ESPTelnet::isClientConnected(WiFiClient client)
{
	return client.connected();
}

/* --added by hoeulm-------------------------------- */

void ESPTelnet::disconnect()
{
	client.stop();
}

/* --added by hoeulm-------------------------------- */
bool ESPTelnet::available()
{
	return client.available();
}

/* --added by hoeulm-------------------------------- */
void ESPTelnet::setPort(uint16_t p)
{
	i_port=p;
}
/* --added by hoeulm-------------------------------- */
String ESPTelnet::getPort()
{
	return String(i_port);
}

/* --added by hoeulm-------------------------------- */

String ESPTelnet::getID()
{
	client.print("\x5");
	long l_ms0=millis(); while(!client.available() and (millis()-l_ms0)<1000){};
	String s="";
	while(client.available()){s+=client.read();}
	b_isPuTTY=(s=="PuTTY");
	return s;
}

/* --added by hoeulm-------------------------------- */

void ESPTelnet::setTextColor(String s)
{
	if(b_isANSI){ client.print(s);}
}

/* ------------------------------------------------- */

void ESPTelnet::loop()
{
//	check if there are any new clients
	if(server.hasClient())
	{
		isConnected = true;
		// already a connection?
		if(client && client.connected() && isClientConnected(client))
		{
			WiFiClient newClient = server.available();
			attemptIp  = newClient.remoteIP().toString();
			// reconnected?
			if(attemptIp == ip)
			{
				if(on_reconnect != NULL) on_reconnect(ip);
				client.stop();
				client = newClient;
				// disconnect the second connection
			}
			else
			{
				if(on_connection_attempt != NULL) on_connection_attempt(ip);
				return;
			}
		}
		else // first connection
		{
			client = server.available();
			ip = client.remoteIP().toString();
			if(on_connect != NULL) on_connect(ip);
			client.setNoDelay(true);
			client.flush();
		}
	}

	if(client && isConnected && !isClientConnected(client))     // check whether to disconnect
	{
		if(on_disconnect != NULL) on_disconnect(ip);
		isConnected = false;
		ip = "";
	}

	if(client && isConnected && client.available())             // gather input
	{
		char c = client.read();
		if(c != '\n')
				{if(c >= 32) { input += c; }	}
			else{if(on_input != NULL) on_input(input); input = ""; } // EOL -> send input
	}
	yield();
}

/* ------------------------------------------------- */

void ESPTelnet::print(char c)
{
	if(client && isClientConnected(client)) {client.print(c); }
}

/* ------------------------------------------------- */

void ESPTelnet::print(String str)
{
	if(client && isClientConnected(client)) {client.print(str); }
}

/* --modified by hoeulm----------------------------- */

void ESPTelnet::println(String str)
{
	if(!b_addCR){client.print(str + "\n");} else {client.print(str + "\r\n");}
}

/* ------------------------------------------------- */

void ESPTelnet::println(char c)
{
	client.print(c + "\n");
}

/* ------------------------------------------------- */

void ESPTelnet::println()
{
	client.print("\n");
}

/* ------------------------------------------------- */

String ESPTelnet::getIP() const
{
	return ip;
}

/* ------------------------------------------------- */

String ESPTelnet::getLastAttemptIP() const
{
	return attemptIp;
}

/* ------------------------------------------------- */

void ESPTelnet::onConnect(CallbackFunction f)
{
	on_connect = f;
}

/* ------------------------------------------------- */

void ESPTelnet::onConnectionAttempt(CallbackFunction f)
{
	on_connection_attempt = f;
}

/* ------------------------------------------------- */

void ESPTelnet::onReconnect(CallbackFunction f)
{
	on_reconnect = f;
}

/* ------------------------------------------------- */

void ESPTelnet::onDisconnect(CallbackFunction f)
{
	on_disconnect = f;
}

/* ------------------------------------------------- */

void ESPTelnet::onInputReceived(CallbackFunction f)
{
	on_input = f;
}

/* ------------------------------------------------- */
