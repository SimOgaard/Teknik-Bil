#include "EspMQTTClient.h"
#include <ArduinoJson.h>

void onConnectionEstablished();

EspMQTTClient client(
  "ABB_Indgym_Guest", 
  "Welcome2abb",
  "maqiatto.com",
  1883,
  "oliver.witzelhelmin@abbindustrigymnasium.se",
  "vroom",
  "broom_broom",
  onConnectionEstablished,
  true,
  true
);

#define D1 0
#define Pw 5
#define He 4

unsigned long previousMillis = 0, currentMillis = 0;

float RequestedRPM = 2;
int maxSpeed = 1024;
float RPM;
int Rev;
int Speed = 300;

boolean go = false;

void onConnectionEstablished() {
  client.publish("oliver.witzelhelmin@abbindustrigymnasium.se/broom_broom", "we aint groomin we vroomin");
  client.subscribe("oliver.witzelhelmin@abbindustrigymnasium.se/vroom_vroom", [] (const String & payload) {
    StaticJsonBuffer<200> JsonBuffer;
    JsonObject& root = JsonBuffer.parseObject(payload);
    if(root.success()) {
      if(root["Message"] == "Start"){
        go = true;
      }if(root["Message"] == "Stop"){
        go=false;
      }
    }
  });
}

void setup() {
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(He), HtoL, FALLING);
  pinMode(Pw, OUTPUT), pinMode(D1, OUTPUT);
  digitalWrite(D1, HIGH);
}

void loop() {
  if(go == true){
    rpm(100);
  }
  client.loop();
}

void AccToPw(int Interval){
  while(analogRead(Pw) < maxSpeed){
    currentMillis = millis(); 
    if (currentMillis > previousMillis + Interval){
      previousMillis = currentMillis;
      analogWrite(Pw, analogRead(Pw)+1);
    }
  }
}

void rpm(int Interval){
  currentMillis = millis();
  if (currentMillis > previousMillis + Interval){
    RPM = float(float(Rev)/float(currentMillis-previousMillis));
    SendJSON();
    currentMillis = millis();
    AccToRPM();
    Rev = 0;
    previousMillis = currentMillis;
  }
}

void AccToRPM(){
  if (RPM < RequestedRPM){
    Speed += 5;
  } else if (RPM > RequestedRPM) {
    Speed -= 5;
  }
  analogWrite(Pw, Speed);
}

void SendJSON(){
  String payload = "{\"RPM\":" + String(RPM,6) + ",\"Rev\":" + String(Rev) + ",\"Interval\":" + String((currentMillis-previousMillis)) + ",\"Pw\":" + String(Speed)+"}";
  Serial.println(payload);
  client.publish("oliver.witzelhelmin@abbindustrigymnasium.se/broom_broom", payload);
}

ICACHE_RAM_ATTR void HtoL(){
  Rev++;
}
