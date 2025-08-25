#include "DeneyapKartBLE.h"
using namespace DeneyapKartBLE;

bool Core::begin(const Config& cfg){
  _cfg=cfg; if(!_cfg.deviceName.length()) return false; DKB_BLEDevice::init(_cfg.deviceName.c_str());
#if defined(USE_NIMBLE)
  if(cfg.mtu) NimBLEDevice::setMTU(cfg.mtu);
#endif
  _server = DKB_BLEDevice::createServer(); _server->setCallbacks(this);
  _service = _server->createService(_cfg.serviceUUID.c_str());
#if defined(USE_NIMBLE)
  _tx = _service->createCharacteristic(_cfg.txCharUUID.c_str(), NIMBLE_PROPERTY::NOTIFY);
  _rx = _service->createCharacteristic(_cfg.rxCharUUID.c_str(), NIMBLE_PROPERTY::WRITE|NIMBLE_PROPERTY::WRITE_NR);
#else
  _tx = _service->createCharacteristic(_cfg.txCharUUID.c_str(), BLECharacteristic::PROPERTY_NOTIFY);
  _rx = _service->createCharacteristic(_cfg.rxCharUUID.c_str(), BLECharacteristic::PROPERTY_WRITE|BLECharacteristic::PROPERTY_WRITE_NR);
  _tx->addDescriptor(new BLE2902());
#endif
  _rx->setCallbacks(this);
  _service->start();
  _adv = DKB_BLEDevice::getAdvertising();
#if defined(USE_NIMBLE)
  NimBLEAdvertisementData adv; adv.setName(_cfg.deviceName.c_str()); adv.addServiceUUID(_cfg.serviceUUID.c_str()); adv.setFlags(0x06);
  _adv->setAdvertisementData(adv);
  NimBLEAdvertisementData scan; scan.setName(_cfg.deviceName.c_str()); _adv->setScanResponseData(scan);
  _adv->start();
#else
  _adv->addServiceUUID(_cfg.serviceUUID.c_str()); _adv->setScanResponse(true); _adv->start();
#endif
  if(Serial){Serial.println(F("[DeneyapKartBLE] Basladi")); printInfo();}
  return true;
}

void Core::end(){ if(_adv) _adv->stop(); }

bool Core::send(const uint8_t* d,size_t l,bool wait){ if(!_connected||!_tx||!l)return false; size_t mtu = (_mtuEff>3?_mtuEff:20)-3; size_t off=0; while(off<l){ size_t c=l-off; if(c>mtu)c=mtu; 
#if defined(USE_NIMBLE)
  _tx->setValue(d+off,c); _tx->notify(true);
#else
  _tx->setValue((uint8_t*)d+off,c); _tx->notify();
#endif
  off+=c; if(wait) delay(5);} return true; }

void Core::printInfo(Stream &s) const { s.println(F("[DeneyapKartBLE] Info")); s.print(F("  Service:")); s.println(_cfg.serviceUUID); s.print(F("  TX:")); s.println(_cfg.txCharUUID); s.print(F("  RX:")); s.println(_cfg.rxCharUUID); s.print(F("  Conn:")); s.println(_connected?F("yes"):F("no")); s.print(F("  MTU:")); s.println(_mtuEff); }

bool Core::restartAdvertising(){ if(!_adv) return false; _adv->stop();
#if defined(USE_NIMBLE)
  return _adv->start();
#else
  _adv->start(); return true;
#endif
}
#if defined(USE_NIMBLE)
void Core::onConnect(DKB_BLEServer* s, NimBLEConnInfo& info){ (void)s; _connected=true; _mtuEff=s->getPeerMTU(info.getConnHandle()); if(Serial) Serial.println(F("[DeneyapKartBLE] Connected")); if(_connectCb)_connectCb(); }
void Core::onDisconnect(DKB_BLEServer* s, NimBLEConnInfo& info, int reason){ (void)s;(void)info;(void)reason; _connected=false; _mtuEff=20; if(Serial) Serial.println(F("[DeneyapKartBLE] Disconnected")); if(_disconnectCb)_disconnectCb(); if(_adv)_adv->start(); }
void Core::onWrite(DKB_BLECharacteristic* c, NimBLEConnInfo& info){ (void)info; auto raw=c->getValue(); std::string v(raw.c_str()); if(v.empty()) return; if(_lineMode){ for(char ch: v){ if(ch=='\n'){ if(_rxCb) _rxCb((const uint8_t*)_lineBuf.c_str(), _lineBuf.length()); _lineBuf=""; } else if(ch!='\r'){ _lineBuf+=ch; } } } else { if(_rxCb) _rxCb((const uint8_t*)v.data(), v.length()); } }
#else
void Core::onConnect(DKB_BLEServer* s){ (void)s; _connected=true; if(Serial) Serial.println(F("[DeneyapKartBLE] Connected")); if(_connectCb)_connectCb(); }
void Core::onDisconnect(DKB_BLEServer* s){ (void)s; _connected=false; _mtuEff=20; if(Serial) Serial.println(F("[DeneyapKartBLE] Disconnected")); if(_disconnectCb)_disconnectCb(); if(_adv)_adv->start(); }
void Core::onWrite(DKB_BLECharacteristic* c){ auto raw=c->getValue(); std::string v(raw.c_str()); if(v.empty()) return; if(_lineMode){ for(char ch: v){ if(ch=='\n'){ if(_rxCb) _rxCb((const uint8_t*)_lineBuf.c_str(), _lineBuf.length()); _lineBuf=""; } else if(ch!='\r'){ _lineBuf+=ch; } } } else { if(_rxCb) _rxCb((const uint8_t*)v.data(), v.length()); } }
#endif

// Eski isim için minimal geçiş mesajı (isteğe bağlı) yok - sessiz uyumluluk.
