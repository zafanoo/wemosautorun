#include <ESP8266WiFi.h>
#include "TM1637Display.h"
#include <espnow.h>
#include <EEPROM.h>

#define btn1 D1
#define btn2 D2
#define btn3 D3
#define led D4
#define CLK D5  // pins definitions for TM1637 and can be changed to other ports
#define DIO D6
#define EEPROM_SIZE 12

uint8_t broadcastAddress1[] = {0x84, 0xF3, 0xEB, 0x3B, 0x69, 0x04};//dryer
uint8_t broadcastAddress2[] = {0x5C, 0xCF, 0x7F, 0x18, 0xA8, 0x66};//washer

typedef struct {
    char message[32];  // Pesan teks yang akan dikirim
} struct_message;

struct_message myData;
bool balasan = true;
bool balasan2 = false;
bool kedip = false;
bool sensorLock = false;

TM1637Display display(CLK, DIO);

void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
    struct_message receivedData;
    memcpy(&receivedData, incomingData, sizeof(receivedData));
    Serial.print("Pesan diterima: ");
    Serial.println(receivedData.message);

    if (strcmp(receivedData.message, "mesin sudah hidup") == 0) {
        balasan2 = true; 
        balasan = false;
        sensorLock = false;
    }
    if (strcmp(receivedData.message, "mesin2 selesai") == 0) {
         balasan = true;
        balasan2 = false;
        kedip = false;
       // sensorLock = false;
    }
    if (strcmp(receivedData.message, "mesin selesai") == 0) {
        balasan = true;
        balasan2 = false;
        kedip = false;
        sensorLock = false;
    }
    if (strcmp(receivedData.message, "Sensor lock") == 0) {
        sensorLock = true;
        balasan2 = true; 
        balasan = false;
    }
    if (strcmp(receivedData.message, "mesin sudah dimatikan") == 0) {
        balasan = true;
        balasan2 = false;
        kedip = false;
        sensorLock = false;
    }
}

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
    Serial.println(sendStatus == 0 ? "Pengiriman berhasil" : "Pengiriman gagal");
}

unsigned int angka;

void setup() {
    Serial.begin(115200);  // Memulai komunikasi serial
    WiFi.mode(WIFI_STA);
    pinMode(btn1, INPUT_PULLUP);
    pinMode(btn2, INPUT_PULLUP);
    pinMode(btn3, INPUT_PULLUP);
    pinMode(led, OUTPUT);

    Serial.println(F("Initialize System"));
    EEPROM.begin(EEPROM_SIZE);

    int address = 0;
    EEPROM.get(address, angka);
    Serial.print("Angka di EEPROM: ");
    Serial.println(angka);

    display.setBrightness(3);
    display.showNumberDecEx(angka, 0x40, true);
    delay(2000);

    if (esp_now_init() != 0) {
        Serial.println("Gagal menginisialisasi ESP-NOW");
        return;
    }

    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);

    esp_now_add_peer(broadcastAddress2, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
    esp_now_add_peer(broadcastAddress1, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
}

void loop() {
    // Mengirim pesan dari serial monitor
    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n'); // Read Serial input
        input.trim(); // Remove any trailing spaces/newlines

        if (input.equalsIgnoreCase("run_washer") && balasan) {
            strcpy(myData.message, "hidupkan washer");
            balasan2 = true;
            balasan = false;

            angka = angka + 1;
            

            int address = 0;
            EEPROM.put(address, angka);
            EEPROM.commit();
            display.showNumberDecEx(angka, 0x40, true);

            esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData));
            Serial.println("Perintah 'dryer di hidupkan' diterima, mesin dihidupkan.");
        }
        
        else if (input.equalsIgnoreCase("off_washer") && !sensorLock && !balasan) {
            strcpy(myData.message, "matikan mesin");
           balasan2 = false;

            //angka = angka + 1;
           // balasan2 = false;

            //int address = 0;
            //EEPROM.put(address, angka);
            //EEPROM.commit();
            //display.showNumberDecEx(angka, 0x40, true);

            esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData));
            Serial.println("Perintah 'washer di dimatikan");
        } else if (input.equalsIgnoreCase("reset_counter")) {
            angka = 0;

            int address = 0;
            EEPROM.put(address, angka);
            EEPROM.commit();
            display.showNumberDecEx(angka, 0x40, true);

            Serial.println("Counter direset.");
        } else if (input.equalsIgnoreCase("status")) {
            Serial.print("Angka saat ini: ");
            Serial.println(angka);
            Serial.print("Status balasan2: ");
            Serial.println(balasan2 ? "true" : "false");
            Serial.print("Status balasan: ");
            Serial.println(balasan ? "true" : "false");
            Serial.print("Status kedip: ");
            Serial.println(kedip ? "true" : "false");
        } else {
            Serial.println("Perintah tidak dikenali.");
        }
    }

    // Mengirim pesan saat tombol 1 ditekan
    if (digitalRead(btn1) == 0 && balasan) {
        strcpy(myData.message, "hidupkan washer");
        balasan2 = true;
        balasan = false;

        angka++;  // Menambah angka
        

        int address = 0;
        EEPROM.put(address, angka);
        EEPROM.commit();
        display.showNumberDecEx(angka, 0x40, true);
        esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData)); // Kirim pesan
        delay(2000);  // Interval pengiriman pesan
    }
     //if (sensorLock) {
   //     Serial.println("mesin digunakan");
   //     delay(1000); // Interval pengiriman pesan
   // }

    // Menyalakan LED jika balasan diterima
    if (balasan2) {
        digitalWrite(led, HIGH);
        Serial.println("mesin digunakan");
        delay(1000);
    } else {
        digitalWrite(led, LOW);
    }

    // Reset angka jika tombol 2 ditekan
    if (digitalRead(btn2) == 0) {
        angka = 0;
        EEPROM.put(0, angka);
        EEPROM.commit();
        display.showNumberDecEx(angka, 0x40, true);
        Serial.println("Angka di-reset melalui tombol");
        delay(1000);
    }
}
