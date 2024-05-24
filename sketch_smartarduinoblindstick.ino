#include <SoftwareSerial.h>
#include <TinyGPS.h>

#define SPKPin 11
#define BuzzPin 13
#define MoisPin A0
#define GPSRX 3
#define GPSTX 4
#define UTrigPin 5
#define UEchoPin 6
#define EMERGENCY_BUTTON 7
#define GSM_TX 10
#define GSM_RX 12

TinyGPS gps;
SoftwareSerial serialGPS(GPSRX, GPSTX);
SoftwareSerial serialESP(8, 9);
SoftwareSerial gsmModule(GSM_TX, GSM_RX);

int threshold = 500; // Adjust threshold as needed for detecting voice playback

void setup() {
  Serial.begin(115200);
  serialGPS.begin(9600);
  serialESP.begin(115200);
  gsmModule.begin(9600); // Change baud rate if needed
  pinMode(SPKPin, OUTPUT);
  pinMode(UTrigPin, OUTPUT);
  pinMode(UEchoPin, INPUT);
  pinMode(EMERGENCY_BUTTON, INPUT_PULLUP);
}

void loca() {
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;

  for (unsigned long start = millis(); millis() - start < 1000;) {
    while (serialGPS.available()) {
      char c = serialGPS.read();
      if (gps.encode(c))
        newData = true;
    }
  }

  if (newData) {
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

void beep() {
  digitalWrite(BuzzPin, HIGH);
  delay(100);
  digitalWrite(BuzzPin, LOW);
  delay(100);
}

void playVoice() {
  digitalWrite(SPKPin, HIGH); // Trigger playback
  delay(100); // Adjust delay as needed
  digitalWrite(SPKPin, LOW);
}

void dist() {
  int duration, distance;
  digitalWrite(UTrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(UTrigPin, LOW);
  duration = pulseIn(UEchoPin, HIGH);
  distance = (duration / 2) / 29.1;
  if (distance < 50) {
    Serial.println("Obstacle detected");
    playVoice();
  }
}

void mois() {
  int moisture;
  moisture = analogRead(MoisPin);
  if (moisture < 800) {
    Serial.println("Water detected");
    playVoice();
  }
}

void checkEmergencyButton() {
  if (digitalRead(EMERGENCY_BUTTON) == LOW) {
    loca(); // Get current location
    Serial.println("Emergency Button Pressed!");
    float flat, flon;
    unsigned long age;
    gps.f_get_position(&flat, &flon, &age); // Retrieve latitude and longitude
    gsmModule.println("AT+CMGF=1"); // Set SMS mode to text
    delay(100);
    gsmModule.println("AT+CMGS=\"+918306708847\""); // Replace with recipient's phone number
    delay(100);
    gsmModule.print("Emergency! Current Location: LAT=");
    gsmModule.print(flat, 6);
    gsmModule.print(" LON=");
    gsmModule.print(flon, 6);
    //gsmModule.print(". ESP32 IP Address: ");
    //gsmModule.print(esp32IPAddress);
    gsmModule.write(26); // End of message character
    Serial.println("Message Sent!");
    //delay(5000); // Wait for SMS to be sent
  }
}

void loop() {
  loca();
  dist();
  mois();
  checkEmergencyButton();
  while (serialESP.available() > 0) {
    char inByte = serialESP.read();
    Serial.write(inByte);
  }
  while (gsmModule.available() > 0) {
    char inByte = gsmModule.read();
    Serial.write(inByte);
  }
}
