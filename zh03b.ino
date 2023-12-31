#include <SD_ZH03B.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

// RTC_DS3231 rtc;
// char daysOfTheWeek[7][12] = {"Ahad", "Senin", "Selasa", "Rabu", "Kamis", "Jum'at", "Sabtu"};
// int jam, menit, detik;
// int tanggal, bulan, tahun;
// String hari;
float ppm;
int pm2_5, pm10, AQIco, AQIpm2_5, AQIpm10;
String str;
SoftwareSerial ZHSerial(3, 4); // RX, TX
SoftwareSerial nodemcu(5,6);
LiquidCrystal_I2C lcd(0x27,16,4);
SD_ZH03B ZH03B( ZHSerial, SD_ZH03B::SENSOR_ZH03B );  // same as the line above
// const int reedSwitchPin = 8; // Digital pin 13
// volatile int tipCount = 0;    // Counter for tip events
// volatile float conversionFactor = 0.5;
// float rain=0;
// unsigned long lastResetTime = 0; // Variable to store the last reset time
// const unsigned long resetInterval = 86400000; // 1 minute in milliseconds

// void reedSwitchISR() {
//   tipCount++; // Increment the tip count when the reed switch is triggered
//   rain = tipCount * conversionFactor;
//      lcd.clear(); 
//   lcd.setCursor(1, 4);    // Set the cursor to the second line
//   lcd.print("CRH_HJN:");   // Display a label
//   lcd.print(rain);
//   Serial.print("CRH_HJN:");   // Display a label
//   Serial.print(rain);  
//   lcd.setCursor(11,4);
//   lcd.print("Tip:");   // Display a label
//   lcd.print(tipCount); 
//   Serial.print("Tip:");   // Display a label
//   Serial.print(tipCount); 
// }

void getdata(){
   if( ZH03B.readData() ) {
   pm2_5 = ZH03B.getPM2_5();
  pm10 = ZH03B.getPM10_0();

    // Calculate AQI for PM2.5
  double Iapm2_5, Ibpm2_5, Xxpm2_5, Xapm2_5, Xbpm2_5;
  Xxpm2_5 = pm2_5;
  if (pm2_5 >= 0 && pm2_5 <= 55.4) {
    Iapm2_5 = 100;
    Ibpm2_5 = 50;
    Xapm2_5 = 55.4;
    Xbpm2_5 = 15.4;
  } else if (pm2_5 >= 55.5 && pm2_5 <= 150.4) {
    Iapm2_5 = 200;
    Ibpm2_5 = 100;
    Xapm2_5 = 150.4;
    Xbpm2_5 = 55.4;
  } else if (pm2_5 >= 150.5 && pm2_5 <= 250.4) {
    Iapm2_5 = 300;
    Ibpm2_5 = 200;
    Xapm2_5 = 250.4;
    Xbpm2_5 = 150.4;
  } else if (pm2_5 >= 500) {
    Iapm2_5 = 500;
    Ibpm2_5 = 300;
    Xapm2_5 = 1000;
    Xbpm2_5 = 500;
  }
  double ataspm2_5 = (Iapm2_5 - Ibpm2_5) * (pm2_5 - Xbpm2_5);
  double bawahpm2_5 = (Xapm2_5 - Xbpm2_5);
  int AQIpm2_5 = (ataspm2_5 / bawahpm2_5) + Ibpm2_5;

   // Calculate AQI for PM10
  double Iapm10, Ibpm10, Xxpm10, Xapm10, Xbpm10;
  Xxpm10 = pm10;
  if (pm10 >= 0 && pm10 <= 150) {
    Iapm10 = 100;
    Ibpm10 = 50;
    Xapm10 = 150;
    Xbpm10 = 50;
  } else if (pm10 >= 151 && pm10 <= 350) {
    Iapm10 = 200;
    Ibpm10 = 100;
    Xapm10 = 350;
    Xbpm10 = 150;
  } else if (pm10 >= 351 && pm2_5 <= 420) {
    Iapm10 = 300;
    Ibpm10 = 200;
    Xapm10 = 420;
    Xbpm10 = 350;
  } else if (pm10 >= 500) {
    Iapm10 = 500;
    Ibpm10 = 300;
    Xapm10 = 1000;
    Xbpm10 = 500;
  }
  double ataspm10 = (Iapm10 - Ibpm10) * (pm10 - Xbpm10);
  double bawahpm10 = (Xapm10 - Xbpm10);
  int AQIpm10 = (ataspm10 / bawahpm10) + Ibpm10;


    Serial.print("PM 2.5: ");
    Serial.println(pm2_5);
        Serial.print("PM 10: ");
    Serial.println(pm10);
          lcd.setCursor(7, 1);
      lcd.print(pm2_5);
      lcd.setCursor(7, 2);
      lcd.print(pm10);
            lcd.setCursor(10, 2);
      lcd.print("AQI:");
            lcd.setCursor(10, 1);
      lcd.print("AQI:");
                  lcd.setCursor(15, 2);
      lcd.print(AQIpm10);
            lcd.setCursor(15, 1);
      lcd.print(AQIpm2_5);
      

           StaticJsonDocument<200> doc;
  doc["a"] = pm2_5;
  doc["b"] = pm10;
  doc["c"] = AQIpm2_5;
  doc["d"] = AQIpm10; 
  // doc["e"] = rain;

  String jsonString;
  serializeJson(doc, jsonString);
  Serial.println(jsonString);
  nodemcu.println(jsonString);
   }
}

void setup() {
//    pinMode(reedSwitchPin, INPUT_PULLUP); // Enable internal pull-up resistor
// attachInterrupt(digitalPinToInterrupt(reedSwitchPin), reedSwitchISR, FALLING); // Attach ISR to the falling edge of the reed switch signal
//   lastResetTime = millis();
//    unsigned long currentTime = millis();
//    if (currentTime - lastResetTime >= resetInterval) {
//      // Reset the tipCount to zero
//      tipCount = 0;
//      rain = 0; 
//         // Update the last reset time
//      lastResetTime = currentTime;
//    }
    Serial.begin(9600);
    nodemcu.begin(9600);
    delay(1000);
    Serial.println("program started");
    Serial.println("-- Initializing ZH03B...");
     ZHSerial.begin(9600);
    delay(500);
    ZH03B.setMode( SD_ZH03B::IU_MODE );
    Serial.println("-- Reading ZH03B --");
    delay(200);
	  //Serial.print("sizeof frame(should be 24bytes): "); Serial.println( sizeof(union_t) );
//  if (! rtc.begin()) {
//     Serial.println("Couldn't find RTC");
//     Serial.flush();
//     while (1) delay(10);
//   }
    lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Air Quality:");
  lcd.setCursor(0, 1);
  lcd.print("PM2.5: ");
  lcd.setCursor(0, 2);
  lcd.print("PM10: ");
  lcd.setCursor(0, 3);
}


void loop () {
  // DateTime now = rtc.now();
  // jam     = now.hour();
  // menit   = now.minute();
  // detik   = now.second();
  // tanggal = now.day();
  // bulan   = now.month();
  // tahun   = now.year();
  // hari    = daysOfTheWeek[now.dayOfTheWeek()];
  // Serial.println(String() + hari + ", " + tanggal + "-" + bulan + "-" + tahun);
  // Serial.println(String() + jam + "." + menit + "-" + tahun);
  // Serial.println();
  delay(1000);
 getdata();
//  delay(3000);

}
