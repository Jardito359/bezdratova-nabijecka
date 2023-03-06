#include <M5StickCPlus.h>
#include "M5_ADS1115.h"
#include "PinDefinitionsAndMore.h"
#include <IRremote.hpp>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>


uint8_t sAddress = 0x8;
uint8_t sCommand = 0x3D;
uint8_t sRepeats = 1;


const char* ssid = "Beranovi 2,4GHZ";
const char* password = "Beran 58";
const char* server = "api.thingspeak.com";

const String apiKey = "Q6YWHA59CXHHWF68";
const String channelId = "2046903";
const String field1Name = "Napětí";
const String field2Name = "Proud";


//0x49 - zapnuto 0x3D vypnuto

ADS1115 Ammeter(AMETER, AMETER_ADDR, AMETER_EEPROM_ADDR);
ADS1115 voltmeter;

float page512_volt = 2000.0F;
float page4096_volt = 60000.0F;

int16_t volt_raw_list[10];
uint8_t raw_now_ptr = 0;
int16_t adc_raw     = 0;

int16_t hope = 0.0;

ADS1115Gain_t now_gain = PAG_512;

WiFiClient client;

void setup(void) {
    M5.begin();
    Wire.begin();
    
    voltmeter.setMode(SINGLESHOT);              // | PAG      | Max Input Voltage(V) |
    voltmeter.setRate(RATE_8);                  // | PAG_6144 |        128           |
    voltmeter.setGain(PAG_512);                 // | PAG_4096 |        64            |
    hope = page512_volt / voltmeter.resolution; // | PAG_2048 |        32            |
                                                // | PAG_512  |        16            |
                                                // | PAG_256  |        8             |
    M5.Lcd.setTextFont(4);  // Set font to 4 point font.  设置字体为4号字体

    M5.Lcd.setCursor(52, 210);  // Set the cursor at (52,210).  将光标设置在(52, 210)

    M5.begin();
    Serial.begin(115200);
    M5.Lcd.setTextFont(1);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(WHITE);

    Ammeter.setMode(SINGLESHOT);
    Ammeter.setRate(RATE_8);
    Ammeter.setGain(PAG_512);
    hope = page512_volt / Ammeter.resolution;
    // | PAG      | Max Input Voltage(V) |
    // | PAG_6144 |        128           |
    // | PAG_4096 |        64            |
    // | PAG_2048 |        32            |
    // | PAG_512  |        16            |
    // | PAG_256  |        8             |

    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextFont(1);

    M5.Lcd.setCursor(51, 225);

    //   M5.Lcd.setCursor(118, 90);
    //   M5.Lcd.printf("SAVE");

    // bool result1 = Ammeter.saveCalibration2EEPROM(PAG_256, 1024, 1024);
    // delay(10);

    pinMode(10, OUTPUT); // nastavení pinu 10 jako výstupní pin pro ovládání LED diody
    digitalWrite(10, HIGH); // nastavení červené barvy

    // Connect to Wi-Fi network
    M5.begin();
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
}

void loop() 

{
M5.update(); 

    sAddress = 0x1;
    sCommand = 0xA;
    sRepeats = 0x1;
    IrSender.sendNEC(sAddress, sCommand, sRepeats);
    M5.Lcd.setCursor(10,180);
    M5.Lcd.print(sCommand, HEX);
    M5.Lcd.setCursor(10,190);
    M5.Lcd.println("poslano");
    Serial.print("poslano ");

    voltmeter.getValue();

    volt_raw_list[raw_now_ptr] = voltmeter.adc_raw;
    raw_now_ptr                = (raw_now_ptr == 9) ? 0 : (raw_now_ptr + 1);

    int count = 0;
    int total = 0;

    for (uint8_t i = 0; i < 10; i++) {
        if (volt_raw_list[i] == 0) {
            continue;
        }
        total += volt_raw_list[i];
        count += 1;
    }

    if (count == 0) {
        adc_raw = 0;
    } else {
        adc_raw = total / count;
    }

    float volt= voltmeter.getValue() *(-1);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setCursor(10, 20);
    M5.Lcd.printf("Napeti: %.2f mv \r\n",adc_raw * voltmeter.resolution * voltmeter.calibration_factor);

    
    //M5.Lcd.setTextColor(WHITE, BLACK);
    //M5.Lcd.setCursor(10, 40);
    //M5.Lcd.printf("Cal ADC: %.0f \r\n",adc_raw * voltmeter.calibration_factor);
        
    //powerbanka max 11763V = 117,63
    //sluchátka krabička 26%
    float baterie = volt/12;
    float current = Ammeter.getValue() ;
    Serial.print(volt);
    Serial.print(" ");
    Serial.print(current);
    Serial.print(" ");
    Serial.print(baterie);
    
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setCursor(10, 40);
    M5.Lcd.printf("Proud:  %.2f mA", current);

    volt_raw_list[raw_now_ptr] = Ammeter.adc_raw;
    raw_now_ptr                = (raw_now_ptr == 9) ? 0 : (raw_now_ptr + 1);


    for (uint8_t i = 0; i < 10; i++) {
        if (volt_raw_list[i] == 0) {
            continue;
        }
        total += volt_raw_list[i];
        count += 1;
    }

    if (count == 0) {
        adc_raw = 0;
    } else {
        adc_raw = total / count;
    }
    
    //M5.Lcd.setTextColor(WHITE, BLACK);
    //M5.Lcd.setCursor(10, 80);
    //M5.Lcd.printf("Cal ADC: %.0f", adc_raw * Ammeter.calibration_factor);

    Serial.print("Voltage: ");
    Serial.print(volt);
    Serial.print(" V, Current: ");
    Serial.print(current);
    Serial.println(" A");

    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = "http://" + String(server) + "/update?api_key=" + apiKey + "&" + field1Name + "=" + String(volt) + "&" + field2Name + "=" + String(current);
        http.begin(url);
        int httpCode = http.GET();
        if (httpCode > 0) {
        Serial.println("Data sent to ThingSpeak");
        } else {
        Serial.println("Error sending data to ThingSpeak");
        }
        http.end();
    } else {
        Serial.println("WiFi not connected");
    }

  delay(15000); // prodleva 10 sekund
}

float volt() {
  // kód pro čtení hodnoty z voltmetru
}

float current() {
  // kód pro čtení hodnoty z ampérmetru
}
