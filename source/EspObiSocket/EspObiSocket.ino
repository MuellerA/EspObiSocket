////////////////////////////////////////////////////////////////////////////////
// EspObiSocket.ino
////////////////////////////////////////////////////////////////////////////////

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiUdp.h>
#include <FS.h>

#include <map>
#include <functional>

#include "timezone.h"
#include "EspObiSocket.h"

////////////////////////////////////////////////////////////////////////////////

char HostName[32] ;

const uint8_t IoBtnOnOff =  5 ;
const uint8_t IoLedGreen = 12 ;
const uint8_t IoLedRed   = 13 ;
const uint8_t IoRelay    =  4 ;

ESP8266WebServer httpServer ( 80 ) ;
ESP8266HTTPUpdateServer httpUpdater;

const uint16_t UDP_PORT = 1123 ;
WiFiUDP udp ;
Ntp ntp ;

Settings settings ;

bool WifiConnected = false ;

TZ::TimeZone tz ;

Relay relay(IoRelay, IoLedRed) ;

////////////////////////////////////////////////////////////////////////////////
// util

bool ascDec2bin(char c, uint8_t &d)
{
  if (('0' <= c) && (c <= '9')) { d = c - '0'      ; return true ; }
  return false ;
}

bool ascHex2bin(char c, uint8_t &h) 
{
  if (('0' <= c) && (c <= '9')) { h = c - '0'      ; return true ; }
  if (('a' <= c) && (c <= 'f')) { h = c - 'a' + 10 ; return true ; }
  if (('A' <= c) && (c <= 'F')) { h = c - 'A' + 10 ; return true ; }
  return false ;
}

template<typename T>
bool ascInt2bin(String str, T &val)
{
  if (str.length() > 5)
    return false ;

  uint8_t e = str.length() ;
  if (e < 1)
    return false ;

  uint8_t idx = 0 ;
  bool neg = false ;

  if      (str[0] == '-') { neg = true ; idx = 1 ; }
  else if (str[0] == '+') {              idx = 1 ; }

  if (e < (idx + 1))
    return false ;
  
  T v = 0 ;
  for ( ; idx < e ; ++idx)
  {
    uint8_t d ;
    if (!ascDec2bin(str[idx], d))
      return false ;
    v = v*10 + d ;
  }
  val = neg ? -v : v ;

  return true ;
}

template<typename T>
bool ascInt2bin(String str, T &val, T min, T max)
{
  T val0 ;
  if (ascInt2bin(str, val0) &&
      (min <= val0) && (val0 <= max))
  {
    val = val0 ;
    return true ;
  }
  return false ;
}

String timeToString(uint32_t t)
{
  char buff[16] ;
  sprintf(buff, "%02ld:%02ld:%02ld", (long)(t / 60 / 60 % 24), (long)(t / 60 % 60), (long)(t % 60)) ;
  return String(buff) ;
}

uint32_t stringToTime(const String &str)
{
  long h,m,s ;
  sscanf(str.c_str(), "%02ld:%02ld:%02ld", &h, &m, &s) ;
  return uint32_t(h*60*60 + m*60 + s) ;
}

////////////////////////////////////////////////////////////////////////////////
// Relay
////////////////////////////////////////////////////////////////////////////////

Relay::Relay(uint8_t pin, uint8_t pinLed) : _pin(pin), _pinLed(pinLed)
{
  pinMode(_pin, OUTPUT) ;
  pinMode(_pinLed, OUTPUT) ;
  off() ;
}

Relay::~Relay()
{
  off() ;
}

Relay::State Relay::on()
{
  Relay::State old = _state ;
  _state = Relay::State::On ;
  digitalWrite(_pin, HIGH) ;
  digitalWrite(_pinLed, HIGH) ;
  return old ;
}

Relay::State Relay::off()
{
  Relay::State old = _state ;
  _state = Relay::State::Off ;
  digitalWrite(_pin, LOW) ;
  digitalWrite(_pinLed, LOW) ;
  return old ;
}

Relay::State Relay::toggle()
{
  if (_state == Relay::State::Off)
    return on() ;
  return off() ;
}

Relay::State Relay::set(State state)
{
  if (state == Relay::State::Off)
    return on() ;
  return off() ;
}
  
Relay::State Relay::get() const
{
  return _state ;
}

////////////////////////////////////////////////////////////////////////////////

void update(uint64_t time)
{
}

void updateBtnOnOff(uint32_t currTick)
{
  // active low
  
  static uint32_t prevTick = 0 ;
  static uint8_t  prevVal  = HIGH ;

  if (currTick < (prevTick + 300))
    return ;

  uint8_t currVal = digitalRead(IoBtnOnOff) ;

  if ((currVal != prevVal) && (currVal == LOW))
  {
    relay.toggle() ;
  }
  
  prevTick = currTick - 300 ;
  prevVal  = currVal ;
}

////////////////////////////////////////////////////////////////////////////////
// Arduino Main
////////////////////////////////////////////////////////////////////////////////

void setup()
{
  pinMode(IoBtnOnOff, INPUT_PULLUP) ;
  pinMode(IoLedGreen, OUTPUT) ;
  
  sprintf(HostName, "WifiSocket-%06x", ESP.getChipId()) ;
  
  Serial.begin ( 115200 );
  Serial.printf("\n") ;
  //Serial.setDebugOutput(true) ;
  
  SPIFFS.begin() ;
  
  settings.load() ;
  Serial.printf("AP: SSID=%s / PSK=%s / Channel=%d\n", settings._apSsid.c_str(), settings._apPsk.c_str(), settings._apChan) ;
  
  WifiSetup() ;
  HttpSetup() ;
  
  udp.begin(UDP_PORT) ;
}

////////////////////////////////////////////////////////////////////////////////

void loop()
{
  uint32_t now = millis() ;
  
  int udpLen = udp.parsePacket() ;
  if (udpLen)
  {
    if ((udp.remotePort() == Ntp::port()) &&
        (udpLen == Ntp::size()))
      ntp.rx(udp) ;
  }
  
  WifiLoop() ;

  // Update
  {
    static uint32_t last = 0 ;

    if (ntp.valid())
    {
      digitalWrite(IoLedGreen, HIGH) ;
      last = now ;

      if (ntp.inc())
      {
        uint64_t local = ntp.local() ;
        update(local) ;
      }
    }
    else
    {
      static uint32_t val = LOW ;
      if (now >= (lastVal + 500))
      {
        val = (val == LOW) ? HIGH : LOW ;
        digitalWrite(IoLedGreen, val) ;
        last = now ;
      }
    }
  }

  updateBtnOnOff(now) ;
  
  // NTP
  if (WifiConnected)
    ntp.tx(udp, settings._ntp) ;
  
  httpServer.handleClient() ;

  delay(50) ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
