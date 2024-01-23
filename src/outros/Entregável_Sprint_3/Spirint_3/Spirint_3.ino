#include <WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <Arduino.h>
#include <PubSubClient.h>

// Definições para a conexão WiFi
#define SSID "Inteli-COLLEGE"
#define PASSWORD "QazWsx@123"

// Configurações de conexão ao Broker MQTT
#define MQTT_ID "VoltzBanana"
#define MQTT_BROKER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_TOPIC "esp32Voltz/test/"

// Configurações da balança
#define DATA_PIN 25
#define CLOCK_PIN 33
#define TARE_BUTTON_PIN 18
#define RGB_R_PIN 27
#define RGB_G_PIN 12
#define RGB_B_PIN 13
#define BUZZER_PIN 32

// Arrays de pinos para dispositivos de entrada e saída
const int outPutDevices[4] = { 27, 12, 13, 32 };
const int inPutDevices[5] = { 19, 18, 17, 16, 15 };

float weight;
char *topicValue;
int state;

LiquidCrystal_I2C lcd(0x27, 16, 2);
HX711 scale;

// Client de Rede
WiFiClient espClient;

// Cliente MQTT
PubSubClient MQTT(espClient);

// Protótipos de funções
void initPins();
void setupMQTT();
void setupWiFi(char *ssid, char *password);
void setUpHX711(byte dataPin, byte clockPin);
void setUpLCD();
float getWeight();
void showResult(float input);
void alerts();
void LCD_callback(char *topic, byte *payload, unsigned int length);

void initPins() {
  // Configuração dos pinos de saída
  for (int i = 0; i < 4; i++) {
    pinMode(outPutDevices[i], OUTPUT);
  }

  // Configuração dos pinos de entrada com resistores de pull-down
  for (int i = 0; i < 5; i++) {
    pinMode(inPutDevices[i], INPUT_PULLDOWN);
  }

  pinMode(18, INPUT_PULLDOWN);  // Botão de tara da balança
}

void setupWiFi() {
  // Verifica se já está conectado à WiFi
  if (WiFi.status() == WL_CONNECTED) {
    return;
  } else {
    // Conecta ao WiFi
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(SSID);

    WiFi.begin(SSID, PASSWORD);

    // Aguarda a conexão
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    // Imprime informações da conexão no monitor serial
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP Address:");
    Serial.println(WiFi.localIP());
  }
}

void setupMQTT() {
  // Configura a conexão ao Broker MQTT
  MQTT.setServer(MQTT_BROKER, MQTT_PORT);
  MQTT.setCallback(LCD_callback);

  // Conecta ao Broker MQTT
  while (!MQTT.connected()) {
    Serial.print("- MQTT Setup: Trying to connect to MQTT Broker: ");
    Serial.println(MQTT_BROKER);

    if (MQTT.connect(MQTT_ID)) {
      Serial.println("- MQTT Setup: Successfully connected");
      MQTT.subscribe(MQTT_TOPIC);
    } else {
      Serial.println("- MQTT Setup: Failed to connect, trying again in 2s");
      delay(2000);
    }
  }
}

void setUpHX711(byte dataPin, byte clockPin) {
  // Aguarda até que a célula de carga esteja conectada
  while (!scale.is_ready()) {
    Serial.println("Sensor not connected");
  }

  // Inicializa a célula de carga
  scale.begin(dataPin, clockPin);

  Serial.println("Sensor connected");

  scale.set_scale(-212097.487876);  // Configuração da escala da balança

  delay(1500);
  scale.tare();  // Zera a balança
  Serial.println("Scale Zeroed");
}

void setUpLCD() {
  // Função de inicialização do LCD (se necessário)
}

void setup(void) {
  Serial.begin(115200);

  initPins();

  setUpHX711(DATA_PIN, CLOCK_PIN);

  lcd.init();
  lcd.backlight();

  setupWiFi();

  setupMQTT();

  // Publica *on* no tópico para iniciar com o LED aceso
  MQTT.publish(MQTT_TOPIC, "SCD iniciado");
}

void loop(void) {
  weight = getWeight();
  char topLength[15];

  topicValue = dtostrf(weight, 5, 2, topLength);

  // Publica o peso no tópico MQTT
  MQTT.publish(MQTT_TOPIC, topicValue);
  MQTT.loop();

  // Verifica alertas
  alerts();

  delay(250);  // Delay para estabilidade do loop
}

void LCD_callback(char *topic, byte *payload, unsigned int length) {
  String msg;
  Serial.print("- MQTT Callback Topic: ");
  Serial.println(topic);

  // Obtém a string do payload recebido
  for (int i = 0; i < length; i++) {
    char c = (char)payload[i];
    msg += c;
  }

  // Verifica mensagens recebidas e realiza ações correspondentes
  if (msg.equals("write")) {
    lcd.setCursor(0, 1);
    lcd.print("Msg received");
    Serial.println("- MQTT Sub Conn: *write* Received");
  } else if (msg.equals("clear")) {
    lcd.clear();
    Serial.println("- MQTT Sub Conn: *clear* Received");
  }

  Serial.println();
  Serial.println("-----------------------");
}

float getWeight() {
  float medida = scale.get_units(5);  // Média de 5 leituras do sensor
  scale.power_down();                 // Desliga o sensor
  delay(100);                         // Aguarda 0,1 segundo
  scale.power_up();                   // Liga o sensor

  Serial.print("   Kg");
  Serial.println(medida, 5);  // Imprime o peso com 5 casas decimais
  delay(20);                  // Aguarda 0,02 segundos

  // Atualiza o LCD com o resultado do peso
  showResult(medida);

  return medida;
}

void showResult(float input) {
  lcd.print("KG  ");  // Imprime "KG" e o valor do peso com 5 casas decimais
  lcd.print(input, 3);
  lcd.setCursor(0, 0);  // Configura o cursor para a linha 1, início à esquerda e fim à direita
  delay(500);           // Delay para estabilidade da exibição
}

void alerts() {
  // Exibe o status da conexão WiFi no monitor serial
  Serial.println(WiFi.status());

  // Enumeração para estados possíveis
  enum espState {
    Iddle,
    WiFiDisconnected,
    SensorDisconnected,
    AllDisconnected
  };

  // Lê o valor do sensor
  long leitura = scale.read();
  Serial.println(leitura);

  // Verifica o estado e toma ações correspondentes
  if (leitura <= -22000) {
    Serial.println("Sensor Disconnected");
    state = SensorDisconnected;
  } else if (WiFi.status() == 1) {
    state = WiFiDisconnected;
    Serial.println("WiFi Disconnected");
    WiFi.reconnect();
  } else if (WiFi.status() == WL_CONNECTION_LOST && scale.is_ready() == true) {
    state = AllDisconnected;
  } else {
    state = Iddle;
  }

  // Atualiza LEDs e buzzer com base no estado
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

  // Aciona o buzzer se o peso for maior ou igual a 0.5
  if (getWeight() >= 0.5) {
    analogWrite(BUZZER_PIN, 15);
  } else {
    analogWrite(BUZZER_PIN, 0);
  }
}
