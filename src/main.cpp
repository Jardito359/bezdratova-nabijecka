#include <M5StickCPlus.h>
#include "M5_ADS1115.h"
#include "PinDefinitionsAndMore.h"
#include <IRremote.hpp>
#include <Wire.h>

uint8_t sAddress = 0x8;
uint8_t sCommand = 0x3D;
uint8_t sRepeats = 1;

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
}


int i;

void loop() 
{
M5.update();  // Check the status of the key.  检测按键的状态
  if (M5.BtnA.wasPressed()) {
      voltmeter.setMode(SINGLESHOT);  // Set the mode.  设置模式
      voltmeter.setRate(RATE_8);      // Set the rate.  设置速率
      voltmeter.setGain(PAG_512);
      now_gain = PAG_512;
      hope     = page512_volt / voltmeter.resolution;

      for (uint8_t i = 0; i < 10; i++) {
        volt_raw_list[i] = 0;
      }
    }
    if (M5.BtnA.wasPressed()) {
        Ammeter.setMode(SINGLESHOT);
        Ammeter.setRate(RATE_8);
        Ammeter.setGain(PAG_512);
        now_gain = PAG_512;
        hope     = page512_volt / Ammeter.resolution;

        for (uint8_t i = 0; i < 10; i++) {
            volt_raw_list[i] = 0;
        }
    }


    if (M5.BtnB.wasPressed()) {
        voltmeter.setMode(SINGLESHOT);
        voltmeter.setRate(RATE_8);
        voltmeter.setGain(PAG_4096);
        now_gain = PAG_4096;
        hope     = page4096_volt / voltmeter.resolution;

        for (uint8_t i = 0; i < 10; i++) {
            volt_raw_list[i] = 0;
        }
    }

    sAddress = 0x1;
    sCommand = 0x1;

    M5.update();
    if(M5.BtnA.isPressed()){

        if(i ==0){
            sCommand = 0x49;
            IrSender.sendNEC(sAddress, sCommand, sRepeats);
            i++;
        }
        else{
            sCommand = 0x3D;
            IrSender.sendNEC(sAddress, sCommand, sRepeats);
            i = 0;
        }

    }
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


    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setCursor(10, 20);
    M5.Lcd.printf("Napeti : %.2f mv \r\n",adc_raw * voltmeter.resolution * voltmeter.calibration_factor);
    
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setCursor(10, 40);
    M5.Lcd.printf("Cal ADC: %.0f \r\n",adc_raw * voltmeter.calibration_factor);
    
    M5.Lcd.setCursor(10,180);
    M5.Lcd.print(sCommand, HEX);
    M5.Lcd.setCursor(10,190);
    M5.Lcd.println("poslano");
    Serial.print("poslano ");
     
    float current = Ammeter.getValue();

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
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setCursor(10, 60);
    M5.Lcd.printf("Cal volt:");
    M5.Lcd.setCursor(10, 60);
    M5.Lcd.printf("Proud: %.2f mA", current);

    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setCursor(10, 80);
    M5.Lcd.printf("Cal ADC: %.0f", adc_raw * Ammeter.calibration_factor);

}
