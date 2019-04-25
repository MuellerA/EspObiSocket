////////////////////////////////////////////////////////////////////////////////
// EspObiSocket.ino
////////////////////////////////////////////////////////////////////////////////

// HTTPS axTLS or BearSSL or none (= unsecured http)
#define HTTPS_None    0
#define HTTPS_axTLS   1
#define HTTPS_BearSSL 2
#define HTTPS HTTPS_None

#include <ESP8266WiFi.h>
#if (HTTPS == HTTPS_axTLS)
#  include <WiFiClientSecure.h>
#  include <ESP8266WebServerSecureAxTLS.h>
#elif (HTTPS == HTTPS_BearSSL)
#  include <WiFiClientSecure.h>
#  include <ESP8266WebServerSecureBearSSL.h>
#elif (HTTPS == HTTPS_None)
#  include <ESP8266WebServer.h>
#else
#error "unknown macro HTTPS "
#endif
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiUdp.h>
#include <FS.h>

#include <map>
#include <functional>
#include <algorithm>

#include "timezone.h"
#include "password.h"
#include "EspObiSocket.h"

////////////////////////////////////////////////////////////////////////////////

const uint8_t IoBtnOnOff =  5 ;
const uint8_t IoLedGreen = 12 ;
const uint8_t IoLedRed   = 13 ;
const uint8_t IoRelay    =  4 ;

#if HTTPS == HTTPS_axTLS
axTLS::ESP8266WebServerSecure httpServer ( 443 ) ;
#elif HTTPS == HTTPS_BearSSL
BearSSL::ESP8266WebServerSecure httpServer ( 443 ) ;
#else
ESP8266WebServer httpServer ( 80 ) ;
#endif
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

bool ascDecToBin(char c, uint8_t &d)
{
  if (('0' <= c) && (c <= '9')) { d = c - '0'      ; return true ; }
  return false ;
}

bool ascHexToBin(char c, uint8_t &h) 
{
  if (('0' <= c) && (c <= '9')) { h = c - '0'      ; return true ; }
  if (('a' <= c) && (c <= 'f')) { h = c - 'a' + 10 ; return true ; }
  if (('A' <= c) && (c <= 'F')) { h = c - 'A' + 10 ; return true ; }
  return false ;
}

template<typename T>
bool ascIntToBin(const String &str, T &val)
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
    if (!ascDecToBin(str[idx], d))
      return false ;
    v = v*10 + d ;
  }
  val = neg ? -v : v ;

  return true ;
}

template<typename T>
bool ascIntToBin(const String &str, T &val, T min, T max)
{
  T val0 ;
  if (ascIntToBin(str, val0) &&
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

Relay::State Relay::set(State state)
{
  if (state == Relay::State::On)
    return on() ;
  return off() ;
}
  
Relay::State Relay::get() const
{
  return _state ;
}

////////////////////////////////////////////////////////////////////////////////

static const uint32_t daySec = 24 * 60 * 60 ;
bool GetState(uint64_t time, Relay::State &relayState)
{
  uint32_t nowInWeek = (time + (3 * daySec)) % (7 * daySec) ; // utc 1.1.1970 is Thu, State table is Mon

  Settings::State *state = nullptr ;

  for (uint8_t wd = 0 ; wd < 7 ; ++wd) // Mon .. Sun
  {
    for (uint8_t iS = 0 ; iS < settings._stateNum ; ++iS) // states
    {
      Settings::State &ss = settings._states[iS] ;

      if (ss._enable && (ss._weekDay & (0x80 >> wd))) // enabled & valid for wd
      {
        uint32_t t = wd * daySec + ss._daySecond ;
        if (t > nowInWeek)
        {
          if (state)
          {
            relayState = state->_state ;
            return true ;
          }
          nowInWeek += 7 * daySec ;
        }
        else
        {
          state = &ss ;
        }
      }
    }
  }
  if (state)
  {
    relayState = state->_state ;
    return true ;
  }
  return false ;
}


void update(uint64_t time)
{
  Relay::State relayState ;

  if (GetState(time, relayState))
    relay.set(relayState) ;
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
    switch (settings._mode)
    {
    case Settings::Mode::On:
      settings._mode = Settings::Mode::Off ;
      relay.off() ;
      break ;
    case Settings::Mode::Off:
      settings._mode = Settings::Mode::Time ;
      break ;
    case Settings::Mode::Time:
      settings._mode = Settings::Mode::On ;
      relay.on() ;
      break ;
    }
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
  updateBtnOnOff(now) ;
  
  if (settings._mode == Settings::Mode::Time)
  {
    static uint32_t last = 0 ;

    if (ntp.valid())
    {
      digitalWrite(IoLedGreen, HIGH) ;
      last = now ;

      if (ntp.inc() && (settings._mode == Settings::Mode::Time))
      {
        uint64_t local = ntp.local() ;
        update(local) ;
      }
    }
    else
    {
      static uint32_t val = LOW ;
      if (now >= (last + 500))
      {
        val = (val == LOW) ? HIGH : LOW ;
        digitalWrite(IoLedGreen, val) ;
        last = now ;
      }
    }
  }
  else
  {
    if (ntp.valid())
      ntp.inc() ;
    digitalWrite(IoLedGreen, LOW) ;
  }

  // NTP
  if (WifiConnected)
    ntp.tx(udp, settings._ntp) ;
  
  httpServer.handleClient() ;

  delay(25) ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
