#include <TM1637Display.h>
#include <EEPROM.h>
#include <espnow.h>
#include <ESP8266WiFi.h>

// Define the connections pins
#define CLK D5
#define DIO D6
#define led D1
#define led2 D2
#define led3 D8
#define up D3
#define down D4
#define save D7

uint8_t onMask[] = {
    0b00111111, // O
    0b01010100,  // N
};
uint8_t displayContent2[] = {
    0, // O
    0, // F
    0,          // Spasi (kosong)  0b01110001  f
    0 // 1
};
uint8_t displayContent21[] = {
    0b00111111, // O
    0, // F
    0,          // Spasi (kosong)  0b01110001  f
    0 // 1
};
uint8_t displayContent22[] = {
    0b00111111, // O
    0b01110001, // F
    0,          // Spasi (kosong)  0b01110001  f
    0 // 1
};
uint8_t displayContent23[] = {
    0b00111111, // O
    0b01110001, // F
    0b01110001,          // Spasi (kosong)  0b01110001  f
    0 // 1
};
uint8_t displayContent[] = {
     // Huruf kecil "o" (menggunakan pola segmen untuk menyederhanakan bentuk o)
    0b00111111, // N0b00111001,
    0b01010100,
    0, // Tanda hubung (disimulasikan dengan segmen tengah)
    0 // 1
};


// Struktur data untuk mengirim pesan
typedef struct {
    char message[32];  // Pesan teks yang akan dikirim
} struct_message;

struct_message myData;
bool ledControl = false;
bool ledControl2 = false;
bool ledControl3 = false;

bool kedip = false;  // Flag untuk melacak kedipan LED
bool alarm = false;
bool matikan = false;
bool hidupkan =true;

// Alamat MAC perangkat penerima
uint8_t broadcastAddress1[] = {0xF4, 0xCF, 0xA2, 0x77, 0x29, 0xA8}; // Alamat MAC penerima (sesuaikan dengan perangkat lain)

void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
    struct_message receivedData;
    memcpy(&receivedData, incomingData, sizeof(receivedData));
    Serial.print("Pesan diterima: ");
    Serial.println(receivedData.message);

    // Bandingkan pesan yang diterima
    if (strcmp(receivedData.message, "hidupkan mesin") == 0) {
        ledControl = true;  // Set flag untuk kontrol LED di loop
    }
    if (strcmp(receivedData.message, "matikan mesin") == 0) {
        ledControl3 = true;  // Set flag untuk kontrol LED di loop
    }
    if (strcmp(receivedData.message, "hidupkan mesin2") == 0) {
        ledControl2 = true;  // Set flag untuk kontrol LED di loop
    }
    
    
}

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
    Serial.println(sendStatus == 0 ? "Pengiriman berhasil" : "Pengiriman gagal");
}

TM1637Display display(CLK, DIO);

int countdownTime = 0; // Countdown time in seconds (e.g., 5 minutes)
int eepromAddress = 0;

int readCountdownTime();
void saveCountdownTime();

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    display.setBrightness(0x0f);
    display.clear();
    
if (countdownTime > 0) {
    Serial.print("Countdown: ");
    Serial.println(countdownTime);
    display.showNumberDecEx(countdownTime, 0x40, true);
} else {
    display.setSegments(displayContent23, 4);
}

    //alarm = true;
    //delay(2000);
    //alarm = false;
    //display.setSegments(countdownTime, 4);

    pinMode(led, OUTPUT);
    pinMode(led2, OUTPUT);
    pinMode(led3, OUTPUT);
    pinMode(up, INPUT_PULLUP);
    pinMode(down, INPUT_PULLUP);
    pinMode(save, INPUT_PULLUP);

    // Inisialisasi ESP-NOW
    if (esp_now_init() != 0) {
        Serial.println("Gagal menginisialisasi ESP-NOW");
        return;
    }

    esp_now_set_self_role(ESP_NOW_ROLE_COMBO); // Sebagai pengirim dan penerima
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);
    esp_now_add_peer(broadcastAddress1, ESP_NOW_ROLE_COMBO, 1, NULL, 0);

      display.setBrightness(0x0f);// Set the display to maximum brightness

    EEPROM.begin(512);
    countdownTime = readCountdownTime(); // Load the countdown time from EEPROM

    Serial.print("Loaded countdown time: ");
    Serial.println(countdownTime);

    strcpy(myData.message, "mesin selesai");
        esp_now_send(broadcastAddress1, (uint8_t *)&myData, sizeof(myData)); // Kirim pesan
        delay(2000); // Set flag to prevent further blinking
        }


void loop() {
    if (countdownTime > 0) {
        int minutes = countdownTime / 60;
        int seconds = countdownTime % 60;

        // Display the time in mm:ss format
        display.showNumberDecEx(minutes * 100 + seconds, 0x40, true);

        delay(1000); // Wait for 1 second
        countdownTime--; // Decrease countdown time

        saveCountdownTime(); // Save updated countdown time to EEPROM
        digitalWrite(led, HIGH); // Turn on the LED during countdown
        alarm = true;
        //hidupkan = true;
       
        
    }

    if (hidupkan && countdownTime > 0) {
       strcpy(myData.message, "mesin sudah hidup");
        esp_now_send(broadcastAddress1, (uint8_t *)&myData, sizeof(myData));
        delay(200);
        hidupkan = false;

    }

    // When countdown reaches 0, stop the countdown and turn off LED
    if (alarm && countdownTime <= 0) {
        countdownTime = 0; // Ensure countdown doesn't go negative
        digitalWrite(led, LOW); // Turn off the LED
        
        kedip=true;
        alarm = false;
      
       display.setSegments(displayContent23, 4);
       


        // Only blink LED once if it hasn't blinked yet
        if (kedip) {
          Serial.println("LED is blinking!");
            delay(1000);
            digitalWrite(led3, HIGH);  // Turn on LED
            delay(500);  // Wait for 500 milliseconds
            digitalWrite(led3, LOW); 
            delay(500);
            digitalWrite(led3, HIGH);  // Turn on LED
            delay(500);  // Wait for 500 milliseconds
            digitalWrite(led3, LOW); 
            delay(500);
            digitalWrite(led3, HIGH);  // Turn on LED
            delay(500);  // Wait for 500 milliseconds
            digitalWrite(led3, LOW); 
            delay(500);
            digitalWrite(led3, HIGH);  // Turn on LED
            delay(500);  // Wait for 500 milliseconds
            digitalWrite(led3, LOW); 
            delay(500);
            digitalWrite(led3, HIGH);  // Turn on LED
            delay(500);  // Wait for 500 milliseconds
            digitalWrite(led3, LOW); 
            delay(500);  // Turn off LED
            // Turn on LED
            // Wait for 500 milliseconds

            kedip = false; 
            
            strcpy(myData.message, "mesin2 selesai");
        esp_now_send(broadcastAddress1, (uint8_t *)&myData, sizeof(myData)); // Kirim pesan
        delay(2000); // Set flag to prevent further blinking
        }
    }

    // Button handling for LED control
    if (ledControl && countdownTime <= 0) {
      display.setSegments(displayContent, 4);
        countdownTime = 15 * 60;
        
        delay(100);
        ledControl = false; 
        delay(300);
       // countdownTime = 10;
       // kedip = true;
        digitalWrite(led, HIGH);
        delay(1500);
        digitalWrite(led2, HIGH);
        delay(300);
        digitalWrite(led2, LOW);
        delay(1000);
        //kedip = true;

         // Reset flag
        strcpy(myData.message, "mesin sudah hidup");
        esp_now_send(broadcastAddress1, (uint8_t *)&myData, sizeof(myData)); // Kirim pesan
        delay(2000);  // Interval pengiriman pesan
    }
    if (ledControl3 && countdownTime > 0) {
      display.setSegments(displayContent23, 4);
        countdownTime = 1;
        
        delay(100);
        ledControl3 = false; 
        

         // Reset flag
        strcpy(myData.message, "mesin sudah mati");
        esp_now_send(broadcastAddress1, (uint8_t *)&myData, sizeof(myData)); // Kirim pesan
        delay(2000);  // Interval pengiriman pesan
    }
    if (ledControl2 && countdownTime <= 0) {
       display.setSegments(displayContent, 4);
        countdownTime = 10 * 60;
        
       
        delay(100);
        ledControl2 = false; 
        delay(300);
       // countdownTime = 10;
        
        digitalWrite(led, HIGH);
        delay(1500);
        digitalWrite(led2, HIGH);
        delay(300);
        digitalWrite(led2, LOW);
        delay(1000);
       // kedip = true;

         // Reset flag
        strcpy(myData.message, "mesin sudah hidup");
        esp_now_send(broadcastAddress1, (uint8_t *)&myData, sizeof(myData)); // Kirim pesan
        delay(2000);  // Interval pengiriman pesan
    }
    if (matikan && countdownTime >= 0) {
        countdownTime = 1;
        display.setSegments(displayContent23, 4);
        delay(100);
        matikan = false; 
       
         // Reset flag
        strcpy(myData.message, "mesin sudah dimatikan");
        esp_now_send(broadcastAddress1, (uint8_t *)&myData, sizeof(myData)); // Kirim pesan
        delay(2000);  // Interval pengiriman pesan
    }

    // Button handling for UP and DOWN buttons
    if (digitalRead(up) == LOW) {
        countdownTime = 10;
       // kedip = true;
        display.setSegments(displayContent, 4);
         strcpy(myData.message, "mesin sudah hidup");
        esp_now_send(broadcastAddress1, (uint8_t *)&myData, sizeof(myData)); // Kirim pesan
        delay(2000);  // Set countdown time to 10 seconds when UP button is pressed
    }

    if (digitalRead(down) == LOW) {
        countdownTime = 0;
       // kedip = true;  // Reset countdown time to 0 when DOWN button is pressed
    }

    if (digitalRead(save) == LOW) {
        countdownTime = 5;
       // kedip = true;
        display.setSegments(displayContent, 4);
         strcpy(myData.message, "mesin sudah hidup");
        esp_now_send(broadcastAddress1, (uint8_t *)&myData, sizeof(myData)); // Kirim pesan
        delay(2000); // Reset countdown time to 5 seconds when SAVE button is pressed
    }
}

// Function to read the countdown time from EEPROM
int readCountdownTime() {
    int lowByte = EEPROM.read(eepromAddress); // Read the lower byte
    int highByte = EEPROM.read(eepromAddress + 1); // Read the upper byte
    return (highByte << 8) | lowByte; // Combine the two bytes into a single integer
}

// Function to save the countdown time to EEPROM
void saveCountdownTime() {
    if (countdownTime > -1) {
        EEPROM.write(eepromAddress, countdownTime & 0xFF);         // Lower byte
        EEPROM.write(eepromAddress + 1, (countdownTime >> 8) & 0xFF); // Upper byte
        EEPROM.commit(); // Commit the changes to EEPROM
        Serial.print("Saved countdown time: ");
        Serial.println(countdownTime);
    }
}
