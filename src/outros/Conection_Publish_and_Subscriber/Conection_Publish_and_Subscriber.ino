#include <WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <Arduino.h>

// Configurações de conexão WiFi
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


float weight;
char* topicValue;


LiquidCrystal_I2C lcd(0x27, 16, 2);
HX711 scale;


// Client de Rede
WiFiClient espClient;


// Cliente MQTT
PubSubClient MQTT(espClient);


void setupWIFI() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  } else {
    // Conecta ao WiFi

    Serial.println();
    Serial.print("Connecting to");
    Serial.println(SSID);

    WiFi.begin(SSID, PASSWORD);

    // Loop para checar a conexão
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    // Printa no monitor serial as
    // informações da conexão
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

  // Conecta
  while (!MQTT.connected()) {
    Serial.print("- MQTT Setup: Tentando se conectar ao Broker MQTT: ");
    Serial.println(MQTT_BROKER);

    if (MQTT.connect(MQTT_ID)) {
      Serial.println("- MQTT Setup: Conectado com sucesso");
      MQTT.subscribe(MQTT_TOPIC);
    } else {
      Serial.println("- MQTT Setup: Falha ao se conectar, tentando novamente em 2s");
      delay(2000);
    }
  }
}

void setup(void) {
  Serial.begin(115200);

  initScale();

  lcd.init();
  lcd.backlight();
  // pinMode(LED, OUTPUT);
  setupWIFI();
  setupMQTT();

  // Publica *on* no tópico para iniciar com o LED aceso
  MQTT.publish(MQTT_TOPIC, "write");
}

void loop(void) {
  weight = getWeight();
  
  showResult();

  MQTT.publish(MQTT_TOPIC, (char));
  MQTT.loop();
  delay(2000);
}

// Função de Callback
// Chamada quando existe o recebimento de informação no tópico assinado
void LCD_callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  Serial.print("- MQTT Callback Topic: ");
  Serial.println(topic);

  //obtem a string do payload recebido
  for (int i = 0; i < length; i++) {
    char c = (char)payload[i];
    msg += c;
  }
  if (msg.equals("write")) {
    lcd.setCursor(0, 1);
    lcd.print("Msg recebida");
    Serial.println("- MQTT Sub Conn: *write* Received");
  }
  if (msg.equals("clear")) {
    lcd.clear();
    Serial.println("- MQTT Sub Conn: *clear* Received");
  }
  Serial.println();
  Serial.println("-----------------------");
}



void initScale() {
  while (!scale.is_ready()) {  // Enquanto a célula de carga não estiver conectada, o serial irá alertar da situação
    Serial.println("Sensor não conectado");
  }

  scale.begin(DATA_PIN, CLOCK_PIN);

  Serial.println("Sensor conectador");
  
  scale.set_scale(-212097.487876);

  delay(1500);
  scale.tare();  // Tira a tara da balança
  Serial.println("Balança Zerada");
}



// Função que pega o peso do objeto
// Rertorna o valor float do peso
float getWeight() {

  float medida = scale.get_units(5);  // Salvando na variável o valor da média de 5 medidas
  scale.power_down();                 // Desliga o sensor
  delay(100);                         // Aguarda 0,1 Segundo
  scale.power_up();                   // Liga o sensor

  Serial.print("   Kg");
  Serial.println(medida, 5);  // Pritna o valor do peso com 5 casas decimais
  delay(20);  // Espera 0,02s


  return medida;
}

// Função que peojeta os valores no LCD
float showResult() {
  lcd.print("KG  ");          // Printa "KG"  e o valor do peso, com 5 casas de precisão
  lcd.print(weight);
  lcd.setCursor(0, 0);        // Configura a o print, de cima, para a linha 1, tendo início na esquerda e fim na direita;
  delay(500);
  return 
}