// Biblotek //
#include "EspMQTTClient.h"  //Hantering av hämtning/skickning av data (Mqtt)
#include <ArduinoJson.h>    //Hantering av strängar i json format

// Pins //
#define D1 0 // Motor Dir pin
#define Pw 5 // Motor pin
#define He 4 // Hallelement input pin

// Variabler //
String OWNER="O";                                                         // (Sträng) Ändra till S eller O beroende på vems bil
unsigned long previousMillis, currentMillis = 0;                          // (Unsigned long) för användningen av millis (pga datatypens storlek)
const int maxPwm = 1023;                                                  // (Constant int) Högsta tillgängliga motor värdet 
const int minPwm = 0;                                                     //      -||-      Minsta           -||-
float RPM, Ki, Kp, RequestedRPM, e, Pwm, KiArea, Kd, DistanceDriven, Dist;// (Float) För högre presition av inskickta samt uträknade variabler/värden 
int Rev, Interval, MqttSlowRN, MqttSlow;                                  // (Int) För heltal
String payload, payloadArray, Task;                                       // (Sträng) Strängar som vi kan Jsonifiera
int LedState = LOW;                                                       // (Int) Lampans status

// Mqtt //
void onConnectionEstablished();                 // Krävs för att bibloteket ska fungera när denna har körts klart är du säker på att du är ansluten

EspMQTTClient client(                           // Alla parametrar för att anslutningen ska funka, ip, namn, lösen osv
  "ABB_Indgym_Guest",                           //
  "Welcome2abb",                                //
  "maqiatto.com",                               //
  1883,                                         //
  "oliver.witzelhelmin@abbindustrigymnasium.se",//
  "vroom",                                      //
  "broom_broomO",                               // Ändra till S eller O beroende på vems bil
  onConnectionEstablished,                      //
  true,                                         //
  true                                          //
);

void onConnectionEstablished() {
  client.publish("oliver.witzelhelmin@abbindustrigymnasium.se/broom_broom"+OWNER, OWNER + " aint groomin he bauta vroomin");  // Vid lyckad anslutning skicka medelandet till representerande adress
  client.subscribe("oliver.witzelhelmin@abbindustrigymnasium.se/vroom_vroom"+OWNER, [] (const String & payload) {
    StaticJsonBuffer<500> JsonBuffer;                                                       // Skapar en buffer, hur mycket minne som vårt blivand jsonobject får använda
    JsonObject& root = JsonBuffer.parseObject(payload);                                     // Skapar ett jsonobject av datan payload som vi kallar root
    if(root.success() && root["V"][0] == OWNER || root.success() && root["V"][0] == "A" ) { // Om ovan lyckas och Jsonobjekten är pointerat till "A" (alla) eller "S"/"O" (representativ till variabeln "OWNER")
      Task = root["V"][1].asString(); // Konstanterar variablernas värden (Dem behlålls samma till yttligare inskickning av data)
      Interval = root["V"][2];        // -||-
      Ki = root["V"][3];              // -||-
      Kp = root["V"][4];              // -||-
      Kd = root["V"][5];              // -||-
      RequestedRPM = root["V"][7];    // -||-
      MqttSlow = root["V"][6];        // -||-
      Dist = root["V"][8];            // -||-
      currentMillis = millis(), previousMillis = millis();              // Reset av värden
      Rev = 0, RPM = 0, e = 0, Pwm = 0, KiArea = 0, DistanceDriven = 0; //     -||-
    }
  });
}

// Setup //
void setup() {
  Serial.begin(9600);                                                     // Sätter datarate i bits per sekund för serial data överförning (9600 bits per sekund)
  attachInterrupt(digitalPinToInterrupt(He), HtoL, FALLING);              // Digitala pin med interuppt till pin "He" funktionen "HtoL" ska köras vid "Falling" högt värde till lågt 
  pinMode(Pw, OUTPUT), pinMode(D1, OUTPUT), pinMode(LED_BUILTIN, OUTPUT); // Konfigurerar pins att bete sig som outputs
  digitalWrite(D1, HIGH);                                                 // Skriver till pin "D1" hög volt (3.3v) 
}

// Loop //
void loop() {
  client.loop();              // När den kallas tillåter du klienten att processera inkommande medelanden, skicka data och bibehålla anslutningen:
  if(!client.isConnected()){  // Om funtkionen "client.isConnected()" get False, då har klienten tappat anslutningen
    stopped(500);             //    Anropa funktionen stopped() med parametern Blinktime sätt till 500 ms för att kunna förtydliga felet utan serialmonitor medelande
  } else if (Task == "R") {   // Om vi vill komma åt en distance:
    reachDistance();          //    Anropa funktionen reachDistance()
  } else if (Task == "A"){    // Om vi vill hålla i en hastighet;
    attainRPM();              //    Anropa funktionen attainRPM() 
  } else {                    // Annars:
    stopped(2500);            //    Anropa funktionen stopped() med parametern Blinktime sätt till 500 ms för att kunna förtydliga felet utan serialmonitor medelande
  }
}

// Funktioner "States" //
void reachDistance(){
  attainRPM();                      // Anropa funktionen attainRPM() 
  if (DistanceDriven >= Dist*1000){ // Om variabeln DistanceDriven är större än eller lika med eftersökta distansen * 1000 (pga enhets omvandling):
    Task = "S";                     //    Ändra variabeln Task till en ny uppgift (Stanna)
  }
}

void attainRPM(){
  if (getRPM()){            // Om funktionen getRPM ger tillbaka True:
    calculateTerms();       //    Anropa funktionen calculateTerm() 
    Pwm = speedControll();  //    Sätt Pwm till rurnerat värde av funktionen speedControll()
    analogWrite(Pw, Pwm);   //    Skriver till pin "Pw" värde från 0 - 1023 (0 representerar lågt, 1023 representerar högt)
    sendJSON();             //    Anropa funktionen sendJSON() 
  }
}

// Funktion "Skaffa data" //
boolean getRPM(){
  currentMillis = millis();                                         // Sätter variabeln currentMillis till millis();
  if (currentMillis > (previousMillis + Interval)) {                // Om currentMillis är större än förra + Interval:
    RPM = (float) Rev / 96 / (currentMillis-previousMillis) * 60000;// Räknar ut Rpm de senaste (Interval) ms
    e = RequestedRPM-RPM;                                           // Räknar ut differansen av Rpm
    KiArea += (float) (currentMillis-previousMillis)/1000*e;        // Räknar ut Arean och adderar det på variabeln
    Rev = 0;                                                        // Reset av värden
    previousMillis = currentMillis;                                 // Sätter variabeln previousMillis till currentMillis
    payload = "{\""+OWNER+"\":{\"RPM\":\"" + String(RPM)+"\"";      // Sätter strängen "payload" till representiv sträng i jsonformat
    payloadArray = "{\""+OWNER+"\":[\""+String(RPM)+"\",";          // Sätter strängen "payload" till representiv stäng i jsonformat (för mindre data)
    return true;                                                    // Returnerar True
  }
  return false;                                                     // Returnerar False
}

// Funktioner "Använd data" //
void calculateTerms(){
  Pwm = 0;                              // Resetar Pwm så den framtida additionen av värdet proportionellTerm() blir som =, och inte +=
  addPayload("Kp", proportionellTerm());// Anropa funktionen addPayload() med parameterna "strTerm" Kp, "intTerm" funktionen "proportionellTerm()" returnerande värde 
  addPayload("Ki", integrationTerm());  // Anropa funktionen addPayload() med parameterna "strTerm" Ki, "intTerm" funktionen "integrationTerm()" returnerande värde 
  addPayload("Kd", deriveringTerm());   // Anropa funktionen addPayload() med parameterna "strTerm" Kd, "intTerm" funktionen "deriveringTerm()" returnerande värde 
}

void addPayload(String strTerm, float intTerm){
  Pwm += intTerm;                                               // Lägger till "intTerm" i "Pwm" värdet (motorns styrka)
  payload += (",\"" + strTerm + "\":\"" + String(intTerm)+"\"");// Lägger till strängen "strTerm" och "intTerm" till payload i jsonformat
  payloadArray += ("\"" + String(intTerm) + "\",");             // Lägger till strängen "intTerm" till payloadArray i jsonformat (för mindre data)
}

void proportionellTerm(){
  return (float) Kp * e;      // Returnerar flytvärdet av konstanten "Kp" multiplicerat med "e" differansen av Rpm
}

float integrationTerm(){
  return (float) Ki * KiArea; // Returnerar flytvärdet av konstanten "Ki" multiplicerat med "KiArea" grafens "volym"
}

float deriveringTerm() {
  return 0;                   // Returnerar 0, hann inte med deriverandeTermen
}

// Funktion "Kontrolera hastighet" //
float speedControll(){
  if (Pwm > maxPwm){        // Om Pwm är större än största tilllåta värdet:
    return maxPwm;          //    Returnerar största tilllåta värdet
  } else if (Pwm < minPwm) {// Om Pwm är mindre än minsta tillåtna värdet:
    return minPwm;          //    Returnerar minsta tillåtna värdet
  } else {                  // Annars:
    return int(Pwm+0.5);    //    Returnerar avrundande värdet av flyttalet Pwm (användningen av +0.5 motverkar kodens bristfälliga "avrundning" från flyttal till int)
  }
}

// Funktion "Skicka data" //
void sendJSON(){
  MqttSlowRN++; // Lägger på 1 på variabeln MqttSlowRN 
  payload += (",\"DistanceDriven\":\"" + String(DistanceDriven) + "\",\"Pw\":\"" + String(Pwm) + "\",\"CalculationTime\":\"" + String(millis()-previousMillis) + "}}"); // Lägger till Strängen till payload i jsonformat
  payloadArray += ("\""+String(millis())+"\"]}"); // Lägger till millis() till payloadArray i jsonformat (för mindre data)
  Serial.println(payload);                        // Skriver till serialmonitor vårat fina mer informationrika data
  if (MqttSlowRN >= MqttSlow){                    // Om denna funktion har anropats "MqttSlow" antal gånger:
    client.publish("oliver.witzelhelmin@abbindustrigymnasium.se/broom_broom"+OWNER, payloadArray);  // Skickar våran konsentrerade data
    MqttSlowRN = 0; // Reset av värdet
  }
}

// Funktion "Stannad" //
void stopped(int BlinkTime){
  analogWrite(Pw, 0);                             // Skriver till pin "Pw" värdet 0 (0 representerar lågt)
  currentMillis = millis();                       // Sätter variabeln currentMillis till millis();
  if (currentMillis > previousMillis + BlinkTime){// Om currentMillis är större än förra + parametern BlinkTime:
    previousMillis = currentMillis;               //    Sätter variabeln previousMillis till currentMillis
    LedState = !LedState;                         //    "Nottar Ledstate", sätter LedState till motsatta värdet det var innan
    digitalWrite(LED_BUILTIN, LedState);          //    Skriver till inbyggda led lampan LedState (3.3v eller 0v) 
  }
}

// Funktion "Interupt" //
ICACHE_RAM_ATTR void HtoL(){
  Rev++;                  // Lägger på 1 på variabeln Rev
  DistanceDriven += 1.23; // Lägger på 1.23 på variabeln DistanceDriven (1.23 pga hjulets diameter, motorns växellåda och magneternas ("+" -> "-" -> "+" -> "-") poler)
}
