#define D1 0
#define Pw 5
#define He 4

unsigned long previousMillis, currentMillis = 0;
const int maxSpeed = 1023;
const int minSpeed = 0;
float RPM = 0;
float Integration = 0;
float Proportionell = 5;
float Easing = 0;
float Derivering = 0;
float ProportionellSafety = 0.1;
float RequestedRPM = 1.5;
float Difference;
float SpeedTerm;
int Rev;
int Speed = 325;
int Interval = 1000; 
int DeriveringInterval = 100;
String payload;

void setup() {
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(He), HtoL, FALLING);
  pinMode(Pw, OUTPUT), pinMode(D1, OUTPUT), pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(D1, HIGH);
}

void loop() {
  GetRPM();
}

void GetRPM(){
  currentMillis = millis();
  if (currentMillis > (previousMillis + Interval)){
    RPM = float(float(float(Rev)/float(96))/float(float(currentMillis)-float(previousMillis)) * 60000);
    Difference = RequestedRPM-RPM;
    payload = "{\"RPM\":" + String(RPM,6);
//    integrationTerm();
//    proportionellTerm();
//    easingTerm();  
//    deriveringTerm();  
    act();
    SendJSON();
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

void deriveringTerm() { //123
  currentMillis = millis();
  previousMillis = currentMillis;
  while (currentMillis > (previousMillis + DeriveringInterval)){
    currentMillis = millis();
  }
  RPM = float(float(float(Rev)/float(96))/float(float(currentMillis)-float(previousMillis)) * 60000);
  Difference = RequestedRPM-RPM;

  if(SpeedTerm > Difference){ //om mycket kommer hända och lite ska hända
    Speed += SpeedTerm/Derivering;
  }else if(SpeedTerm < Difference){ //om lite kommer hända och mycket ska hända
    Speed += SpeedTerm*Derivering;
  } else {
    Speed += SpeedTerm;
  }
}

void addPayload(String Term){
  Speed += SpeedTerm;
  payload += (",\"" + Term + "\":" + String(SpeedTerm));
}

void SendJSON(){
  payload += (",\"PwAfter\":" + String(Speed)+"}");
  Serial.println(payload);
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

ICACHE_RAM_ATTR void HtoL(){
  Rev++;
}

