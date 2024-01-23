#include "Led.h"
#include "HX711.h"
#include <LiquidCrystal_I2C.h>

// Definindo pinos do amplificador HX711
#define DT_PIN 6
#define SCK_PIN 7

// INTANCIANDO CLASSES
LED led_1(13, LOW); // led 1 com pino(13) e estado LOW
LED led_2(12, HIGH); // led 2 com pino(12) e estado HIGH
HX711 scale;
LiquidCrystal_I2C lcd(0x27, 16,2); // LCD com endereço 0x27 e tamanho de 16x2 

float medida = 0; // Variável que armazenará a medida de peso

void setup() {
  Serial.begin(115200);
  scale.begin(DT_PIN, SCK_PIN);// Iniciando a célula de carga 
  scale.set_scale(377.9999921);// Definindo a escala de midição
  delay(1500);
  scale.tare();
  Serial.println("Balança Zerada");
  delay(200);
  // Estartadno LEDs e LCD
  led_1.init();
  led_2.init();
  lcd.init();
}

void loop() {
  medida = scale.get_units(100); // Salvando na variável o valor da média de 10 medidas
  lcd.print("Peso de ");// Faz o LCD imprimir o valor do peso da carga
  lcd.print(medida,6);
  lcd.setCursor(0,0);
  scale.power_down(); // Desliga o sensor
  delay(20); // Aguarda 0,2 Segundo
  scale.power_up(); // Liga o sensor

  // Estado do LED verde, apenas, ligado
  if(medida < 40){
    led_1.on();
    led_2.off();
  }
  if(medida > 40 and medida < 45){ // Mudança de estado dos LEDs e aviso do LCD de peso alto
      led_1.toggleLedState();
      led_2.toggleLedState();
      lcd.print("Peso prox limite ");
      lcd.setCursor(0,1);
  }else if(medida > 45){ // Mudança do estado do LED laranja e aviso de peso muito alto, pelo LCD
      led_1.off();
      led_2.toggleLedState();
      lcd.print("Peso muito alto ");
      lcd.setCursor(0,1);
  }
}
