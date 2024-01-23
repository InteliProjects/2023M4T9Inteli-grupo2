#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <Arduino.h>
#include <DHT.h>
#include <SD.h>

#define CS_PIN 5     // Pino de Chip Select (CS)
#define MOSI_PIN 23   // Pino MOSI
#define MISO_PIN 15   // Pino MISO
#define SCK_PIN 18    // Pino Serial Clock


#define SSID "Inteli-COLLEGE"
#define PASSWORD "QazWsx@123"

#define MQTT_ID "VoltzBanana"
#define MQTT_BROKER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_TOPIC "esp32Voltz/test/"

#define DATA_PIN 25
#define CLOCK_PIN 33

#define RGB_R_PIN 27
#define RGB_G_PIN 12
#define RGB_B_PIN 13
#define BUZZER_PIN 32
#define BUTTON_TARE 19
#define BUTTON_TOGGLE_LCD 16
#define DHTPIN 4
#define DHTTYPE DHT11




DHT dht(DHTPIN, DHTTYPE);
float humidity;
float humidity_atual;
float temperature;
float temperature_atual;

const int outPutDevices[4] = { 27, 12, 13, 32 };

float weight;
char *topicValue;
int state;

LiquidCrystal_I2C lcd(0x27, 16, 2);
HX711 scale;

WiFiClient espClient;
PubSubClient MQTT(espClient);

bool lcdIsOn = true;  // Variável de controle para o estado do LCD

void initPins() {
  for (int i = 0; i < 4; i++) {
    pinMode(outPutDevices[i], OUTPUT);
  }
  pinMode(BUTTON_TARE, INPUT_PULLDOWN);
  pinMode(BUTTON_TOGGLE_LCD, INPUT_PULLDOWN);
}

void setupWIFI() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  } else {
    Serial.println();
    Serial.print("Connecting to ");
    lcd.setCursor(0, 0);
    lcd.print("Conectando ");
    lcd.print(SSID);
    Serial.println(SSID);
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP Address:");
    Serial.println(WiFi.localIP());
  }
}

void setupMQTT() {
  MQTT.setServer(MQTT_BROKER, MQTT_PORT);
  MQTT.setCallback(LCD_callback);
  while (!MQTT.connected()) {
    Serial.print("- MQTT Setup: Trying to connect to MQTT Broker: ");
    Serial.println(MQTT_BROKER);
    if (MQTT.connect(MQTT_ID)) {
      Serial.println("- MQTT Setup: Connected successfully");
      MQTT.subscribe(MQTT_TOPIC);
    } else {
      Serial.println("- MQTT Setup: Failed to connect, trying again in 2s");
      delay(2000);
    }
  }
}

void initSensors(byte dataPin, byte clockPin) {
  while (!scale.is_ready()) {
    Serial.println("Sensor not connected");
  }
  scale.begin(dataPin, clockPin);
  Serial.println("Sensor connected");
  scale.set_scale(-212097.487876);
  delay(1500);
  scale.tare();
  Serial.println("Scale Zeroed");
  dht.begin();
}

// void readTemperature() {
//   humidity = dht.readHumidity();
//   temperature = dht.readTemperature();

//   if (isnan(humidity) || isnan(temperature)) {
//     Serial.println("Failed to read DHT!!");
//   } else if (humidity_atual != humidity || temperature_atual != temperature) {
//     humidity_atual = humidity;
//     temperature_atual = temperature;
//     Serial.print("Temp.");
//     Serial.print(temperature);
//     Serial.print("°");
//     Serial.println("C");

//     Serial.print("Umid.");
//     Serial.print(humidity);
//     Serial.println("%");
//     Serial.println("====================");
//   }
// }

void toggleLCD() {
  lcdIsOn = !lcdIsOn;  // Inverte o estado do LCD

  if (lcdIsOn) {
    lcd.backlight();      // Ligue o LCD
    lcd.on();  // Ligue o backlight
  } else {
    lcd.off();  // Desligue o backlight
    lcd.noBacklight();        // Desligue o LCD
  }
}

void setup(void) {
  Serial.begin(115200);

  initPins();
  initSensors(DATA_PIN, CLOCK_PIN);
  lcd.init();
  lcd.backlight();
  setupWIFI();
  setupMQTT();
  MQTT.publish(MQTT_TOPIC, "SCD started");

  
  
}

void loop(void) {
  if (digitalRead(BUTTON_TARE) == HIGH) {
    scale.tare();
  }

  if (digitalRead(BUTTON_TOGGLE_LCD) == HIGH) {
    toggleLCD();  // Acione a função para ligar/desligar o LCD
    delay(500);   // Adicione um pequeno atraso para evitar detecção falsa de múltiplos pressionamentos
  }

  weight = getValues();
  char topLength[15];
  topicValue = dtostrf(weight, 5, 2, topLength);
  MQTT.publish(MQTT_TOPIC, topicValue);
  MQTT.loop();
  alerts();
  delay(250);
}

void LCD_callback(char *topic, byte *payload, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) {
    char c = (char)payload[i];
    msg += c;
  }
  if (msg.equals("write")) {
    lcd.setCursor(0, 1);
    lcd.print("Received Msg");
  }
  if (msg.equals("clear")) {
    lcd.clear();
  }
  Serial.println();
}

float getValues() {
  float load = scale.get_units(5);
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  if (load < 0) {
    load = 0;
  }


  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read DHT!!");
  } else if (humidity_atual != humidity || temperature_atual != temperature) {
    humidity_atual = humidity;
    temperature_atual = temperature;
    Serial.print("Temp.");
    Serial.print(temperature);
    Serial.print("°");
    Serial.println("C");

    Serial.print("Umid.");
    Serial.print(humidity);
    Serial.println("%");
  }
  // scale.power_down();
  // delay(100);
  // scale.power_up();
  Serial.println(load, 5);
  delay(20);
  showResult(load, humidity, temp);
  return load;
}

void showResult(float load, float humidity, float temp) {
  lcd.setCursor(0, 0);
  lcd.print("KG  ");
  lcd.print(load, 3);
  lcd.setCursor(0, 1);
  lcd.print(temp,2);
  lcd.print("°C");
  delay(250);
}

void alerts() {
  enum espState {
    Iddle,
    WiFiDisconnected,
    SensorDisconnected,
    AllDisconnected
  };

  long leitura = scale.read();

  if (leitura <= -22000) {
    Serial.println("Sensor Disconnected");
    state = SensorDisconnected;
  } else if (WiFi.status() == 1) {
    state = WiFiDisconnected;
    WiFi.reconnect();
  } else if (WiFi.status() == WL_CONNECTION_LOST && scale.is_ready() == true) {
    state = AllDisconnected;
  } else {
    state = Iddle;
  }

  if (state == 0) {
    analogWrite(RGB_G_PIN, 255);
    analogWrite(RGB_R_PIN, 0);
    analogWrite(RGB_B_PIN, 0);
  } else if (state == 1) {
    analogWrite(RGB_G_PIN, 255);
    analogWrite(RGB_R_PIN, 220);
    analogWrite(RGB_B_PIN, 0);
  } else if (state == 2) {
    analogWrite(RGB_B_PIN, 255);
    analogWrite(RGB_R_PIN, 0);
    analogWrite(RGB_G_PIN, 0);
  } else if (state == 3) {
    analogWrite(RGB_R_PIN, 255);
    analogWrite(RGB_G_PIN, 0);
    analogWrite(RGB_B_PIN, 0);
  } else {
    analogWrite(RGB_R_PIN, 255);
    delay(250);
    analogWrite(RGB_R_PIN, 0);
    delay(250);
  }

  if (getValues() >= 0.5) {
    analogWrite(BUZZER_PIN, 15);
  } else {
    analogWrite(BUZZER_PIN, 0);
  }
}
