////////////////////////////////////////////////////////////////////////////////
// EspObiSocket.h
////////////////////////////////////////////////////////////////////////////////

extern char HostName[] ;

extern const uint8_t IoBtnOnOff ;
extern const uint8_t IoLedGreen ;
extern const uint8_t IoLedRed ;
extern const uint8_t IoRelay ;

////////////////////////////////////////////////////////////////////////////////

class HttpBuff ;

////////////////////////////////////////////////////////////////////////////////

class Relay
{
 public:
  enum class State
  {
    Off,
    On
  } ;
  
  Relay(uint8_t pin, uint8_t pinLed) ;
  ~Relay() ;

  State on() ;
  State off() ;
  State toggle() ;
  State set(State state) ;
  State get() const ;
  
private:
  uint8_t _pin    = IoRelay ;
  uint8_t _pinLed = IoLedRed ;
  State   _state  = State::Off ;
} ;

////////////////////////////////////////////////////////////////////////////////

enum class Lang
{
  EN,
  DE,
  Size // last
} ;

enum class State
{
  Off,
  On,
} ;

struct Settings
{
  Settings() = default ;
  Settings(const Settings &) = delete ;
  Settings& operator=(const Settings &) = delete ;

  void load() ;
  void save() const ;

  // Language
  Lang _lang { Lang::DE } ;

  // Wifi Access Point
  String _apSsid ;
  String _apPsk  ;
  int    _apChan ;

  // Wifi Station
  String _ssid ;
  String _psk ;

  // NTP
  String    _ntp { "pool.ntp.org" } ;
  TZ::Month _tzDstMonth  { TZ::Month:: Mar } ;
  TZ::Week  _tzDstWeek   { TZ::Week:: Last } ;
  TZ::Day   _tzDstDay    { TZ::Day::   Sun } ;
  uint8_t   _tzDstHour   {               2 } ;
  int16_t   _tzDstOffset {             120 } ;
  TZ::Month _tzStdMonth  { TZ::Month:: Oct } ;
  TZ::Week  _tzStdWeek   { TZ::Week:: Last } ;
  TZ::Day   _tzStdDay    { TZ::Day::   Sun } ;
  uint8_t   _tzStdHour   {               3 } ;
  int16_t   _tzStdOffset {              60 } ;

} ;

////////////////////////////////////////////////////////////////////////////////

class Ntp
{
#pragma pack(push, 1)
  struct NtpData
  {
    uint8_t liVnMode ;        // 0 [2:3:3]
    uint8_t stratum ;
    uint8_t poll ;
    uint8_t precision ;
    uint32_t rootDelay ;      // 4 
    uint32_t rootDispersion ;
    uint32_t referenceId ;
    uint32_t referenceTsSec ; // 16
    uint32_t referenceTsFrac ;
    uint32_t originTsSec ;    // 24
    uint32_t originTsFrac ;
    uint32_t receiveTsSec ;   // 32
    uint32_t receiveTsFrac ;
    uint32_t transmitTsSec ;  // 40
    uint32_t transmitTsFrac ;
  } ;
#pragma pop

  enum class State
  {
    off     = 0,
    wait    = 1,
    success = 2,
    force   = 3,
  } ;

public:
  void     start() ; // enable transmitting ntp messages
  void     stop() ;  // disable transmitting ntp messages
  
  bool     valid()  const { return _valid ; } // a time was set (manually or by NTP)
  bool     isNtp()  const { return _isNtp ; } // time was received via NTP
  bool     inc() ; // time changed
  uint64_t local() const ; // current local time
  void     setLocal(uint64_t local) ;
  
  void printSerial(const NtpData &data) const ;

  String toLocalString() const ;
  bool   fromLocalString(const String &str) ;
  bool   isSync() const ; // last succesful sync is not too long ago
  String lastSyncToString() const ;
  
  static uint16_t port() { return 123 ; }
  static uint16_t size() { return sizeof(NtpData) ; }
  bool tx(WiFiUDP &udp, const String &ntpServer) ;
  bool rx(WiFiUDP &udp) ;

private:
  bool fromLocalString1(const String &s, uint8_t offset, uint8_t &v) const ;

private:
  static const int _delayWait    =      15 * 1000 ; // 15 sec
  static const int _delaySuccess = 30 * 60 * 1000 ; // 30 min
  
  static const int _delayWarnSyncNtp    =       6 * 60 * 60 ; // 3 hour
  static const int _delayWarnSyncManual =  7 * 24 * 60 * 60 ; // 1 week
  
  NtpData _txData ;
  NtpData _rxData ;

  State    _state       = State::off ;
  bool     _valid       = false ;
  bool     _next59      = false ;
  bool     _next61      = false ;
  uint32_t _lastInc     = 0 ;     // last millis when inc'ed current time
  uint32_t _lastRequest = 0 ;     // millis of last request
  uint64_t _tsReceived  = 0 ;     // last received ntp time
  uint64_t _tsTransmit  = 0 ;     // last tx timestamp
  bool     _isNtp       = false ; // NTP vs manual setting (== utc time vs local time)
  uint64_t _current     = 0 ;     // current utc or local time
  uint64_t _lastSync    = 0 ;     // last sync (ntp or manual)
  
} ;

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
