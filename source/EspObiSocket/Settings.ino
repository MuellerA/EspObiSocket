////////////////////////////////////////////////////////////////////////////////
// Settings
////////////////////////////////////////////////////////////////////////////////

#define SettingsMagic    "WiFi Socket Settings V0.1"
#define SettingsFileName "/wifi-socket.settings"

String fileRead(File &cfg)
{
  String str = cfg.readStringUntil('\n') ;
  str.trim() ;
  return str ;
}

void Settings::load()
{
  Serial.print("Settings::load: ") ;

  _apSsid = "WiFiSocket" ;
  _apPsk  = HostName ;
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
    
    _ssid = fileRead(cfg) ;
    _psk  = fileRead(cfg) ;
    _ntp  = fileRead(cfg) ;

    uint8_t tmp ;
    ascInt2bin(fileRead(cfg), tmp         ) ; _tzDstMonth = (TZ::Month)tmp ;
    ascInt2bin(fileRead(cfg), tmp         ) ; _tzDstWeek  = (TZ::Week) tmp ;
    ascInt2bin(fileRead(cfg), tmp         ) ; _tzDstDay   = (TZ::Day)  tmp ;
    ascInt2bin(fileRead(cfg), _tzDstHour  ) ;
    ascInt2bin(fileRead(cfg), _tzDstOffset) ;
    ascInt2bin(fileRead(cfg), tmp         ) ; _tzStdMonth = (TZ::Month)tmp ;
    ascInt2bin(fileRead(cfg), tmp         ) ; _tzStdWeek  = (TZ::Week) tmp ;
    ascInt2bin(fileRead(cfg), tmp         ) ; _tzStdDay   = (TZ::Day)  tmp ;
    ascInt2bin(fileRead(cfg), _tzStdHour  ) ;
    ascInt2bin(fileRead(cfg), _tzStdOffset) ;

    ascInt2bin(fileRead(cfg), tmp) ; _lang = (Lang)tmp ;

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

  // magic aendern!
  
  cfg.close() ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
