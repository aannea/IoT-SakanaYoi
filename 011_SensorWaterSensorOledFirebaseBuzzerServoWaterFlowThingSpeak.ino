#include <DallasTemperature.h>
#include <OneWire.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>
#include <Servo.h>
#include <ThingSpeak.h>

#define OLED_ADDR 0x3C
#define OLED_SDA D3
#define OLED_SCL D4
#define BUZZER_PIN D6
#define ONE_WIRE_BUS D5
#define SERVO_PIN D7

const int sensorPin = A0;
const int flowSensorPin = D0;
const char* ssid = "";
const char* password = "";
const char* host = "";
const char* auth = "";
unsigned int flowPulse = 0;
unsigned long currentTime;
unsigned long interval = 1000;
float flowRate = 0;
float totalLiters = 0;
unsigned long myChannelNumber = ;
const char * myWriteAPIKey = "";

WiFiClient client;
FirebaseData firebaseData;
Servo servo;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
Adafruit_SSD1306 display(128, 64, &Wire, OLED_ADDR);
int i = 0;

void setup(void)
{
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.setCursor(28, 20);
  display.print("Connecting!!");
  pinMode(BUZZER_PIN, OUTPUT);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    display.setCursor(i, 28);
    display.print("*");
    display.display();
    i += 4;
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
    Serial.print(".");
  }
  delay(1000);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(1000);
  display.clearDisplay();
  display.setCursor(18, 20);
  display.print("WiFi Connected!");
  display.display();
  digitalWrite(BUZZER_PIN, LOW);
  
  ThingSpeak.begin(client);
  Firebase.begin(host, auth);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  sensors.begin();
  servo.attach(SERVO_PIN);
  servo.write(0);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(flowSensorPin, INPUT);
}

void loop(void)
{
  currentTime = millis();
  flowPulse = pulseIn(flowSensorPin, HIGH);
  flowRate = (flowPulse / 7.5) * 60;
  totalLiters += (flowRate / 1000) * (interval / 1000); 
  
  float temperature;
  int sensorValue = analogRead(sensorPin);
  bool hasWater = sensorValue > 270;
  String status = hasWater ? "Normal" : "Kurang";
  
  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextColor(WHITE);
  display.println("Air: " + status);
  display.setCursor(0, 16);
  display.setTextColor(WHITE);
  display.print("Temperature : ");
  display.print(temperature);
  display.println(" C");
  display.setCursor(0, 32);
  display.setTextColor(WHITE);
  display.println("Aliran : " + String(flowRate) + " LPM");
  display.setCursor(0, 48);
  display.setTextColor(WHITE);
  display.println("Total Aliran : " + String(totalLiters) + " L");
  display.display();
  
  Firebase.setFloat(firebaseData, "/sensorFlow", flowRate);
  Firebase.setFloat(firebaseData, "/sensorFlowTotal", totalLiters);
  Firebase.setFloat(firebaseData, "/sensorSuhu", temperature);
  Firebase.setString(firebaseData, "/sensorTinggi", status);
  if (Firebase.getBool(firebaseData, "/makan"))
  {
    if (firebaseData.dataAvailable())
    {
      bool makan = firebaseData.boolData();
      if (makan)
      {
        rotateServo();
        delay(500);
        resetServo();
        Firebase.setBool(firebaseData, "/makan", false);
      }
    }
  }
  ThingSpeak.writeField(myChannelNumber, 1, temperature, myWriteAPIKey);
  ThingSpeak.writeField(myChannelNumber, 2, flowRate, myWriteAPIKey);
  delay(1000);
}

void rotateServo()
{
  for (int pos = 0; pos <= 180; pos += 1)
  {
    servo.write(pos);
    delay(15);
  }
}

void resetServo()
{
  for (int pos = 180; pos >= 0; pos -= 1)
  {
    servo.write(pos);
    delay(15);
  }
}
