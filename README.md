# M5_Stick_Plus_TelnetServer
Telnet Server for M5 Stick Plus
(uses a modified Version of https://github.com/LennartHennigs/ESPTelnet)<br/>
This allows you to control your M5 Stick Plus via a Telnet-Session <br/>
The following Commands are already supported:
<ul>
<li><b>led</b> on | off | tog</b></li>
<li><b>uptime</b></li>
<li><b>clear</b></li>
<li><b>bat</b> Show Status of the Akku</li>
<li><b>rot</b> Rotate the Display</li>
<li><b>sht</b> Show Temperature and Humidity from external ENVII HAT</li>
<li><b>bmp</b> Show Temperature and Pressure from external ENVII HAT</li>
<li><b>wget</b> Download a File from the Internet and store it in LITTEFS</li>
<li><b>img</b> Show a 565-Bitmap on the LCD</li>
<li><b>i2c</b> Scan the I2C-Bus for Devices</li>
<li><br>mkdir</b> Create a Directory on LITTEFS</li>
<li><b>..</b></li>
</ul>
FAR away from a Operating System on M5-Stick, but a nice begin..

