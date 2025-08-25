# DeneyapKartBLE

Deneyap Kartlar ve ESP32 ailesi (ESP32, ESP32-C3, ESP32-S3) üzerinde hem klasik Bluedroid hem de NimBLE yığınlarını otomatik seçerek çalışan hafif ve tek tip (unified) BLE haberleşme kütüphanesi. Notify (TX) + Write/WriteNR (RX) karakteristik çifti ile basit, düşük gecikmeli, seri benzeri ikili veya metin veri aktarımı sağlar.

---
## ✨ Özellikler
- Otomatik BLE stack seçimi: (C3 / S3 → NimBLE) + (Klasik ESP32 → Bluedroid, NimBLE varsa onu tercih eder)
- Tek API ile begin / send / receive callback modeli
- MTU tespiti ve parçalama (chunk) – uzun mesajları otomatik böler
- Opsiyonel satır (line) modu: '\n' ile biten komutları tamponlar ve callback'e bütün blok olarak verir
- Bağlantı / MTU / servis bilgisi debug çıktısı (`printInfo()`)
- Hızlı yeniden reklam başlatma (`restartAdvertising()`)
- Geriye dönük namespace ve header alias desteği

## 🧩 Mimari
```
Service UUID   (varsayılan: 12345678-1234-1234-1234-1234567890ab)
	├─ TX Characteristic (Notify)  UUID ...90ac
	└─ RX Characteristic (Write / Write Without Response)  UUID ...90ad
```
Host (telefon / PC) RX karakteristiğine yazar, kütüphane veriyi callback ile iletir; siz de `send()` ile TX üzerinden notify yayınlarsınız.

## 🛠 Kurulum
### Arduino IDE (manuel)
1. Bu depoyu ZIP olarak indirin veya klonlayın.
2. Klasörü (adı `DeneyapKartBLE` olacak şekilde) `Documents/Arduino/libraries/` altına yerleştirin.
3. IDE'yi yeniden başlatın. Örnekler menüsünde `DeneyapKartBLE` görünecektir.

## 🚀 Hızlı Başlangıç
```cpp
#include <DeneyapKartBLE.h>
using namespace DeneyapKartBLE;

Core ble;

void setup(){
	Serial.begin(115200);
	Config cfg;            // İsteğe göre özelleştir
	cfg.deviceName = "DeneyapKartBLE";
	ble.begin(cfg);

	ble.onReceive([](const uint8_t* data, size_t len){
		// Echo
		ble.send(data, len);
	});
}

void loop(){ /* uygulama işiniz */ }
```

## 📥 API Özeti
| Fonksiyon | Açıklama |
|-----------|----------|
| `bool begin(const Config&)` | BLE başlatır, servis & karakteristikleri oluşturur, reklamı başlatır. |
| `void end()` | Reklamı durdurur (bağlantıyı sonlandırır). |
| `bool send(const uint8_t*, size_t, bool wait=false)` | Veriyi MTU'ya göre parçalayıp notify gönderir. |
| `void onReceive(RxCallback)` | RX (write) geldiğinde çağrılacak callback. |
| `void onConnect(ConnectCallback)` | Bağlanınca çağrılır. |
| `void onDisconnect(DisconnectCallback)` | Ayrılınca çağrılır (reklam otomatik tekrar başlar). |
| `void enableLineMode(bool)` | Satır modu aç/kapat. |
| `bool restartAdvertising()` | Reklamı hızlı şekilde yeniden başlat. |
| `void printInfo(Stream&)` | Servis/char UUID ve MTU bilgisi basar. |
| `uint16_t getEffectiveMTU()` | Negotiated MTU değeri (paket başlığı hariç değil). |
| `const char* getStackName()` | "NimBLE" veya "Bluedroid" döner. |

### Config Alanları
| Alan | Varsayılan | Açıklama |
|------|-----------|----------|
| `deviceName` | `"DeneyapKart"` | Reklamlanan cihaz adı |
| `serviceUUID` | Varsayılan servis UUID | Değiştirilebilir |
| `txCharUUID` | Varsayılan TX UUID | Notify karakteristiği |
| `rxCharUUID` | Varsayılan RX UUID | Write / WriteNR karakteristiği |
| `mtu` | `0` | 0 → otomatik; >0 ise (NimBLE) setMTU denenir |

## 📝 Örnekler
| Örnek | Amaç |
|-------|------|
| `SeriHaberlesme` | Terminalden geleni aynen geri gönderir (echo) |
| `LedKontrol` | AC / KAPAT / DURUM komutları ile LED kontrolü |

## 🔡 Satır (Line) Modu
`enableLineMode(true)` aktifken gelen karakterler '\n' görülene kadar tamponlanır; callback tek seferde tam satır alır. Komut işleyici yazmak için uygundur. Satır sonu olmadan çok uzun veri gönderirseniz tampon büyümesi siz yönetirsiniz (varsayılan 128 başta rezerve edilir, gerekirse `String` büyür).

## 📦 MTU ve Parçalama (Chunking)
`send()` otomatik olarak (MTU - 3) uzunluğunda parçalara böler. `wait=true` derseniz her parça arası küçük gecikme (`delay(5)`) ekler; yüksek hacimli aktarımda akış kontrolüne yardımcı olur.

## 🔄 Yığın (Stack) Seçimi
- Derleyicide C3 / S3 hedefi → doğrudan NimBLE
- Aksi halde `__has_include(<NimBLEDevice.h>)` ile NimBLE bulunursa onu kullanır, yoksa Bluedroid

## 🧪 Test / Debug İpuçları
- Bağlandıktan hemen sonra `printInfo()` çağırarak MTU ve UUID doğrulayıp host uygulamanızda enable notifications yapın.
- Veri gelmiyorsa: (1) RX karakteristiğine WRITE yapıldığından emin olun. (2) Notify enable edilmedi ise TX görünmez. (3) Satır modu açıksa '\n' göndermeyi unutmayın.


## ❓ SSS
**Veri gelmiyor, neden?** Genellikle notify enable edilmemiştir veya line mode açıkken '\n' göndermiyorsunuzdur.

**MTU neden 23 görünüyor?** Bazı host uygulamaları (özellikle iOS) negotiation yapmadan kalabilir; NimBLE ile manuel `cfg.mtu` ayarlamayı deneyin.

## 📄 Lisans
MIT

---
Herhangi bir ek özellik isteğiniz varsa issue açın veya katkıda bulunun.

