#include <ESP8266WiFi.h>
#include <espnow.h>
#include "macaddress1.h"


// Pin untuk LED dan Sensor
#define led1  D8
#define led2  D6
#define led3  D2
#define led4  D5
#define led5  D4
#define led6  D3
#define led8  D7
#define sensor D1

typedef struct {
    char message[32]; // Struktur pesan teks untuk komunikasi
} struct_message;

struct_message myData;
bool ledControl = false;  // Flag untuk kontrol LED
bool tombolSebelumnya = HIGH;

// Alamat MAC perangkat 1
//uint8_t broadcastAddress1[] = {0xF4, 0xCF, 0xA2, 0x77, 0x29, 0xA8};

// Callback untuk data diterima
void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
    struct_message receivedData;
    memcpy(&receivedData, incomingData, sizeof(receivedData));
    Serial.print("Pesan diterima: ");
    Serial.println(receivedData.message);

    if (strcmp(receivedData.message, "hidupkan mesin") == 0) {
        ledControl = true; // Aktifkan kontrol LED
    }
}

// Callback untuk data terkirim
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
    Serial.println(sendStatus == 0 ? "Pengiriman berhasil" : "Pengiriman gagal");
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    // Konfigurasi pin
    pinMode(led1, OUTPUT);
    pinMode(led2, OUTPUT);
    pinMode(led3, OUTPUT);
    pinMode(led4, OUTPUT);
    pinMode(led5, OUTPUT);
    pinMode(led6, OUTPUT);
    pinMode(led8, OUTPUT);
    pinMode(sensor, INPUT_PULLUP);

    // Inisialisasi ESP-NOW
    if (esp_now_init() != 0) {
        Serial.println("Gagal menginisialisasi ESP-NOW");
        return;
    }

    esp_now_set_self_role(ESP_NOW_ROLE_COMBO); // Perangkat sebagai pengirim & penerima
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);

    // Tambahkan peer
    esp_now_add_peer(macAddress1, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
}

void loop() {
    // Kontrol LED berdasarkan flag
    if (ledControl) {
        // Animasi nyala LED
        digitalWrite(led4, HIGH);
        delay(1000);
        digitalWrite(led4, LOW);
        delay(1800);
        digitalWrite(led3, HIGH);
        delay(500);
        digitalWrite(led3, LOW);
        delay(500);
         digitalWrite(led3, HIGH);
        delay(500);
        digitalWrite(led3, LOW);
        delay(1000);
         digitalWrite(led2, HIGH);
        delay(500);
        digitalWrite(led2, LOW);
        delay(500);
         digitalWrite(led2, HIGH);
        delay(500);
        digitalWrite(led2, LOW);
         delay(500);
         digitalWrite(led2, HIGH);
        delay(500);
        digitalWrite(led2, LOW);
        delay(1000);
        
        digitalWrite(led1, HIGH);
        delay(1200);
        digitalWrite(led1, LOW);
        delay(1000);
        digitalWrite(led8, HIGH);
        delay(3000);
        digitalWrite(led8, LOW);
        
        ledControl = false; // Reset flag
        strcpy(myData.message, "mesin sudah hidup");
        esp_now_send(macAddress1, (uint8_t *)&myData, sizeof(myData)); // Kirim pesan
    }

    // Deteksi tombol/sensor ditekan
    bool tombolSekarang = digitalRead(sensor);
    if (tombolSebelumnya == HIGH && tombolSekarang == LOW) {
        Serial.println("Sensor ditekan!");
        strcpy(myData.message, "mesin lock");
        esp_now_send(macAddress1, (uint8_t *)&myData, sizeof(myData));
        delay(200); // Hindari bouncing
    }

    // Deteksi tombol/sensor dilepas
    if (tombolSebelumnya == LOW && tombolSekarang == HIGH) {
        Serial.println("Sensor dilepas!");
        digitalWrite(led4, HIGH); delay(700);
        digitalWrite(led4, LOW);
        strcpy(myData.message, "mesin selesai");
        esp_now_send(macAddress1, (uint8_t *)&myData, sizeof(myData));
        delay(200); // Hindari bouncing
    }

    // Simpan status tombol untuk iterasi berikutnya
    tombolSebelumnya = tombolSekarang;
}
