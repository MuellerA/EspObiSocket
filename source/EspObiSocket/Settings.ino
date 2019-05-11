////////////////////////////////////////////////////////////////////////////////
// Settings
////////////////////////////////////////////////////////////////////////////////

#define SettingsMagic    "WiFi Socket Settings V0.2"
#define SettingsFileName "/wifi-socket.settings"

bool Settings::State::operator<(const State &that) const
{
  if (this->_enable    < that._enable   ) return false ; // enableder
  if (this->_enable    > that._enable   ) return true  ;
  if (this->_daySecond < that._daySecond) return true  ; // earlier
  if (this->_daySecond > that._daySecond) return false ;
  if (this->_weekDay   < that._weekDay  ) return false ; // mondayer
  if (this->_weekDay   > that._weekDay  ) return true  ;
  if (this->_state     < that._state    ) return false ; // onner
  if (this->_state     > that._state    ) return true  ;
  return false ;
}

String fileRead(File &cfg)
{
  String str = cfg.readStringUntil('\n') ;
  str.trim() ;
  return str ;
}

void Settings::load()
{
  Serial.print("Settings::load: ") ;

  char ssid[32] ;
  sprintf(ssid, "WiFiSocket-%06x", ESP.getChipId()) ;
  _apSsid = ssid ;
  _apPsk  = PASSWORD ;
  _apChan = 8 ;

  String str ;
  File cfg = SPIFFS.open(SettingsFileName, "r") ;

  if (!cfg)
  {
    Serial.println("file not found") ;
    return ;
  }
  
  str = fileRead(cfg) ;
 
  if (str == SettingsMagic)
  {
    Serial.println("magic match") ;

    _apSsid = fileRead(cfg) ;
    ascIntToBin(fileRead(cfg), _apChan, (uint8_t)1, (uint8_t)14) ;
    
    _ssid = fileRead(cfg) ;
    _psk  = fileRead(cfg) ;
    _ntp  = fileRead(cfg) ;
 
    uint8_t tmp ;
    ascIntToBin(fileRead(cfg), tmp         ) ; _tzDstMonth = (TZ::Month)tmp ;
    ascIntToBin(fileRead(cfg), tmp         ) ; _tzDstWeek  = (TZ::Week) tmp ;
    ascIntToBin(fileRead(cfg), tmp         ) ; _tzDstDay   = (TZ::Day)  tmp ;
    ascIntToBin(fileRead(cfg), _tzDstHour  ) ;
    ascIntToBin(fileRead(cfg), _tzDstOffset) ;
    ascIntToBin(fileRead(cfg), tmp         ) ; _tzStdMonth = (TZ::Month)tmp ;
    ascIntToBin(fileRead(cfg), tmp         ) ; _tzStdWeek  = (TZ::Week) tmp ;
    ascIntToBin(fileRead(cfg), tmp         ) ; _tzStdDay   = (TZ::Day)  tmp ;
    ascIntToBin(fileRead(cfg), _tzStdHour  ) ;
    ascIntToBin(fileRead(cfg), _tzStdOffset) ;

    ascIntToBin(fileRead(cfg), tmp) ; _lang = (Settings::Lang)tmp ;

    uint32_t stateNum ;
    ascIntToBin(fileRead(cfg), stateNum) ;
    _stateNum = stateNum < _stateMax ? stateNum : _stateMax ;

    for (uint8_t i = 0 ; i < stateNum ; ++i)
    {
      uint8_t tmp2 ;
      State dummy ;
      State &state = (i < _stateMax) ? _states[i] : dummy ;
      ascIntToBin(fileRead(cfg), tmp2) ; state._enable = (bool) tmp2 ;
      ascIntToBin(fileRead(cfg), state._weekDay) ;
      ascIntToBin(fileRead(cfg), state._daySecond) ;
      ascIntToBin(fileRead(cfg), tmp2) ; state._state = (Relay::State) tmp2 ;
    }

    ascIntToBin(fileRead(cfg), tmp         ) ; _bootMode   = (Mode)     tmp ;
    
    // magic aendern!

    tz.resetRules() ;
    tz.addRule( { _tzDstMonth, _tzDstWeek, _tzDstDay, _tzDstHour, _tzDstOffset } ) ;
    tz.addRule( { _tzStdMonth, _tzStdWeek, _tzStdDay, _tzStdHour, _tzStdOffset } ) ;
  }
  else
  {
    Serial.println("magic mismatch") ;
  }

  if (_ntp.length())
    ntp.start() ;
  else
    ntp.stop() ;
 
  cfg.close() ;
}

void Settings::save() const
{
  Serial.println("Settings::save") ;

  File cfg = SPIFFS.open(SettingsFileName, "w") ;

  if (!cfg)
    return ;

  cfg.println(SettingsMagic) ;

  cfg.println(_apSsid) ;
  cfg.println(_apChan) ;
  
  cfg.println(_ssid) ;
  cfg.println(_psk) ;
  cfg.println(_ntp) ;
  
  cfg.println((uint8_t)_tzDstMonth ) ;
  cfg.println((uint8_t)_tzDstWeek  ) ;
  cfg.println((uint8_t)_tzDstDay   ) ;
  cfg.println(         _tzDstHour  ) ;
  cfg.println(         _tzDstOffset) ;
  cfg.println((uint8_t)_tzStdMonth ) ;
  cfg.println((uint8_t)_tzStdWeek  ) ;
  cfg.println((uint8_t)_tzStdDay   ) ;
  cfg.println(         _tzStdHour  ) ;
  cfg.println(         _tzStdOffset) ;

  cfg.println((uint8_t)_lang) ;

  cfg.println(_stateNum) ;
  for (uint8_t i = 0 ; i < _stateNum ; ++i)
  {
    const State &state = _states[i] ;
    cfg.println((uint8_t)state._enable        ) ;
    cfg.println(         state._weekDay       ) ;
    cfg.println(         state._daySecond     ) ;
    cfg.println((uint8_t)state._state         ) ;
  }

  cfg.println((uint8_t)_bootMode) ;

  // magic aendern!
  
  cfg.close() ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
