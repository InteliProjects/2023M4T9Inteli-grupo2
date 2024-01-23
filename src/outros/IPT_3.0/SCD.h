#include "esp32-hal.h"
#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <WiFi.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
HX711 scale;



#define TARE_BUTTON_PIN 18

#define RGB_R_PIN 27
#define RGB_G_PIN 12
#define RGB_B_PIN 13

#define BUZZER_PIN 32

#define ACCEPT_BUTTON 19
#define RIGHT_BUTTON  17
#define BACK_BUTTON   18
#define LEFT_BUTTON   16
#define TARE_BUTTON   15

#ifndef SIGNAL_CONDITIONER_DEVICE
#define SIGNAL_CONDITIONER_DEVICE

#include <Arduino.h>

const int outPutDevices[4] = { 27, 12, 13, 32 };
const int inPutDevices[5] = { 19, 18, 17, 16, 15 };

class SCD {
private:
  byte dataPin;
  byte clockPin;
  float weight;
  int state;
  long millis;
public:
  SCD(){};


  void init() {
    lcd.init();
    lcd.backlight();
    for (int i = 0; i < 4; i++) {
      pinMode(outPutDevices[i], OUTPUT);
    }

    for(int i = 0; i < 5; i++){
      pinMode(inPutDevices[i], INPUT_PULLDOWN);
    }

    pinMode(18, INPUT_PULLDOWN);
  }


  void connectToWifi(char *ssdi, char *password) {
    // Função que tenta conectar-se à rede WiFi
    lcd.setCursor(0, 0);
    lcd.print("Connecting to ");
    lcd.setCursor(0, 1);
    lcd.print("WiFi ");

    WiFi.begin("Inteli-COLLEGE", "QazWsx@123", 6);
    while (WiFi.status() != WL_CONNECTED) {
      delay(250);
      spinner();
    }

    // Exibe "Online" no LCD quando a conexão é estabelecida
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.println("Online");
    delay(1000);
  }


  void connectToHX711(byte dataPin, byte clockPin) {
    // Função que inicializa a célula de carga
    lcd.setCursor(0, 0);
    lcd.print("Conectando");
    lcd.setCursor(0, 1);
    lcd.print("Celula");

    scale.begin(dataPin, clockPin);

    // while (scale.is_ready() == false) {
    //   delay(250);
    //   spinner();
    // }

    // Configura a escala da célula de carga
    scale.set_scale(-208794.854 /*-211434.7938*/);

    // Exibe "Sensor conectado" no LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor conectado");
    delay(1000);

    lcd.clear();

    // Zera a balança
    scale.tare();
    lcd.setCursor(0, 0);
    lcd.print("Balanca zerada");
    delay(1000);

    lcd.clear();
  }


  float getWeight() {
    // Função que obtém o peso da célula de carga
    float load;
    scale.power_up();
    load = scale.get_units();
    scale.power_down();
    Serial.println(load, 8);
    delay(50);

    return load;
  }


  void showMensageOnDisplayMenu(char *mensage_title, float mensage_value, char *mensage_value_indicator, int decimal_precision) {
    // Função que exibe mensagens no LCD com título, valor e unidade
    lcd.setCursor(0, 0);
    lcd.print(mensage_title);
    lcd.setCursor(0, 1);
    lcd.print(mensage_value, decimal_precision);
    lcd.print(" ");
    lcd.print(mensage_value_indicator);
    lcd.print(" ");
  }


  void checkConnections() {
    enum espState { Iddle,
                    WiFiDisconnected,
                    SensorDisconnected,
                    AllDisconnected };

    if (WiFi.status() != WL_CONNECTED) {
      this->state = WiFiDisconnected;
    } else if (scale.is_ready() == true) {
      this->state = SensorDisconnected;
    } else if (WiFi.status() != WL_CONNECTED && scale.is_ready() == true) {
      this->state = AllDisconnected;
    } else {
      this->state = Iddle;
    }
  }


  void alerts() {
    checkConnections();

    if (state == 0) {
      analogWrite(RGB_G_PIN, 150);
    } else if (state == 1) {
      analogWrite(RGB_B_PIN, 150);
    } else if (state == 2) {
      analogWrite(RGB_G_PIN, 150);
      analogWrite(RGB_R_PIN, 150);
    } else if (state == 3) {
      analogWrite(RGB_R_PIN, 150);
    } else if (state == 4) {
      analogWrite(RGB_R_PIN, 255);
      delay(250);
      analogWrite(RGB_R_PIN, 0);
      delay(250);
    }

    if (getWeight() >= 0.5) {
      analogWrite(BUZZER_PIN, 50);
    } else {
      analogWrite(BUZZER_PIN, 0);
    }
  }


  void spinner() {
    // Função que exibe um caractere giratório no display LCD
    static int8_t counter = 0;
    const char *glyphs = "\xa1\xa5\xdb";
    lcd.setCursor(15, 1);
    lcd.print(glyphs[counter++]);
    if (counter == strlen(glyphs)) {
      counter = 0;
      delay(250);
    }
  }

  void disconnectWiFi() {
    WiFi.disconnect();
  }
};
#endif