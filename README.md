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
  <li>Arduino 1.8.9</li>
  <li><a href="https://github.com/esp8266/Arduino">Arduino Core for ESP8266 2.5.0</a></li>
</ul>

<h2>Setup</h2>
<ol>
  <li>Open the Arduino IDE</li>
  <li>Install the required libraries</li>
  <li>Check the <a href="source/ArduinoSettings.png">Board Settings</a></li>
  <li>Modify the password.h file, the password is used in AP mode and as OTA update password</li>
  <li>Load the Clock.ino sketch</li>
  <li>Flash the ESP8266 using Arduino IDE</li>
</ol>

<h2>First Start</h2>
<ul>
  <li>Open the Serial Monitor in Arduino IDE</li>
  <li>Power on / reset the Wifi Socket</li>
  <li>Connect to Network with SSID = "WifiSocket-xxxxxx" and PSK you defined above</li>
  <li>Open the web page "http://192.168.4.1" (or sometimes 192.168.244.1)</li>
  <li>On the Settings page enter your WiFi credentials (SSID and PSK) and press "save"</li>
  <li>The Wifi Socket will now connect to the network</li>
  <li>At next power on the Wifi Socket will remember the settings</li>
</ul>

<h2>Attaching the Wifi Socket to the grid</h2>
<p>The Wifi Socket always start in "Off" position.</p>

<h2>Using the hardware buttons</h2>
<p>Reset - Restart the Wifi Socket</p>
<p>On/Off - Rotate from OFF to "Time Controlled" to ON</p>

<h2>LEDs</h2>
<ul>
  <li>Red ON: Socket has power</li>
  <li>Red OFF: Socket has no power</li>
  <li>Green OFF: manual mode</li>
  <li>Green ON: Time Controlled mode</li>
  <li>Green blink: Time Controlled mode ERROR</li>
</ul>


<h2>UI Screenshots</h2>
<p>
  ...
</p>
