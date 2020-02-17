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
const int maxSpeed = 1023;
const int minSpeed = 0;
float RPM, Integration, Proportionell, Easing, Derivering, ProportionellSafety, RequestedRPM, Difference, SpeedTerm, DeriveringSafety, Fade, Distance;
int Rev, Speed, Interval, DeriveringInterval, MqttSlowRN, MqttSlow;
boolean go;
String payload;
int LedState = LOW;

void onConnectionEstablished() {
  client.publish("oliver.witzelhelmin@abbindustrigymnasium.se/broom_broom", "we aint groomin we vroomin");
  client.subscribe("oliver.witzelhelmin@abbindustrigymnasium.se/vroom_vroom", [] (const String & payload) {
    StaticJsonBuffer<200> JsonBuffer;
    JsonObject& root = JsonBuffer.parseObject(payload);
    if(root.success()) {
      go = root["Message"];                                                       // TESTVÄRDEN:
      Interval = root["Interval"];                                                // 1000
      DeriveringInterval = root["DeriveringInterval"];                            // 50
      Integration = root["Integration"];                                          // 0
      Proportionell = root["Proportionell"];                                      // 10
      Easing = root["Easing"];                                                    // 0
      Derivering = root["Derivering"];                                            // 1
      ProportionellSafety = root["ProportionellSafety"];                          // 1
      RequestedRPM = root["RequestedRPM"];                                        // 125
      MqttSlow = root["PubFrequency"];                                            // 10
      DeriveringSafety = root["DeriveringSafety"]; //123 LÄGG TILL I HEMSIDAN     // 
    }
  });
}

void setup() {
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(He), HtoL, FALLING);
  pinMode(Pw, OUTPUT), pinMode(D1, OUTPUT), pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(D1, HIGH);
}

void loop() {
  if(!client.isConnected()){
//  if(true){
    Speed = 0;
    analogWrite(Pw, Speed);
    ConnectionErr(10000);
  } else if (!go){
    Speed = 0;
    analogWrite(Pw, Speed);
    Stopped(1000);
  } else {
    GetRPM();
  }
  client.loop();
}

void GetRPM(){
  currentMillis = millis();
  if (currentMillis > (previousMillis + Interval)){
//    Distance += Rev * 1.210822;
    RPM = float(float(float(Rev)/float(96))/float(float(currentMillis)-float(previousMillis)) * 60000);
    Difference = RequestedRPM-RPM;
    payload = "{\"RPM\":" + String(RPM,6);
    integrationTerm();
    proportionellTerm();
    easingTerm();  
    deriveringTerm();  
    act();
    if (MqttSlowRN >= MqttSlow){
      SendJSON();
      MqttSlowRN = 0;
    }
    MqttSlowRN++;
    Rev = 0;
    currentMillis = millis();
    previousMillis = currentMillis;
  }
}

void integrationTerm(){
  SpeedTerm = Integration*pow(Difference, 3);
  addPayload("Integration");
}

void proportionellTerm(){
  if (Difference > ProportionellSafety){
    SpeedTerm = Proportionell;
  } else if (Difference < (-1*ProportionellSafety)) {
    SpeedTerm = (-1*Proportionell);
  } else {
    SpeedTerm = 0;
  }
  addPayload("Proportionell");
}

void easingTerm(){
  SpeedTerm = Easing * sqrt(abs(Difference));
  addPayload("Easing");
}

void deriveringTerm() {
  currentMillis = millis();
  previousMillis = currentMillis;
  while (currentMillis > (previousMillis + DeriveringInterval)){
    currentMillis = millis();
  }
  RPM = float(float(float(Rev)/float(96))/float(float(currentMillis)-float(previousMillis)) * 60000);
  Difference = RequestedRPM-RPM;
//  Distance += Rev * 1.210822;
  if(SpeedTerm*DeriveringSafety > Difference){ //om mycket kommer hända och lite ska hända
    Speed += SpeedTerm/Derivering - SpeedTerm;
    payload += (",\"SpeedTermWithDerivering\":" + String(SpeedTerm/Derivering));
  }else if(SpeedTerm*DeriveringSafety < Difference){ //om lite kommer hända och mycket ska hända
    Speed += SpeedTerm*Derivering - SpeedTerm;
    payload += (",\"SpeedTermWithDerivering\":" + String(SpeedTerm*Derivering));
  } else {
    payload += (",\"SpeedTermWithDerivering\":" + String(SpeedTerm));
    Speed += SpeedTerm;
  }
  payload += (",\"RPMrn\":" + String(RPM));
}

void addPayload(String Term){
  Speed += SpeedTerm;
  payload += (",\"" + Term + "\":" + String(SpeedTerm));
}

void SendJSON(){
  payload += (",\"PwAfter\":" + String(Speed) + ",\"Distance\":" + String(Distance) + "}");
  Serial.println(payload);
  client.publish("oliver.witzelhelmin@abbindustrigymnasium.se/broom_broom", payload);
}

void act(){
  if (Speed > maxSpeed){
    analogWrite(Pw, maxSpeed);
  } else if (Speed < minSpeed) {
    analogWrite(Pw, minSpeed);
  } else {
    analogWrite(Pw, Speed);
  }
}

void Stopped(int BlinkTime){
  currentMillis = millis();
  if (currentMillis > previousMillis + BlinkTime){
    previousMillis = currentMillis;
    LedState = !LedState;
    digitalWrite(LED_BUILTIN, LedState);
  }
}

void ConnectionErr(int FadeTime){
  currentMillis = millis();
  Fade = int(0.0255*(currentMillis-previousMillis));
  if(currentMillis > previousMillis + FadeTime){
    previousMillis = currentMillis;
  }
  Serial.println(String(Fade));
  analogWrite(LED_BUILTIN, Fade);
}

ICACHE_RAM_ATTR void HtoL(){
  Rev++;
  Distance += 1.210822;
}
