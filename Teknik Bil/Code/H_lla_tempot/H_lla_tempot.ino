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

unsigned long previousMillis, currentMillis = 0;

const int maxSpeed = 1024;
float RPM;
int Rev, SlowPub;
int Speed = 200;

float PubFrequency, Interval, DeriveringInterval, Integration, Proportionell, Easing, Derivering, ProportionellSafety, RequestedRPM;

int SpeedTerm, IntegrationTerm, ProportionellTerm, EasingTerm;

boolean go;

void onConnectionEstablished() {
  client.publish("oliver.witzelhelmin@abbindustrigymnasium.se/broom_broom", "we aint groomin we vroomin");
  client.subscribe("oliver.witzelhelmin@abbindustrigymnasium.se/vroom_vroom", [] (const String & payload) {
    StaticJsonBuffer<200> JsonBuffer;
    JsonObject& root = JsonBuffer.parseObject(payload);
    if(root.success()) {
      go = root["Message"];
      Interval = root["Interval"];
      DeriveringInterval = root["DeriveringInterval"];
      Integration = root["Integration"];
      Proportionell = root["Proportionell"];
      Easing = root["Easing"];
      Derivering = root["Derivering"];
      PubFrequency = root["PubFrequency"];
      RequestedRPM = root["RequestedRPM"];
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
    rpm();
  }else{
    analogWrite(Pw, 0);
  }
  client.loop();
}

/*
void AccToPw(int Interval){
  while(analogRead(Pw) < maxSpeed){
    currentMillis = millis(); 
    if (currentMillis > previousMillis + Interval){
      previousMillis = currentMillis;
      analogWrite(Pw, analogRead(Pw)+1);
    }
  }
}
*/

void rpm(){
  currentMillis = millis();
  if (currentMillis > previousMillis + Interval){
    RPM = float(float(Rev)/float(currentMillis-previousMillis));
    AccToRPM();
    if (SlowPub >= PubFrequency ){
      SendJSON();
      SlowPub = 0;
      currentMillis = millis();
    }
    SlowPub++;
    Rev = 0;
    previousMillis = currentMillis;
  }
}

/*
void rpmRN(){
  currentMillis = millis();
  while (currentMillis < previousMillis + DeriveringInterval){
    currentMillis = millis();
  }
  RPMrn = float(float(Rev)/float(currentMillis-previousMillis));
  previousMillis = currentMillis;
  Rev = 0;
}
*/

void AccToRPM(){
  IntegrationTerm = Integration*pow(RequestedRPM-RPM, 3); //Integration

  if (RPM < RequestedRPM-ProportionellSafety){ //Proportionell 
    ProportionellTerm = Proportionell;
  } else if (RPM > RequestedRPM+ProportionellSafety) {
    ProportionellTerm = -1*Proportionell;
  }

  EasingTerm = Easing * sqrt(abs(RequestedRPM-RPM)); //Easing
  
  SpeedTerm = IntegrationTerm+ProportionellTerm+EasingTerm; //Derivering

  Speed+=SpeedTerm;
/*
  rpmRN(DeriveringInterval);
  if(abs(SpeedTerm) > 1 && abs(RequestedRPM-RPM) < 1){ //om mycket kommer hända och lite ska hända
    Speed += SpeedTerm/Derivering;
  }else if(abs(SpeedTerm) < 1 && abs(RequestedRPM-RPM) > 1){ //om lite kommer hända och mycket ska hända
    Speed += SpeedTerm*Derivering;
  } else {
    Speed += SpeedTerm;
  }
*/
  analogWrite(Pw, Speed);
}

void SendJSON(){
  String payload = "{\"RPM\":" + String(RPM,6) + ",\"Rev\":" + String(Rev) + ",\"Interval\":" + String((currentMillis-previousMillis)) + ",\"Pw\":" + String(Speed) + ",\"IntegrationTerm\":" + String(IntegrationTerm) + ",\"ProportionellTerm\":" + String(ProportionellTerm) + ",\"EasingTerm\":" + String(EasingTerm) + ",\"SpeedTerm\":" + String(SpeedTerm) + ",\"DeriveringTerm\":" + String(SpeedTerm/Derivering) + "}";
  Serial.println(payload);
  client.publish("oliver.witzelhelmin@abbindustrigymnasium.se/broom_broom", payload);
}

ICACHE_RAM_ATTR void HtoL(){
  Rev++;
}
