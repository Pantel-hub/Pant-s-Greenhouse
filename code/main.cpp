#include <LiquidCrystal_I2C.h>
#include <Servo.h>

int lcdColumns = 16;
int lcdRows = 2;
int nProgramState;
int yellowbutton = 6;
int redbutton = 7;
int servoPin = 9;
int servoPos;
int motorIn1Pin = 11;
int motorIn2Pin = 5;
int LED1Pin = 2;
int LED2Pin = 3;

Servo myservo;
#define sensorPin A2
#define ldrPin A0

float targetTemp = 10;
LiquidCrystal_I2C lcd(0x27, 16, 2);
String messageToScroll = "GreenHouse Program";

const int LDR_ANALOG_MAX_VOLTAGE = 1023;
const float MAX_LUX = 200;
float Ev_max = MAX_LUX;
float Ev_half = Ev_max / 2;
float T_set_half;

float readLightSensorLux() {
  int ldrAnalogValue = analogRead(ldrPin);
  float lux = map(ldrAnalogValue, 0, LDR_ANALOG_MAX_VOLTAGE, 0, MAX_LUX);
  if (lux < 0) lux = 0;
  if (lux > MAX_LUX) lux = MAX_LUX;
  return lux;
}

float readTemperatureC(int pin) {
  int analogValue = analogRead(pin);
  Serial.print("Raw Analog Value (Temp Sensor): ");
  Serial.println(analogValue);
  float temperatureC = analogValue * 0.36977;
  Serial.print("Calculated Temp (Adjusted): ");
  Serial.println(temperatureC, 1);
  return temperatureC;
}

void scrollText(int row, String message, int delayTime, int lcdColumns) {
  for (int i = 0; i < lcdColumns; i++) {
    message = " " + message;
  }
  message = message + " ";
  for (int pos = 0; pos < message.length(); pos++) {
    lcd.setCursor(0, row);
    lcd.print(message.substring(pos, pos + lcdColumns));
    delay(delayTime);
  }
}

void readTemperatureSetting() {
  lcd.setCursor(3, 0);
  lcd.print("GreenHouse");
  lcd.setCursor(5, 1);
  lcd.print("Control");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Set Temp Limit");
  lcd.setCursor(0, 0);
  scrollText(0, messageToScroll, 250, lcdColumns);
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press SW1/SW2 to");
  lcd.setCursor(0, 1);
  lcd.print("+/Enter Temp:");
  lcd.setCursor(14, 1);
  lcd.print(targetTemp);
  while (digitalRead(redbutton) != HIGH) {
    if (digitalRead(yellowbutton) == HIGH) {
      targetTemp = targetTemp + 1;
      if (targetTemp == 61)
        targetTemp = 10;
      delay(500);
      lcd.setCursor(0, 0);
      lcd.print("Press SW1/SW2 to");
      lcd.setCursor(14, 1);
      lcd.print(targetTemp);
      lcd.setCursor(0, 1);
      lcd.print("+/Enter Temp:");
    }
  }
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Greenhouse");
  lcd.setCursor(2, 1);
  lcd.print("Control: ON");
  delay(1000);
  nProgramState = 1;
}

void setup() {
  Serial.begin(9600);
  pinMode(yellowbutton, INPUT);
  pinMode(redbutton, INPUT);
  pinMode(ldrPin, INPUT);
  pinMode(sensorPin, INPUT);
  myservo.attach(servoPin);
  lcd.init();
  lcd.clear();
  lcd.backlight();
  pinMode(LED2Pin, OUTPUT);
  pinMode(LED1Pin, OUTPUT);
  pinMode(motorIn1Pin, OUTPUT);
  pinMode(motorIn2Pin, OUTPUT);
  myservo.write(180);
  delay(15);
  nProgramState = 0;
  readTemperatureSetting();
}

void loop() {
  switch (nProgramState) {
    case 0:
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("Greenhouse");
      lcd.setCursor(2, 1);
      lcd.print("Control: OFF");
        digitalWrite(motorIn1Pin, LOW);
        digitalWrite(motorIn2Pin, LOW);
        digitalWrite(LED2Pin, LOW);
      digitalWrite(LED1Pin, LOW);
      delay(3000);
      readTemperatureSetting();
      break;
    case 1:
      if (digitalRead(redbutton) == HIGH) {
        nProgramState = 0;
        break;
      }
      float currentTemp = readTemperatureC(sensorPin);
      float luxValue = readLightSensorLux();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Luminan:");
      lcd.print(luxValue, 1);
      lcd.print("Lux");
      lcd.setCursor(3, 1);
      lcd.print("Temp:");
      lcd.print(currentTemp, 1);
      lcd.print((char)223);
      lcd.print("C ");
      if (luxValue > Ev_half) {
        servoPos = 0;
        myservo.write(servoPos);
        delay(50);
        digitalWrite(LED2Pin, LOW);
      }
      if (luxValue < Ev_half) {
        servoPos = map(luxValue, 0, 200, 0, 180);
        myservo.write(servoPos);
        delay(50);
        digitalWrite(LED2Pin, HIGH);
      }
      T_set_half = targetTemp / 2;
      if (currentTemp > T_set_half) {
        digitalWrite(motorIn1Pin, HIGH);
        digitalWrite(motorIn2Pin, LOW);
        digitalWrite(LED1Pin, HIGH);
      } else if(currentTemp < T_set_half) {
        digitalWrite(motorIn1Pin, LOW);
        digitalWrite(motorIn2Pin, LOW);
        digitalWrite(LED1Pin, LOW);
      }
      delay(800);
      break;
  }
} 
