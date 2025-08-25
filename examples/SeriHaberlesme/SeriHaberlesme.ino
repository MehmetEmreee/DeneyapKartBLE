#include <DeneyapKartBLE.h>
using namespace DeneyapKartBLE;

Core core;

void setup(){
  Serial.begin(115200);
  Config cfg; cfg.deviceName = "DeneyapKartBLE"; 
  core.begin(cfg);
  core.enableLineMode(false);
  core.onReceive([](const uint8_t* d,size_t l){
    Serial.print("RX: ");
    for(size_t i=0;i<l;i++) Serial.print((char)d[i]);
    Serial.println();
    core.send(d,l); 
  });
}

void loop(){ }
