<h1>OBI Socket V2</h1>

<h2>Features</h2>
<ul>
  <li>Use NTP for time</li>
  <li>Convert from UTC to local time and DST</li>
  <li>Manual and automatic time triggered On/Off</li>
  <li>OTA update</li>
</ul>

<h2>Used Libraries</h2>
<ul>
  <li><a href="https://github.com/esp8266/Arduino">Arduino Core for ESP8266</a></li>
</ul>

<h2>Setup</h2>
<ol>
  <li>Open the Arduino IDE and load the Clock.ino sketch</li>
  <li>In the Tools menu set the "Flash size" to use SPIFS</li>
  <li>Flash the ESP8266 using Arduino IDE</li>
  <li>
    Connect the NodeMCU with the buck converter, WS2812 LEDs and the photo resistor:<br/>
    <img src="images/Clock.sch.png" width="600" alt="schematic"/><br/>
  </li>
</ol>

<h2>First Start</h2>
<ul>
  <li>Open the Serial Monitor in Arduino IDE</li>
  <li>Power on / reset the Clock</li>
  <li>Connect to Network with SSID = "Clock" and PSK as shown in the Serial Monitor (e.g. "clock-123456")</li>
  <li>Open the web page "https://192.168.4.1"</li>
  <li>On the Settings page enter your WiFi credentials (SSID and PSK) and press "save"</li>
  <li>The clock will now connect to the network, fetch the current time and display it</li>
  <li>At next power on the clock will remember the settings</li>
</ul>
  
<h2>UI Screenshots</h2>
<p>
<a style="vertical-align: top" href="images/home.png"    ><img src="images/home.png"     alt="home"     width="250" border="1px"/></a>
<a style="vertical-align: top" href="images/settings.png"><img src="images/settings.png" alt="settings" width="250" border="1px"/></a>
<a style="vertical-align: top" href="images/update.png"  ><img src="images/update.png"   alt="update"   width="250" border="1px"/></a>
</p>
