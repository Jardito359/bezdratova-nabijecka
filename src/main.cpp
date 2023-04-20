#include <M5StickCPlus.h>
#include "M5_ADS1115.h"
#include "PinDefinitionsAndMore.h"
#include <IRremote.hpp>
#include <Wire.h>
#include "ThingSpeak.h"
#include <WiFi.h>
#include <AsyncTaskLib.h>
#define CHANNEL_ID 2046903
#define CHANNEL_API_KEY "Q6YWHA59CXHHWF68"
#define WIFI_TIMEOUT_MS 20000
#define WIFI_NETWORK "Beranovi 2,4GHZ"
#define WIFI_PASSWORD "Beran 58"

float volt = 0;
float current = 0;
float baterie = 0;

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

void connectToWiFi(){
    Serial.print("Connecting to Wifi");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);
    unsigned long startAttemptTime = millis();
    // Keep looping while we're not connected and haven't reached the timeout
    while (WiFi.status() != WL_CONNECTED && 
              millis() - startAttemptTime < WIFI_TIMEOUT_MS){
        Serial.print(".");
        delay(100);
    }
    // Make sure that we're actually connected, otherwise go to deep sleep
    if(WiFi.status() != WL_CONNECTED){
        Serial.println(" Failed!");
      // Handle this case. Restart ESP, go to deep sleep, retry after delay...
    }else{
        Serial.print(" Connected!");
        Serial.println(WiFi.localIP());
    }
}
void odesilani(float volt,float current,float baterie)
{
    Serial.print("posilam");
    if (volt != 0 || current!= 0 || baterie!= 0){
            ThingSpeak.setField(1, volt);
            ThingSpeak.setField(2, current);
            ThingSpeak.setField(3, baterie);
            ThingSpeak.writeFields(CHANNEL_ID, CHANNEL_API_KEY);
        Serial.print("poslano");
        }
}
AsyncTask task(15000, true, []() { odesilani(volt, current,baterie); });

void setup(void) {
    M5.begin();
    Wire.begin();
    voltmeter.setMode(SINGLESHOT);              // | PAG      | Max Input Voltage(V) |
    voltmeter.setRate(RATE_8);                  // | PAG_6144 |        128           |
    voltmeter.setGain(PAG_512);                 // | PAG_4096 |        64            |
    hope = page512_volt / voltmeter.resolution; // | PAG_2048 |        32            |
                                                // | PAG_512  |        16            |
                                                // | PAG_256  |        8             |
    M5.begin();
    Serial.begin(115200);
    M5.Lcd.setTextFont(2);
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
    M5.Lcd.setCursor(51, 225);
    //   M5.Lcd.setCursor(118, 90);
    //   M5.Lcd.printf("SAVE");
    // bool result1 = Ammeter.saveCalibration2EEPROM(PAG_256, 1024, 1024);
    // delay(10);


    // Connect to Wi-Fi network
    M5.begin();
    Serial.begin(115200);
    connectToWiFi(); // this function comes from a previous video
    
    ThingSpeak.begin(client);
    task.Start();
    }
void loop() 
    {
    M5.update(); 
    task.Update();

    //M5.Lcd.println("poslano");
    //Serial.print("poslano ");
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
    volt= voltmeter.getValue() ;
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setCursor(16, 20);
    M5.Lcd.printf(" %.2f mv \r\n",adc_raw * voltmeter.resolution * voltmeter.calibration_factor);
    
    //M5.Lcd.setTextColor(WHITE, BLACK);
    //M5.Lcd.setCursor(10, 40);
    //M5.Lcd.printf("Cal ADC: %.0f \r\n",adc_raw * voltmeter.calibration_factor);
        
    //powerbanka max 11763V = 117,63
    //sluchátka krabička 26%
    baterie = volt/120;
    current = Ammeter.getValue() *-1;
    /*Serial.print(volt);
    Serial.print(" ");
    Serial.print(current);
    Serial.print(" ");
    Serial.print(baterie);
    */
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setCursor(15, 90);
    M5.Lcd.printf("  %.2f mA", current);
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
}