#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <RTClib.h>
SoftwareSerial nodemcu(15,13);

// Replace with your network credentials
const char* ssid ="Taruna Spot"; //"BOLT!Super4G-F185";
const char* password ="stmkgofficial";// "796qe1yt";

const int reedSwitchPin = D3; // Digital pin 13
volatile int tipCount24 = 0;    // Counter for tip events
volatile float conversionFactor = 0.5;
float rain24=0;
unsigned long lastResetTime24 = 0; // Variable to store the last reset time

// Initialize Telegram BOT
#define BOTtoken "6482768636:AAFAfvpcGdP1FF5Di9gDnirkPpPAiSYKoFI"  // your Bot Token (Get from Botfather)
#define CHAT_ID "-1623126377"
float pm2_5 = 0;
float pm10 = 0;
float AQIpm2_5 = 0;
float AQIpm10 = 0;

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif
RTC_DS3231 rtc;
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
const IPAddress server(0,0,0,0);
const int httpPort = 80;
// Checks for new messages every 1 second.
int botRequestDelay = 100;
unsigned long lastTimeBotRan;

const int ledPin = 2;
bool ledState = LOW;

void IRAM_ATTR reedSwitchISR() {

  tipCount24++; // Increment the tip count when the reed switch is triggered
  rain24 = tipCount24 * conversionFactor; 
}

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));
  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;
if (text == "/start") {
      String welcome = "Welcome to BOT PKM STMKG, " + from_name + ".\n";
      welcome += "Use the following commands to know about Hidrometeorologi status.\n\n";
      welcome += "/kualitasudara untuk mendapatkan informasi tentang kualitas udara\n";
      welcome += "/Hujan untuk mendapatkan informasi tentang hujan\n";
      bot.sendMessage(chat_id, welcome, "");
    }
    
    
  }
}

void setup() {
  Serial.begin(115200);
  nodemcu.begin(9600);
  while (!Serial) continue;
  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
    client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  #endif

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);
  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());
  bot.sendMessage(CHAT_ID, "Bot Started", "");

  pinMode(reedSwitchPin, INPUT_PULLUP); // Enable internal pull-up resistor
  attachInterrupt(digitalPinToInterrupt(reedSwitchPin), reedSwitchISR, FALLING); // Attach ISR to the falling edge of the reed switch signal
    rtc.begin();

}

void loop() {

DateTime now = rtc.now();
  unsigned long currentTime = now.unixtime();
  if (currentTime - lastResetTime24 >= 86400) {
    // Reset the tipCount to zero
    tipCount24 = 0;
    rain24 = 0; 
    
    // Update the last reset time
    lastResetTime24 = currentTime;
  }


  String data = nodemcu.readStringUntil('\n');
  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, data);

  Serial.println("JSON Object Recieved");
  Serial.print("Recieved pm2.5:  ");
  float pm2_5 = doc["a"];
  Serial.println(pm2_5);
  Serial.print("Recieved pm10:  ");
  float pm10 = doc["b"];
  Serial.println(pm10);
  Serial.print("Recieved AQIpm2_5:  ");
  float AQIpm2_5 = doc["c"];
  Serial.println(AQIpm2_5);
  Serial.print("Recieved AQIpm10:  ");
  float AQIpm10 = doc["d"];
  Serial.println(AQIpm10);
  Serial.println("-----------------------------------------");
    if (isnan(pm2_5)&&isnan(pm10)&&isnan(AQIpm2_5)&&isnan(AQIpm10)) {
    Serial.println(F("Failed to read from sensor!"));
    return;
     }

  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
  String text;
  if (text == "/kualitasudara") {
      String pm25 = "kualitas udara :\nPM 2.5 : ";
        pm25 += int(pm2_5);
        pm25 += "\nPM 10 : ";
        pm25 += int(pm10);
        pm25 += "\nAQIpm2.5 : ";
        pm25 += int(AQIpm2_5);
        pm25 += "\nAQIpm10 : ";
        pm25 += int(AQIpm10);

    bot.sendMessage(CHAT_ID, pm25, "");
    Serial.print("Mengirim data sensor ke telegram");
    }
    
    if (text == "/Hujan") {
      String hujan = "informasi hujan; \nHujan ;";
      hujan += int(rain24);
    bot.sendMessage(CHAT_ID, hujan, "");
    Serial.print("Mengirim data sensor ke telegram");
    }
    
if ((WiFi.status() == WL_CONNECTED)) {
      std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
      client->setInsecure();
      
      //create an HTTPClient instance
      HTTPClient https;
      
      String address;
      //equate with your computer's IP address and your directory application
      // C:\xampp\htdocs\arducoding_tutorial\nodemcu_log\webapi\api\create.php
      // address ="https://hanip.my.id/webapiarghulu/api/create.php?level=0&hujan=";
      // address += String(rain);
      address = "https://fews.stmkg.ac.id/webapipkm/api/create.php";
      address += "?pm2_5=" + String(pm2_5);
      address += "&pm10=" + String(pm10);
      address += "&aqipm2_5=" + String(AQIpm2_5);
      address += "&aqipm10=" + String(AQIpm10);
      address += "&rain=" + String(rain24);


       https.begin(*client,address);  //Specify request destination
      int httpCode = https.POST();//Send the request
      String payload;  
      if (httpCode > 0) { //Check the returning code    
          payload = https.getString();   //Get the request response payload
          payload.trim();
          if( payload.length() > 0 ){
            Serial.println(payload + "\n");
          }
          
      }
      //  delay(120000);

      https.end();   //Close connection
  }else{
    Serial.print("Not connected to wifi ");Serial.println(ssid);
  }

// delay(1000);
}
