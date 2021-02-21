// ----------------------------------------
// Programm: M5_StiP_TelnetServer.ino
// Aufgabe:  Telnet-Server fuer M5 Stick C PLUS
// Autor:    hoeulm  https://github.com/hoeulm
// Version:  2021-01-25
// Hardware: M5-Stick C PLUS 
//           +----------------------+
//           | ESP32 StickC Plus (#)|   Y      Y   +------------+
//           |      +-------------+ |   | WiFi |   |TelnetClient|
//           |      | LCD 240x135 | o---+      +---o z.B. Putty |
//           |BtnA  | 64k Farben  | |              |            |
//           |      +-------------+ |              +------------+
//           |            BtnB      |
//           +----------------------+
// ----------------------------------------
#include <M5StickCPlus.h>
#include <LITTLEFS.h>
#include "ESPTelnet.h"  // Modified Version (Original by Lennart Hennigs) 
#include "helpers.h"

ESPTelnet telnet;
long      l_mslast=0;

void exec_CMD(String s_cmd)
{
	if(s_cmd=="") return;
	String as_opt[4]={"","","",""};
	int i=s_cmd.indexOf(' ');
	if(i>0)
	{
				as_opt[0]=s_cmd.substring(i+1);    s_cmd=s_cmd.substring(0,i);       i=as_opt[0].indexOf(' ');
		if(i>0){as_opt[1]=as_opt[0].substring(i+1); as_opt[0]=as_opt[0].substring(0,i); i=as_opt[1].indexOf(' ');
		if(i>0){as_opt[2]=as_opt[1].substring(i+1); as_opt[1]=as_opt[1].substring(0,i); i=as_opt[2].indexOf(' ');
		if(i>0){as_opt[3]=as_opt[2].substring(i+1); as_opt[2]=as_opt[2].substring(0,i); }
			}
		}
	}
	if(s_cmd=="led")
		{
			if(as_opt[0]=="on") {digitalWrite(M5_LED,LOW);  telnet.print("done [digitalWrite(M5_LED,LOW);]");return;}
			if(as_opt[0]=="off"){digitalWrite(M5_LED,HIGH); telnet.print("done [digitalWrite(M5_LED,HIGH);]");return;}
			if(as_opt[0]=="tog"){digitalWrite(M5_LED,!digitalRead(M5_LED)); telnet.print("done [digitalWrite(M5_LED,!digitalRead(M5_LED);]");return;}
			String s_LED="ON"; if(digitalRead(M5_LED)){s_LED="OFF";} telnet.print("M5_LED is "+s_LED); return;
		}
	if(s_cmd=="bye" or s_cmd=="logout" or s_cmd=="exit" or s_cmd=="quit"){telnet.print("Closing Session after "+String((millis()-l_mslast)/1000.0,1)+" sec"); delay(1000); telnet.disconnect(); return;}
	if(s_cmd=="uptime") { telnet.print("Uptime: "+s_uptime()); return;}
	if(s_cmd=="clear")  { uint16_t col=as_opt[0].toInt(); if(as_opt[0]=="RED"){col=RED;} if(as_opt[0]=="BLUE"){col=BLUE;} if(as_opt[0]=="GREEN"){col=GREEN;} M5.Lcd.fillScreen(col); telnet.print("done [M5.Lcd.fillScreen("+String(col)+");]"); return;}
	if(s_cmd=="reboot") { telnet.print("REBOOT in 2 sec");   delay(2000); ESP.restart(); return;}
	if(s_cmd=="poweroff"){ telnet.print("POWEROFF in 2 sec"); delay(2000); M5.Axp.PowerOff(); return;}
	if(s_cmd=="bat")    { AXP_Report(); return;}
	if(s_cmd=="sht")    { SHT_Report(); return;}
	if(s_cmd=="bmp")    { BMP_Report(); return;}
	if(s_cmd=="ls")     { String s_Dir=as_opt[0]; if(s_Dir==""){s_Dir="/";} dirFiles(s_Dir); return;}
	if(s_cmd=="dim")    { uint8_t i_dim=as_opt[0].toInt(); if(i_dim>15){i_dim=8;} if(i_dim<8){i_dim=15;} M5.Axp.ScreenBreath(i_dim); telnet.print("done [M5.Axp.ScreenBreath("+String(i_dim)+");]"); return;}
	if(s_cmd=="rot")    { uint8_t i_rot=as_opt[0].toInt()%4; M5.Lcd.setRotation(i_rot); telnet.print("done [M5.Lcd.setRotation("+String(i_rot)+");]"); return;}
	if(s_cmd=="msg")    { String s_msg=as_opt[0]; if(as_opt[1]!=""){s_msg+=" "+as_opt[1];} M5.Lcd.drawCentreString(s_msg,M5.Lcd.width()/2,2,4); telnet.print("done [M5.Lcd.drawCentreString(s_msg,x,y,4);]"); return;}
	if(s_cmd=="cat")    { String s_fn=as_opt[0]; cat_file(s_fn); return;}
	if(s_cmd=="mkdir")  { String s_fn=as_opt[0]; LITTLEFS.mkdir(s_fn); return;}
	if(s_cmd=="remove") { String s_fn=as_opt[0]; LITTLEFS.remove(s_fn); return;}
	if(s_cmd=="time")   { long l_ms0=millis()+999; while(true) {if((millis()-l_ms0)>1000){l_ms0=millis(); drawLocalTime(); } if(telnet.available()){break;}} return;}
	if(s_cmd=="wget")   { int i=as_opt[0].substring(8).indexOf('/');  String s_path=as_opt[0].substring(8+i); String s_host=as_opt[0]; String s_fn=as_opt[1];
							if(s_host.substring(0,5)=="https")
									{ s_host=s_host.substring(8,8+i); wget_to_LITTLEFS_HTTPS(s_host,s_path,s_fn); }
								else{ s_host=s_host.substring(7,8+i); wget_to_LITTLEFS_HTTP( s_host,s_path,s_fn); }
							return;
						}
	if(s_cmd=="img")    { String s_bmp=as_opt[0]; int xo=as_opt[1].toInt(); int yo=as_opt[2].toInt(); float zoom=as_opt[3].toFloat();
							drawBMP565(s_bmp,xo,yo,zoom); telnet.print("done [drawBMP565("+s_bmp+","+String(xo)+","+String(yo)+","+String(zoom)+");]"); return;}
	if(s_cmd=="i2c")    { Wire.begin(0,26);
							for(uint16_t i_adr=0; i_adr<128; i_adr++)
							{Wire.beginTransmission(i_adr); int i_res=Wire.endTransmission();
								if(i_res!=0){telnet.print(".. ");} else{String s=String(i_adr,16); if(i_adr<16){s="0"+s;} telnet.print(s+" ");} if(i_adr%16==15){telnet.print("\r\n");}
							}
							return;
						}
	if(s_cmd=="whoami") { telnet.print("Hallo, ");  telnet.setTextColor(A_BOLD); String s_id=telnet.getID();  telnet.print(s_id); telnet.print(A_RESET); return;}
	if(s_cmd=="ansi")   { telnet.b_isANSI=true;
							if(as_opt[0]=="off"){telnet.b_isANSI=false;} s_cmd="rgb";
							if(as_opt[0]=="cls"){telnet.print(A_CLSHOME); return;}
						}
  if(s_cmd=="rgb")      { 	telnet.setTextColor(A_RED);   telnet.print("Tomato ");
							telnet.setTextColor(A_GREEN); telnet.print("Salad ");
							telnet.setTextColor(A_BLUE);  telnet.print("Water ");
							telnet.setTextColor(A_RESET); telnet.print("done");
							return;
						}
	if(s_cmd="p2323")   {
							telnet.println("Closing Session after "+String((millis()-l_mslast)/1000.0,1)+" sec");
							telnet.println("Changing TelnetPort to ! 2323 !"); delay(1000); telnet.disconnect();
							telnet.stop(); telnet.setPort(2323); telnet.begin(); return;
						}
	if(s_cmd=="help" or s_cmd=="?"){telnet.print("Allowed Commands= led on, led off, led tog, bye, logout, exit, quit, msg text, clear, help, ls, bat"); return;}
	telnet.print("SORRY, i dont know who to do \""+s_cmd+"\"");
}

void setup_Telnet()
{
	telnet.onConnect(onTelnetConnect);                    // passing on functions for various telnet events
	telnet.onConnectionAttempt(onTelnetConnectionAttempt);
	telnet.onReconnect(onTelnetReconnect);
	telnet.onDisconnect(onTelnetDisconnect);
	telnet.onInputReceived([](String str) { exec_CMD(str); telnet.print("\r\n>");});   // checks for a certain command
	telnet.begin();
	M5.Lcd.println("Ready for Connects on Port "+telnet.getPort());
}

// (optional) callback functions for telnet events
void onTelnetConnect(String ip)
{
	M5.Lcd.print("Connect from "+telnet.getIP());
	telnet.print("\r\nWelcome to M5StickCPlus on "+WiFi.localIP().toString()+" from " + telnet.getIP());
	telnet.print("\r\n>");
}

void onTelnetDisconnect(String ip)
{	Serial.print("- Telnet: ");  Serial.print(ip);  Serial.println(" disconnected");}

void onTelnetReconnect(String ip)
{	Serial.print("- Telnet: ");  Serial.print(ip);  Serial.println(" reconnected"); }

void onTelnetConnectionAttempt(String ip)
{	Serial.print("- Telnet: ");  Serial.print(ip);  Serial.println(" tried to connected");}

void setup()
{
	M5.begin();
	M5.Lcd.setRotation(3); M5.Lcd.fillScreen(BLACK);
	M5.Lcd.drawString("Telnet Server",0,0,4);
	M5.Lcd.setCursor(0,24);
	pinMode(M5_LED,OUTPUT); digitalWrite(M5_LED,LOW); delay(100); digitalWrite(M5_LED,HIGH);
	LITTLEFS.begin(true);
	setup_WiFi();
	setup_Telnet();
}

void loop()
{
	telnet.loop();
}

