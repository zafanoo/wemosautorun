#include <ESP8266WiFi.h>
#include "TM1637Display.h"
#include <espnow.h>
#include <EEPROM.h>
#define btn1 D1
#define btn2 D2
#define led1 D4
#define CLK D5//pins definitions for TM1637 and can be changed to other ports       
#define DIO D6
#define EEPROM_SIZE 12

uint8_t broadcastAddress1[] = {0xF4, 0xCF, 0xA2, 0x77, 0x29, 0xA8};
//#define led1 D4
typedef struct {
    char message[32];  // Pesan teks yang akan dikirim
} struct_message;

struct_message myData;
bool balasan = false;
bool balasan2 = false;
bool sensor = false;

TM1637Display display(CLK,DIO);
// Alamat MAC perangkat 2
//uint8_t broadcastAddress2[] = {0x84, 0xF3, 0xEB, 0x3B, 0x69, 0x04}; 
uint8_t broadcastAddress2[] = {0x5C, 0xCF, 0x7F, 0x18, 0xA8, 0x66}; 
 // Ganti sesuai dengan MAC perangkat 2

// Callback untuk data diterima
void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
    struct_message receivedData;
    memcpy(&receivedData, incomingData, sizeof(receivedData));
    Serial.print("Pesan diterima: ");
    Serial.println(receivedData.message);

    if (strcmp(receivedData.message, "mesin sudah hidup") == 0) {
        balasan = true; 
    }
    if (strcmp(receivedData.message, "mesin selesai") == 0) {
        balasan2 = true; 
    }
    if (strcmp(receivedData.message, "mesin lock") == 0) {
        sensor = true;
    }else {
    sensor = false;
    } 
    
}

// Callback untuk data terkirim
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
    Serial.println(sendStatus == 0 ? "Pengiriman berhasil" : "Pengiriman gagal");
}
unsigned int angka;
void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    pinMode(btn1, INPUT_PULLUP);
    pinMode(btn2, INPUT_PULLUP);
    //pinMode(BUILTIN_LED, OUTPUT);
    pinMode(led1,OUTPUT);
    //digitalWrite(BUILTIN_LED, HIGH);

  Serial.println(F("Initialize System"));
  EEPROM.begin(EEPROM_SIZE);
  int address = 0;
  int konter = +1;

  EEPROM.get (address, angka);
  Serial.println(angka);
  display.setBrightness(3);
  display.showNumberDecEx(angka, 0x40, true);
  delay(2000);


    if (esp_now_init() != 0) {
        Serial.println("Gagal menginisialisasi ESP-NOW");
        return;
    }

    esp_now_set_self_role(ESP_NOW_ROLE_COMBO); // Sebagai pengirim dan penerima
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);

    // Tambahkan peer
    esp_now_add_peer(broadcastAddress2, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
}
    //  balasan2 && 
void loop() {
    if (digitalRead(btn1) == 0 && balasan2 ){
    strcpy(myData.message, "hidupkan mesin");
    balasan2 = false;

    angka =angka+1;
    balasan2 = false;
  //kondisi = !kondisi;
  int address = 0;
  int konter = +1;
  EEPROM.put(address, angka);
  EEPROM.commit();
  display.showNumberDecEx(angka, 0x40, true);
   // Isi pesan
    esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData)); // Kirim pesan
    delay(2000); 
     // Interval pengiriman pesan
    }
    if (sensor == true) {
      
        // Jalankan aksi ketika pesan "mesin lock" diterima
        digitalWrite(led1, HIGH); 
        balasan2 = false;// Nyalakan LED pada pin led1
        }
        else {
        digitalWrite(led1, LOW);
        balasan2 = true;
        // Matikan LED setelah 1 detik
    
}
if (digitalRead(btn2) == 0){
  angka = 0;
  //kondisi = !kondisi;
  int address = 0;
  int konter = 0;
  EEPROM.put(address, angka);
  EEPROM.commit();
  display.showNumberDecEx(angka, 0x40, true);
  delay(1000);
}
if (sensor  && !balasan){
   strcpy(myData.message, "sensor on");
   esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData)); // Kirim pesan
    delay(2000); 
  
  delay(1000);
}
    
}
