#ifndef BLINKLED_H
#define BLNKLED_H

#include <Arduino.h>

class LED {
private:
  byte pin; //pino de dentrada digital do LED
  byte state; //state: estado que o LED está HIWG/LOW

public:
  LED(byte pin, byte state) {
    this->pin = pin; // o pino do LED é o parâmetro de entra "pin" ao intanciar a classe
    this->state = state; // o stado do LED é o parâmetro de entrada "state" ao intanciar a classe 
  } 

  // Função que configura o modo dos pinos e seta o estado do LED
  void init() {
    pinMode(pin, OUTPUT);
    if(state == HIGH){
      on();
    } else{
      off();
    }
  }

  // Função que muda o estado do LED
  void toggleLedState(){
    if(state == HIGH){
      off();
    } else {
      on();
    }
  }

  // Função que liga o LED
  void on() {
    digitalWrite(pin, HIGH);
    delay(150);
    state = HIGH;
  }

  // Função que desliga o LED
  void off() {
    digitalWrite(pin, LOW);
    delay(150);
    state = LOW;
  }
};

#endif