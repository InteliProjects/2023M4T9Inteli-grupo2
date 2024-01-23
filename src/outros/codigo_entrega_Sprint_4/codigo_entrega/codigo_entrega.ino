#include <LiquidCrystal_I2C.h>
#include <UbidotsEsp32Mqtt.h>
#include <Arduino.h>
#include <HX711.h>
#include <Wire.h>
#include <WiFi.h>
#include <DHT.h>
#define CS_PIN 5     // Pino de Chip Select (CS)
#define MOSI_PIN 23  // Pino MOSI
#define MISO_PIN 15  // Pino MISO
#define SCK_PIN 18   // Pino Serial Clock
#define SSID "Inteli-welcome"
#define PASSWORD ""
#define UBIDOTS_TOKEN "BBUS-m1uMJPEMhNezBHtCmQgeBeUDz95Gdg"
#define DEVICE_LABEL "voltz"
#define MQTT_BROKER "industrial.api.ubidots.com"
#define DATA_PIN 25
#define CLOCK_PIN 33
#define RGB_R_PIN 12
#define RGB_G_PIN 27
#define RGB_B_PIN 13
#define BUZZER_PIN 32
#define BUTTON_TARE 19
#define BUTTON_TOGGLE_LCD 23
#define DHTPIN 4
#define DHTTYPE DHT11
float getLoad();
float getTemp();
void initLCD();
long timer;
float humidity;
float humidity_atual;
float temperature;
float temperature_atual;
const int outPutDevices[4] = { 27, 12, 13, 32 };
float temp;
float load;
int state;
bool lcdIsOn = true;  // Variável de controle para o estado do LCD
byte temp_circle[8] = {
  B01100,
  B10010,
  B10010,
  B01100,
  B00000,
  B00000,
  B00000
};
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
HX711 scale;
WiFiClient espClient;
Ubidots ubidots(UBIDOTS_TOKEN);
class AlertManager {
public:
  enum State {
    Iddle,
    WiFiDisconnected,
    SensorDisconnected,
    AllDisconnected
  };
  static void checkAlerts() {
    long leitura = scale.read();
    if (leitura <= 2100) {
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
      setRGB(0, 255, 0);
    } else if (state == 1) {
      setRGB(255, 220, 0);
    } else if (state == 2) {
      setRGB(0, 0, 255);
    } else if (state == 3) {
      setRGB(255, 0, 0);
    }
    if (load >= 0.5) {
      analogWrite(BUZZER_PIN, 15);
    } else {
      analogWrite(BUZZER_PIN, 0);
    }
  }
private:
  static void setRGB(int red, int green, int blue) {
    analogWrite(RGB_R_PIN, red);
    analogWrite(RGB_G_PIN, green);
    analogWrite(RGB_B_PIN, blue);
  }
  static void blinkRed() {
    analogWrite(RGB_R_PIN, 255);
    delay(250);
    analogWrite(RGB_R_PIN, 0);
    delay(250);
  }
};
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
    lcd.setCursor(0, 1);
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
void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
void setupMQTT() {
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
}
void initSensors(byte dataPin, byte clockPin) {
  scale.begin(dataPin, clockPin);
  Serial.println("Sensor connected");
  scale.set_scale(-212097.487876);
  delay(1500);
  scale.tare();
  Serial.println("Scale Zeroed");
  dht.begin();
}
float getTemp() {
  float temp = dht.readTemperature();
  if (isnan(temp)) {
    temp = 0;
    Serial.println("Failed to read DHT!!");
  } else {
    Serial.print("Temp. ");
    Serial.print(temp);
    Serial.println("°C");
  }
  return temp;
}
float getLoad() {
  float load = scale.get_units(5);
  Serial.print("Load " + String(load));
  Serial.println("Kg");
  Serial.println(scale.read());

  return load;
}
void showResult(float load, float temp) {
  lcd.setCursor(0, 0);
  lcd.print("Peso:");
  lcd.print(load, 3);
  lcd.print("Kg");
  lcd.setCursor(0, 1);
  lcd.print("Temp:");
  lcd.print(temp, 2);
  lcd.write(1);
  lcd.print("C");
  delay(250);
}
void alerts() {
  AlertManager::checkAlerts();
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
void setup(void) {
  Serial.begin(115200);
  initLCD();
  initPins();
  initSensors(DATA_PIN, CLOCK_PIN);
  setupWIFI();
  setupMQTT();
  lcd.clear();
  Serial.println(scale.read());
}
void loop(void) {
  timer = millis();
  if (timer % 500 == 0) {
    Serial.println(scale.read());
  }
  if (digitalRead(BUTTON_TARE) == HIGH) {
    Serial.println("TARA");
    scale.tare();
  }
  if (digitalRead(BUTTON_TOGGLE_LCD) == HIGH) {
    toggleLCD();  // Acione a função para ligar/desligar o LCD
    Serial.println("LCD");
    delay(1);  // Adicione um pequeno atraso para evitar detecção falsa de múltiplos pressionamentos
  }
  load = getLoad();
  temp = getTemp();
  showResult(load, temp);
  if (!ubidots.connected()) {
    Serial.println("Reconectando ao Ubidots...");
    ubidots.reconnect();
  }
  ubidots.add("peso", load);
  ubidots.add("temperatura", temp);
  // Publica os dados no Ubidots
  ubidots.publish(DEVICE_LABEL);
  // Mantém a conexão com o Ubidots e gerencia as mensagens internamente
  ubidots.loop();
  alerts();

  Serial.println(WiFi.status());
  delay(250);
}
void toggleLCD() {
  lcdIsOn = !lcdIsOn;  // Inverte o estado do LCD
  if (lcdIsOn) {
    lcd.backlight();  // Ligue o LCD
    lcd.on();         // Ligue o backlight
  } else {
    lcd.off();          // Desligue o backlight
    lcd.noBacklight();  // Desligue o LCD
  }
}
void initLCD() {
  lcd.init();
  lcd.backlight();
  lcd.createChar(1, temp_circle);
}