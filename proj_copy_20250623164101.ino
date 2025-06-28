#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SD.h>
#include <SPI.h>

// === Déclaration des broches ===
const int trigPin = 8;
const int echoPin = 9;
const int servoPin = 6;
const int pot = A0;

const int redLedPin = 2;
const int greenLedPin = 3;
const int buzzerPin = 4;
const int chipSelect = 10;

// === Objets ===
Servo myServo;
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Adresse I2C typique

// === Variables ===
long duration;
int distance;
int seuil;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  myServo.attach(servoPin);

  lcd.init();
  lcd.backlight();

  if (!SD.begin(chipSelect)) {
    lcd.setCursor(0, 0);
    lcd.print("Erreur SD !");
    while (1);  // Stoppe tout si SD ne marche pas
  }

  lcd.setCursor(0, 0);
  lcd.print("Radar en marche");
  delay(1000);
  lcd.clear();

  Serial.begin(9600);
}

void loop() {
  // Balayage aller
  for (int angle = 6; angle <= 175; angle++) {
    myServo.write(angle);
    delay(30);

    distance = getDistance();
    seuil = map(analogRead(pot), 0, 1023, 5, 40);  // Mis ici, mis à jour dynamiquement

    updateLCD(angle, distance);
    updateIndicators(distance);
    logToSD(angle, distance);
    serialOutput(angle, distance);
  }

  // Balayage retour
  for (int angle = 175; angle >= 6; angle--) {
    myServo.write(angle);
    delay(30);

    distance = getDistance();
    seuil = map(analogRead(pot), 0, 1023, 5, 40);  // Mis ici aussi

    updateLCD(angle, distance);
    updateIndicators(distance);
    logToSD(angle, distance);
    serialOutput(angle, distance);
  }
}

int getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2;  // Résultat en cm
}

void updateLCD(int angle, int dist) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ang:");
  lcd.print(angle);
  lcd.print(" D:");
  lcd.print(dist);
  lcd.print("cm");

  lcd.setCursor(0, 1);
  if (dist < seuil) {
    lcd.print("Obstacle detecte");
  } else {
    lcd.print("Seuil: ");
    lcd.print(seuil);
  }
}

void updateIndicators(int dist) {
  if (dist < seuil) {
    digitalWrite(redLedPin, HIGH);
    digitalWrite(greenLedPin, LOW);
    tone(buzzerPin, 1000);
  } else {
    digitalWrite(redLedPin, LOW);
    digitalWrite(greenLedPin, HIGH);
    noTone(buzzerPin);
  }
}

void logToSD(int angle, int dist) {
  File dataFile = SD.open("ik.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.print("Angle:;");
    dataFile.print("Distance:\n");
    dataFile.print(angle);
    dataFile.print("deg;");
    dataFile.print(dist);
    dataFile.println("cm");
    dataFile.close();
  }
}

void serialOutput(int angle, int dist) {
  Serial.print(angle);
  Serial.print(",");
  Serial.println(dist);
}