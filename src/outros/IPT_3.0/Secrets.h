#include <WiFi.h>

#ifndef WIFI_SCAN_AND_SECRETS
#define WIFI_SCAN_AND_SECRETS

#define MAX_ARRAY_LENGHT 25

int scannedWiFisArray[MAX_ARRAY_LENGHT];

int currentLength = 0

int[] scanner(){
  numberOfWiFi = WiFi.scanNetWorks();

  if(numberOfWiFi == 0){
    return {0};
  } else{
    for (int i=0; i<numberOfWiFi; i++) {
      scannedWiFisArray(i);
    }
  }
}

// Função para adicionar elementos ao array
void addElement(int element) {
  // Verifica se há espaço disponível no array
  if (currentLenght < MAX_ARRAY_LENGHT) {
    // Adiciona o elemento ao array
    scannedWiFisArray[currentLenght] = element;

    // Incrementa o tamanho atual
    currentLenght++;
  } else {
    // Se o array estiver cheio, imprime uma mensagem de erro
    Serial.println("Erro: Array cheio, não é possível adicionar mais elementos.");
  }
}

#endif