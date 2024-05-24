#include <SoftwareSerial.h>
#include <TinyGPS.h>


#define SPKPin 11
#define MoisPin A0
#define GPSRX 3
#define GPSTX 4
#define UTrigPin 5
#define UEchoPin 6


TinyGPS gps;
SoftwareSerial serialGPS(GPSRX, GPSTX);
SoftwareSerial serialESP(8, 9);


void setup() {
  Serial.begin(115200);
  serialGPS.begin(9600);
  serialESP.begin(115200);
  pinMode(SPKPin, OUTPUT);
  pinMode(UTrigPin, OUTPUT);
  pinMode(UEchoPin, INPUT);
  

}

void loca(){
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;

  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (serialGPS.available())
    {
      char c = serialGPS.read();
      // Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }

  if (newData)
  {
    float flat, flon;
    unsigned long age;
    gps.f_get_position(&flat, &flon, &age);
    Serial.print("LAT=");
    Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
    Serial.print(" LON=");
    Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
    Serial.print(" SAT=");
    Serial.print(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
    Serial.println("");

  }
}

void beep(){
  digitalWrite(SPKPin, HIGH);
  delay(100); // 100ms delay
  digitalWrite(SPKPin, LOW);
  delay(100);
  //BEEP!
}

void dist(){
  int duration, distance;
  digitalWrite(UTrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(UTrigPin, LOW);
  duration = pulseIn(UEchoPin, HIGH);
  distance = (duration/2)/29.1;
  while (distance<50){
    digitalWrite(UTrigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(UTrigPin, LOW);
    duration = pulseIn(UEchoPin, HIGH);
    distance = (duration/2)/29.1;
    Serial.println("OBSTACLE DETECTED!");
    Serial.println(distance);
    beep();
  }
}

void mois(){
  int moisture;
  moisture  = analogRead(MoisPin);
  if (moisture<800){
    beep();
    beep();
    beep();
    Serial.println("WATER DETECTED!");
    Serial.println(moisture);
  }
}

void loop() {
  loca();
  dist();
  mois();
  while (serialESP.available() > 0) {
    char inByte = serialESP.read();
    Serial.write(inByte);
  }
}
