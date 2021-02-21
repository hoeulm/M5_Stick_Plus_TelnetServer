// Original by Lennart Hennigs https://github.com/LennartHennigs/ESPTelnet

#pragma once

#ifndef ESPTelnet_h
#define ESPTelnet_h

#include <WiFi.h>
#include <WebServer.h>

#define A_BLACK   "\x1B[30m"   // ANSICODES for Terminal-Farben
#define A_RED     "\x1B[31m"
#define A_GREEN   "\x1B[32m"
#define A_YELLOW  "\x1B[33m"
#define A_BLUE    "\x1B[34m"
#define A_MAGENTA "\x1B[35m"
#define A_CYAN    "\x1B[36m"
#define A_WHITE   "\x1B[37m"
#define A_BOLD    "\x1B[1m"    // STRONG Schrift
#define A_RESET   "\x1B[0m"    // Reset all Attributes
#define A_CLSHOME "\x1B[2J"    // ClearScreen and Home Cursor

class ESPTelnet
{
	typedef void (*CallbackFunction) (String str);

	public:
		ESPTelnet();
		bool   begin();
		void   loop();
		void   stop();
		void   disconnect();       // Disconnect a Client, but dont stop the Server
		bool   available();        // is there a Input from Client, usefull to break loops
		void   print(String str);
		void   print(char c);
		void   println(String str);
		void   println(char c);
		void   println();
		String getID();             // Get Client-ID by sending 0x5 to Client, Putty returns "PuTTY"
		bool   b_isPuTTY=false;     // Has Client identifiyed himself as "PuTTY"
		bool   b_addCR=false;       // Add a Carridge Return to LineFeed
		bool   b_isANSI=false;      // does the Client support ANSI-Sequences
		void   setTextColor(String s);  // Set a Color for the Terminal(if b_isANSI);
		void   setPort(uint16_t p); // Set a new PortAddress for Telnet Service, does  stop() + begin()
		String getPort();           // Returns the Port of Telnet-Service as a String
		String getIP() const;
		String getLastAttemptIP() const;

		void   onConnect(CallbackFunction f);
		void   onConnectionAttempt(CallbackFunction f);
		void   onReconnect(CallbackFunction f);
		void   onDisconnect(CallbackFunction f);
		void   onInputReceived(CallbackFunction f);

	protected:
		uint16_t   i_port=23;       // allow a different port than 23 for telnet(more secure?)
		WiFiServer server = WiFiServer(i_port);
		WiFiClient client;
		boolean    isConnected = false;
		String     ip;
		String     attemptIp;
		String     input = "";

		bool       isClientConnected(WiFiClient client);

		CallbackFunction on_connect            = NULL;
		CallbackFunction on_reconnect          = NULL;
		CallbackFunction on_disconnect         = NULL;
		CallbackFunction on_connection_attempt = NULL;
		CallbackFunction on_input              = NULL;
};
// << operator
template<class T> inline ESPTelnet &operator <<(ESPTelnet &obj, T arg) { obj.print(arg); return obj; }
#endif
