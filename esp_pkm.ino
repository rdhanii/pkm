#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
SoftwareSerial nodemcu(15,13);

// Replace with your network credentials
const char* ssid = "BOLT!Super4G-F185";
const char* password = "796qe1yt";

// Initialize Telegram BOT
#define BOTtoken "6482768636:AAFAfvpcGdP1FF5Di9gDnirkPpPAiSYKoFI"  // your Bot Token (Get from Botfather)
#define CHAT_ID "-1623126377"
float pm2_5 = 0;
float pm10 = 0;
float AQIpm2_5 = 0;
float AQIpm10 = 0;
float rain = 0;

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
const IPAddress server(0,0,0,0);
const int httpPort = 80;
// Checks for new messages every 1 second.
int botRequestDelay = 100;
unsigned long lastTimeBotRan;

const int ledPin = 2;
bool ledState = LOW;



void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));
  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    
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
}

void loop() {
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


       https.begin(*client,address);  //Specify request destination
      int httpCode = https.GET();//Send the request
      String payload;  
      if (httpCode > 0) { //Check the returning code    
          payload = https.getString();   //Get the request response payload
          payload.trim();
          if( payload.length() > 0 ){
            Serial.println(payload + "\n");
          }
          
      }
       delay(120000);

      https.end();   //Close connection
  }else{
    Serial.print("Not connected to wifi ");Serial.println(ssid);
  }

// delay(1000);
}