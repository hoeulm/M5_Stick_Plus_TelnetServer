#pragma once

#include <M5StickCPlus.h>
#include <LITTLEFS.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>"
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#include "SHT3X.h"
#include "ESPTelnet.h"
#include "MyWiFi_Config.h"   // this contains two lines as this

	// #define WiFi_SSID "MyWiFiSSID"  // <== CHANGE THIS
	// #define WiFi_PASS "TopSecret"   // <== CHANGE THIS
extern ESPTelnet telnet;

RTC_TimeTypeDef rtc_Time;
RTC_DateTypeDef rtc_Date;

String s_uptime()
{
	int ms=millis(), hh=ms/3600000, mi=(ms%3600000)/60000,ss=(ms%60000)/1000; ms%=1000;
	char ac_buff[13];
	sprintf(ac_buff, "%02d:%02d:%02d.%03d",hh,mi,ss,ms);
	return String(ac_buff);
}

bool setup_WiFi(int i_cntdown=100)
{
	uint16_t xo=M5.Lcd.width()/2-50, yo=36;
	M5.begin();
	M5.Lcd.print("Connect to WiFi "+String(WiFi_SSID)+" ");
	WiFi.mode(WIFI_STA);
	M5.Lcd.fillRect(xo,yo,100,16,GREEN);
	WiFi.begin(WiFi_SSID,WiFi_PASS);
	while(WiFi.status()!= WL_CONNECTED)
	{
		delay(500); i_cntdown-=5; M5.Lcd.fillRect(xo+i_cntdown,yo,100-i_cntdown,16,BLACK);
		if(i_cntdown==0){M5.Lcd.setTextColor(RED); M5.Lcd.print("Timeout"); delay(20000); ESP.restart(); return false; }
	} 
	M5.Lcd.println(" OK"); M5.Lcd.fillRect(xo,yo,100,16,BLACK);
	M5.Lcd.print("IP(byDHCP)="); M5.Lcd.setTextColor(GREEN);M5.Lcd.setTextSize(2); M5.Lcd.println(WiFi.localIP().toString());
	M5.Lcd.setTextColor(WHITE,BLACK); M5.Lcd.setTextSize(1);
	WiFi.setAutoReconnect(true);
	WiFi.persistent(true);
	return true;
}

void SHT_Report()
{
	SHT3X sht3x;
	Wire.begin(0,26);
	if(sht3x.get()!=0) { telnet.print("SHT30 NOT found\r\n"); return;}
	float f_tmpC = sht3x.cTemp,f_humi = sht3x.humidity;
	telnet.print("SHT30\r\n  Temp="+String(f_tmpC,2)+" 'C\r\n  Humi="+String(f_humi,1)+" % RF");
}

void BMP_Report()
{
	Adafruit_BMP280 bme;
	float f_bmp_tmpC,f_bmp_pres;
	Wire.begin(0,26);
	if(!bme.begin(0x76)) { telnet.print("BMP280 NOT found"); return;}
	f_bmp_tmpC=bme.readTemperature();   f_bmp_pres = bme.readPressure()/100.0;
	telnet.print("BMP280\r\n  Temp="+String(f_bmp_tmpC,2)+" 'C\r\n  Pres="+String(f_bmp_pres,1)+" hPa");
}

void AXP_Report()
{
	telnet.print("Battery\r\n");    
	telnet.print(" State:"+String(M5.Axp.GetBatState())+"\r\n");
	telnet.print(" Warn :"+String(M5.Axp.GetWarningLevel())+"\r\n");
	telnet.print(" Temp :"+String(M5.Axp.GetTempInAXP192(),1)+"'C\r\n");
	telnet.print(" U[V] :"+String(M5.Axp.GetBatVoltage(),3)+"\r\n"); 
	telnet.print(" I[mA]:"+String(M5.Axp.GetBatCurrent(),1)+"\r\n"); 
	telnet.print(" P[mW]:"+String(M5.Axp.GetBatPower(),3)+"\r\n");
	telnet.print("ASP\r\n");
	telnet.print(" U[V] :"+String(M5.Axp.GetAPSVoltage(),3)+"\r\n");  
	telnet.print("VBus(USB)\r\n");
	telnet.print(" U[V] :"+String(M5.Axp.GetVBusVoltage(),3)+"\r\n"); 
	telnet.print(" I[mA]:"+String(M5.Axp.GetVBusCurrent(),1)+"\r\n"); 
	telnet.print("VIN(5V-In)\r\n");
	telnet.print(" U[V] :"+String(M5.Axp.GetVinVoltage(),3)+"\r\n");  
	telnet.print(" I[mA]:"+String(M5.Axp.GetVinCurrent(),1)+"\r\n"); 
	telnet.print("Uptime:"+s_uptime());
}

void dirFiles(String s_path)
{
	int i_cntd=0,i_cntf=0;
	telnet.print("List Files on LITTLEFS in "+s_path+ "\r\n");
	File root=LITTLEFS.open(s_path);
	if(root)
	{
		while(true)
		{
			File entry =  root.openNextFile(); if(!entry) break;
			if (entry.isDirectory())
					{i_cntd++; telnet.print("DIR: "+String(entry.name())+"\r\n");}
				else{i_cntf++; String s=entry.name(); while(s.length()<19){s+=" ";} s+=String(entry.size()); telnet.print(s+"\r\n"); }
		}
		root.close();
	}
	int i_used=100*LITTLEFS.usedBytes()/LITTLEFS.totalBytes();
	telnet.print("Total "+String(i_cntd)+" Dirs\r\n");
	telnet.print("Total "+String(i_cntf)+" Files\r\n");
	telnet.print("Usage "+String(i_used)+" %\r\n");
	telnet.print("SIZE: "+String(LITTLEFS.totalBytes()/1024)+" kByte\r\n");
}

void cat_file(String s_fn)
{
	File fh=LITTLEFS.open(s_fn);
	String s_Line;
	while(fh.available())
	{ s_Line=fh.readStringUntil('\n'); telnet.print(s_Line+"\n"); }
}

bool wget_to_LITTLEFS_HTTPS(String s_host, String s_path, String s_fnlocal)
{
	long   l_ms0=millis();
	int    i_cnt=0,i_port=443;
	String s_Line,s_prot="https";
	if(s_fnlocal==""){s_fnlocal=s_path;}
	WiFiClientSecure client;
	if(!client.connect(s_host.c_str(),i_port))
	{ telnet.print("[ERR] "+s_host+" Connection failed!"); return false;}
	File fh_down=LITTLEFS.open(s_fnlocal,"w");
	if(!fh_down)
		{ telnet.print("[ERR] "+ s_fnlocal+"  open on SPIFFS failed!"); return false;}
	String s_GET="GET "+s_prot+"://"+s_host+s_path+" HTTP/1.0";
	telnet.print(s_GET+ " on Port "+String(i_port));
	client.println(s_GET);
	client.println("Host: "+s_host);
	client.println("User-Agent: ESP32/M5StickCPlus");
	client.println("Connection: close"); // Close AFTER download NO KEEPALIVE!!
	client.println();          // Endof REQUEST
	while(client.connected())  // Header LOOP
	{
		s_Line = client.readStringUntil('\n');
		if(s_Line.length()==1) { telnet.print("[OK] header received\r\n");  break; }
	}
	while(client.connected())  // DAta Loop
	{
		while (client.available()) { uint8_t b=client.read(); fh_down.write(b); } // Write received Byte to "DISK"
	}
	fh_down.close();
	telnet.println("[OK] Download done in "+String((millis()-l_ms0)/1000.0,3)+" sec");
	return true;
}

bool wget_to_LITTLEFS_HTTP(String s_host, String s_path, String s_fnlocal)
{
	long   l_ms0=millis();
	int    i_cnt=0,i_port=80;
	String s_Line,s_prot="http";
	telnet.print(s_fnlocal+"\r\n");
	if(s_fnlocal==""){s_fnlocal=s_path;}
	WiFiClient clienth;
	if(!clienth.connect(s_host.c_str(),i_port))
	{ telnet.print("[ERR] "+s_host+" on Port "+String(i_port)+" Connection failed!"); return false;}
	telnet.print(s_path+" to "+s_fnlocal+"\r\n");
	File fh_down=LITTLEFS.open(s_fnlocal,"w");
	if(!fh_down)
		{ telnet.print("[ERR] "+ s_fnlocal+"  open on SPIFFS failed!"); return false;}
	String s_GET="GET "+s_prot+"://"+s_host+s_path+" HTTP/1.0";
	telnet.print(s_GET+ " on Port "+String(i_port)+"\r\n");
	clienth.println(s_GET);
	clienth.println("Host: "+s_host);
	clienth.println("User-Agent: ESP32/M5StickCPlus");
	clienth.println("Connection: close"); // Close AFTER download NO KEEPALIVE!!
	clienth.println();          // Endof REQUEST
	while(clienth.connected())  // Header LOOP
	{
		s_Line = clienth.readStringUntil('\n');
		if(s_Line.length()==1) { telnet.print("[OK] header received\r\n");  break; }
	}
	while(clienth.connected())  // DAta Loop
	{
		while (clienth.available()) { uint8_t b=clienth.read(); fh_down.write(b); } // Write received Byte to "DISK"
	}
	fh_down.close();
	telnet.println("[OK] Download done in "+String((millis()-l_ms0)/1000.0,3)+" sec");
	return true;
}

void drawBMP565(String s_fn,int xo,int yo,float f_zoom)
{
	int     i,w,h,col,id,x,xz,y,yz,offs;
	uint8_t ai_bmphead[14];
	uint8_t ai_bmpinfo[40];
	File bmp = LITTLEFS.open(s_fn,"r");
	if(!bmp){telnet.print("[ERR] NO "+s_fn+" on LITTLEFS"); dirFiles("/"); return;}
	i=bmp.read(ai_bmphead,14); i=bmp.read(ai_bmpinfo,40);
	id=256*ai_bmphead[0]+ai_bmphead[1]; offs=ai_bmphead[10];
	w=ai_bmpinfo[4]+256*ai_bmpinfo[5];  h=ai_bmpinfo[8]+256*ai_bmpinfo[9];
//	Serial.printf("BMP565 %-17s size=%d %4X offs=%4d w=%4d h=%4d zoom=%4.2f xo=%3d yo=%3d\n",s_fn.c_str(),bmp.size(),id,offs,w,h,f_zoom,xo,yo);
	if(id!=0x424d){Serial.printf("NO valid BMP %4X\n",id); return;}
	if(f_zoom==1.0)
			 for(y=0;y<h-1;y++){for(x=0;x<w;x++){col=bmp.read()+256*bmp.read(); M5.Lcd.drawPixel(xo+x,yo+y,col);}}
		else for(y=0;y<h-1;y++){for(x=0;x<w;x++){col=bmp.read()+256*bmp.read();xz=x*f_zoom; yz=y*f_zoom; M5.Lcd.drawPixel(xo+xz,yo+yz,col);}}
}

void drawLocalTime()
{
	M5.Lcd.fillScreen(BLACK);
	static const char *ac_wd[7] = {"So","Mo","Di","Mi","Do","Fr","Sa"};
	M5.Rtc.GetTime(&rtc_Time);
	M5.Rtc.GetData(&rtc_Date);
	M5.Lcd.setTextColor(GREEN,BLACK);
	uint16_t w=M5.Lcd.width();
	if(w==80) // x,y,font 7
	{M5.Lcd.setCursor( 0, 2,7); M5.Lcd.printf("%02d",rtc_Time.Hours); M5.Lcd.fillCircle(75,18,3,GREEN); M5.Lcd.fillCircle(75,36,3,GREEN);M5.Lcd.setCursor( 0,60,7); M5.Lcd.printf("%02d",rtc_Time.Minutes);M5.Lcd.setCursor(68,70,1); M5.Lcd.printf("%02d",rtc_Time.Seconds); M5.Lcd.setCursor(0,110, 1);}
	if(w==135)
	{M5.Lcd.setCursor( 0,25,7); M5.Lcd.printf("%02d",rtc_Time.Hours); M5.Lcd.fillCircle(66,38,3,GREEN); M5.Lcd.fillCircle(66,58,3,GREEN);M5.Lcd.setCursor(70,25,7); M5.Lcd.printf("%02d",rtc_Time.Minutes);M5.Lcd.setCursor(20,90, 2);}
	if(w==160)
	{ M5.Lcd.setCursor(0,2,7);M5.Lcd.printf("%02d:%02d", rtc_Time.Hours, rtc_Time.Minutes); M5.Lcd.setTextFont(2); M5.Lcd.printf(":%02d\n",rtc_Time.Seconds);M5.Lcd.setCursor(80,90, 2); }
	if(w>160)
	{ M5.Lcd.setCursor(45,2,7);M5.Lcd.printf("%02d:%02d", rtc_Time.Hours, rtc_Time.Minutes); M5.Lcd.setTextFont(2); M5.Lcd.printf(":%02d\n",rtc_Time.Seconds);M5.Lcd.setCursor(80,90, 2); }
	M5.Lcd.setTextColor(WHITE,BLACK);
	M5.Lcd.printf("%04d-%02d-%02d %s\n", rtc_Date.Year, rtc_Date.Month, rtc_Date.Date, ac_wd[rtc_Date.WeekDay]);
//	if(i_hh_last!=rtc_Time.Hours){beep_Hours(rtc_Time.Hours);}
}
