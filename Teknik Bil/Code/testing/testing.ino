#define D1 0
#define Pw 5
#define He 4

void setup() {
  Serial.begin(9600);
  pinMode(Pw, OUTPUT), pinMode(D1, OUTPUT);
  digitalWrite(D1, HIGH);
  analogWrite(Pw, 350);
}

void loop() {

}
