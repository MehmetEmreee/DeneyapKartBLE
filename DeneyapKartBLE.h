#pragma once
#include <Arduino.h>
#include "sdkconfig.h"
#include <functional>

// Otomatik stack seçimi
#if !defined(USE_NIMBLE)
  #if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32S3)
    #define USE_NIMBLE
  #else
    #if defined(__has_include)
      #if __has_include(<NimBLEDevice.h>)
        #define USE_NIMBLE
      #endif
    #endif
  #endif
#endif

#if defined(USE_NIMBLE)
  #include <NimBLEDevice.h>
  #define DKB_BLEDevice NimBLEDevice
  #define DKB_BLEServer NimBLEServer
  #define DKB_BLEService NimBLEService
  #define DKB_BLECharacteristic NimBLECharacteristic
  #define DKB_BLEAdvertising NimBLEAdvertising
  #define DKB_BLEServerCallbacks NimBLEServerCallbacks
  #define DKB_BLECharacteristicCallbacks NimBLECharacteristicCallbacks
#else
  #include <BLEDevice.h>
  #include <BLEServer.h>
  #include <BLEUtils.h>
  #include <BLE2902.h>
  #define DKB_BLEDevice BLEDevice
  #define DKB_BLEServer BLEServer
  #define DKB_BLEService BLEService
  #define DKB_BLECharacteristic BLECharacteristic
  #define DKB_BLEAdvertising BLEAdvertising
  #define DKB_BLEServerCallbacks BLEServerCallbacks
  #define DKB_BLECharacteristicCallbacks BLECharacteristicCallbacks
#endif

namespace DeneyapKartBLE {
static const char* DEFAULT_SERVICE_UUID = "12345678-1234-1234-1234-1234567890ab";
static const char* DEFAULT_TX_CHAR_UUID = "12345678-1234-1234-1234-1234567890ac";
static const char* DEFAULT_RX_CHAR_UUID = "12345678-1234-1234-1234-1234567890ad";

using RxCallback = std::function<void(const uint8_t*, size_t)>;
using ConnectCallback = std::function<void()>;
using DisconnectCallback = std::function<void()>;

struct Config {
  String deviceName = "DeneyapKart";
  String serviceUUID = DEFAULT_SERVICE_UUID;
  String txCharUUID = DEFAULT_TX_CHAR_UUID; // notify
  String rxCharUUID = DEFAULT_RX_CHAR_UUID; // write
  uint16_t mtu = 0;
};

class Core : public DKB_BLEServerCallbacks, public DKB_BLECharacteristicCallbacks {
public:
  bool begin(const Config& cfg = Config());
  void end();
  bool send(const uint8_t* data, size_t len, bool wait=false);
  bool send(const String& s){return send((const uint8_t*)s.c_str(), s.length());}
  bool send(const char* c){return send((const uint8_t*)c, strlen(c));}
  bool restartAdvertising();

  void onReceive(RxCallback cb){_rxCb=std::move(cb);} 
  void onConnect(ConnectCallback cb){_connectCb=std::move(cb);} 
  void onDisconnect(DisconnectCallback cb){_disconnectCb=std::move(cb);} 

  void enableLineMode(bool en=true){_lineMode=en; if(en) _lineBuf.reserve(128);} 
  void enableLineCommandMode(bool en=true){ enableLineMode(en); }
  bool isConnected() const {return _connected;} 
  void printInfo(Stream &s=Serial) const;
  const char* getStackName() const { return _usingNimble ? "NimBLE" : "Bluedroid"; }
  uint16_t getEffectiveMTU() const { return _mtuEff; }

#if defined(USE_NIMBLE)
  void onConnect(DKB_BLEServer* s, NimBLEConnInfo& info) override;
  void onDisconnect(DKB_BLEServer* s, NimBLEConnInfo& info, int reason) override;
  void onWrite(DKB_BLECharacteristic* c, NimBLEConnInfo& info) override;
#else
  void onConnect(DKB_BLEServer* s) override;
  void onDisconnect(DKB_BLEServer* s) override;
  void onWrite(DKB_BLECharacteristic* c) override;
#endif

private:
  Config _cfg;
  DKB_BLEServer* _server=nullptr; DKB_BLEService* _service=nullptr; 
  DKB_BLECharacteristic* _tx=nullptr; DKB_BLECharacteristic* _rx=nullptr; DKB_BLEAdvertising* _adv=nullptr; 
  RxCallback _rxCb; ConnectCallback _connectCb; DisconnectCallback _disconnectCb; 
  bool _lineMode=false; String _lineBuf; bool _connected=false; uint16_t _mtuEff=20; 
  const bool _usingNimble =
#ifdef USE_NIMBLE
    true;
#else
    false;
#endif
};
}

