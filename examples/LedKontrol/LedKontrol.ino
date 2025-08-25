#include <DeneyapKartBLE.h>
using namespace DeneyapKartBLE;

#ifndef LED_PIN
 #if defined(LED_BUILTIN)
  #define LED_PIN LED_BUILTIN
 #else
  #define LED_PIN 2
 #endif
#endif

Core core;

static void handleCommand(const uint8_t* data,size_t len){
  String cmd; cmd.reserve(len);
  for(size_t i=0;i<len;i++){
    char c = (char)data[i];
    if(c=='\r' || c=='\n') continue;
    cmd += (char)toupper((unsigned char)c);
  }
  if(cmd.length()==0) return;
  if(cmd.equals("AC")){
    digitalWrite(LED_PIN, HIGH);
    core.send("OK AC\n");
  } else if(cmd.equals("KAPAT")){
    digitalWrite(LED_PIN, LOW);
    core.send("OK KAPAT\n");
  } else if(cmd.equals("DURUM")){
    core.send(String("DURUM=") + (digitalRead(LED_PIN)?"1\n":"0\n"));
  } else {
    core.send("HATA BILINMEYEN\n");
  }
}

void setup(){
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Config cfg; cfg.deviceName = "DeneyapKartBLE_LED";
  core.begin(cfg);
  core.enableLineMode(false);
  core.onReceive(handleCommand);
  Serial.println(F("Komutlar: AC / KAPAT / DURUM"));
}

void loop(){ }
