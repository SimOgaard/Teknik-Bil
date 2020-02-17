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

int LedState = LOW;

unsigned long previousMillis, currentMillis = 0;

const int maxSpeed = 1024;
const int minSpeed = 0;
float RPM, Rrn;
int Rev, Pub, SpeedTerm, IntegrationTerm, ProportionellTerm, EasingTerm;
int Speed = 0;

float PubFrequency, Interval, DeriveringInterval, Integration, Proportionell, Easing, Derivering, ProportionellSafety, RequestedRPM;

boolean go;

String payload;

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
  pinMode(Pw, OUTPUT), pinMode(D1, OUTPUT), pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(D1, HIGH);
}

void loop() {
  if(!isConnected()){
    Speed = 0;
    analogWrite(Pw, Speed);
    ConnectionErr(1000);
  } else if (!go){
    Speed = 0;
    analogWrite(Pw, Speed);
    Stopped(1000);
  } else {
    GetRPM(Interval);
  }
  client.loop();
}

void GetRPM(int IntervalValue){ //DERIVERING, SENDJSON and RPM me no likey
  currentMillis = millis();
  if (currentMillis > (previousMillis + IntervalValue)){
    RPM = float(float(Rev)/float(currentMillis-previousMillis));
    AccToRPM();
    if (SlowPub >= PubFrequency) {
      SendJSON();
      SlowPub = 0;
    }
    SlowPub++;
    Rev = 0;
    currentMillis = millis();
    previousMillis = currentMillis;
  }
}

void GetRPMrn(int InterValue){
  Rev = 0;
  delay(InterValue);
  RPM = float(float(Rev)/float(InterValue));
}

void AccToRPM(){
  Difference = RequestedRPM-RPM;
  IntegrationTerm = Integration*pow(Difference, 3); //Integration

  if (Difference > ProportionellSafety){ //Proportionell 
    ProportionellTerm = Proportionell;
  } else if (Difference < (-1*ProportionellSafety)) {
    ProportionellTerm = (-1*Proportionell);
  }

  EasingTerm = Easing * sqrt(abs(Difference)); //Easing
  
  SpeedTerm = IntegrationTerm+ProportionellTerm+EasingTerm; //Derivering

  payload = "{\"RPM\":" + String(RPM,6) + ",\"Rev\":" + String(Rev) + ",\"Interval\":" + String((currentMillis-previousMillis)) + ",\"Pw\":" + String(Speed) + ",\"IntegrationTerm\":" + String(IntegrationTerm) + ",\"ProportionellTerm\":" + String(ProportionellTerm) + ",\"EasingTerm\":" + String(EasingTerm) + ",\"SpeedTerm\":" + String(SpeedTerm);
  
  GetRPMrn(DeriveringInterval);
  Difference = RequestedRPM-RPM;
  if(SpeedTerm > Difference*DeriveringTerm){ //om mycket kommer hända och lite ska hända
    Speed += SpeedTerm/Derivering;
  }else if(SpeedTerm < Difference*DeriveringTerm){ //om lite kommer hända och mycket ska hända
    Speed += SpeedTerm*Derivering;
  } else {
    Speed += SpeedTerm;
  }

  payload += (",\"RPMrn\":" + String(RPM) + ",\"IntervalDerivering\":" + String(DeriveringInterval)) + ",\"RevDerivering\":" + String(Rev) + ",\"Speed\":" + String(Speed) + "}");

  if (Speed > maxSpeed){
    analogWrite(Pw, maxSpeed);
  } else if (Speed < minSpeed) {
    analogWrite(Pw, minSpeed);
  } else {
    analogWrite(Pw, Speed);
  }
}

void SendJSON(){
  Serial.println(payload);
  client.publish("oliver.witzelhelmin@abbindustrigymnasium.se/broom_broom", payload);
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
  if (currentMillis > previousMillis + FadeTime){
    previousMillis = currentMillis;
    LedState = !LedState;
  }
  if(LedState == LOW && LedStateSmooth <= 255){
    LedStateSmooth++;
  }else{
    LedStateSmooth--;
  }
  digitalWrite(LED_BUILTIN, LedStateSmooth);
  delay(FadeTime/255);
}

ICACHE_RAM_ATTR void HtoL(){
  Rev++;
}
