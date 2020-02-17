#define He 4

int Rev, Revs = 0;

void setup() {
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(He), HtoL, FALLING);
}

void loop() {
  if (Rev > Revs+10){
    Rev = Revs;
  }else{
    Revs = Rev;
  }
  Serial.println(Rev);
}

ICACHE_RAM_ATTR void HtoL(){
  Rev++;
}
