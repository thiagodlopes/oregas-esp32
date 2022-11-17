//
// Copyright 2015 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// FirebaseDemo_ESP32 is a sample that demo the different functions
// of the FirebaseArduino API.

#include <WiFi.h>
#include <IOXhop_FirebaseESP32.h>

//DADOS PARA WI-FI E FIREBASE
#define FIREBASE_HOST "eoregasoficial-84973-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "9ZgYNILf9688YHTHDoRz4tGiWSFg0pwReGad2drU"
#define WIFI_SSID "Ricardo Sobreira 2"
#define WIFI_PASSWORD "Fla@14101995"

//REGAS
//LEDs NÍVEL DE ÁGUA NO TANQUE
const int greenLed = 4;
const int yellowLed = 16;
const int redLed = 17;

//LEDs UMIDADE
const int blueLed = 26;
const int whiteLed = 27;
//Obs.: PRECISEI MUDAR OS PINOS DOS SENSORES. NO WI-FI ELES NÃO FUNCIONAM NOS PINOS EM QUE ESTAVAM, SÓ FUNCIONAM NOS "ADC1"
const int moistureSensor = 35; //PINO DO SENSOR DE UMIDADE
const int analogWetSoil = 2900; //VALOR MÁXIMO DE UMIDADE QUE O SENSOR ATINGIU (DENTRO DO POTE COM ÁGUA) 
const int analogDrySoil = 4095; //VALOR MÍNIMO DE UMIDADE (FORA DA ÁGUA)
const int percWetSoil = 100; //PORCENTAGEM MÁXIMA DE "MOLHADO"(DENTRO DO POTE COM ÁGUA)
const int percDrySoil = 0; //PORCENTAGEM MÍNIMA DE "MOLHADO" (FORA DA ÁGUA)
int moistureSensorValue; //VARIÁVEL QUE ARMAZENA O VALOR DO SENSOR DE UMIDADE DO SOLO
int moisturePercentage; //VARIÁVEL QUE ARMAZENA A CONVERSÃO DO sensorValue EM PORCENTAGEM

const int waterLevelSensor = 34; //SENSOR DO RESERVATÓRIO DE ÁGUA
const int analogFullTank = 2200; //CAPACIDADE MÁXIMA DO TANQUE
const int analogEmptyTank = 4095; //MÍNIMO DE ÁGUA NO TANQUE
const int percFullTank = 100;
const int percEmptyTank = 0;
int waterLevelSensorValue;
int waterLevelPercentage;

const int waterBomb = 19; //TRANSISTOR DA BOMBA D'ÁGUA

void setup() {
  Serial.begin(9600);
  //REGAS
  pinMode(greenLed, OUTPUT);
  pinMode(yellowLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(whiteLed, OUTPUT);

  pinMode(moistureSensor, INPUT);

  pinMode(waterBomb, OUTPUT);
  //CONECTA NO WI-FI.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() {
  //REGAS
  moistureSensorValue = constrain(analogRead(moistureSensor),analogWetSoil,analogDrySoil); //MANTÉM sensorValue DENTRO DO INTERVALO (ENTRE analogWetSoil E analogSoloSeco)
  moisturePercentage = map(moistureSensorValue,analogDrySoil,analogWetSoil,percDrySoil,percWetSoil);//EXECUTA A FUNÇÃO "map" DE ACORDO COM OS PARÂMETROS PASSADOS
  PrintMoisture();
  waterLevelSensorValue = constrain(analogRead(waterLevelSensor),analogFullTank,analogEmptyTank);
  waterLevelPercentage = map(waterLevelSensorValue,analogEmptyTank,analogFullTank,percEmptyTank,percFullTank);
  PrintWaterLevel();
  
//NÍVEL DA ÁGUA
  if(waterLevelPercentage == 0){
      digitalWrite(greenLed, LOW);
      digitalWrite(yellowLed, LOW);
      digitalWrite(redLed, HIGH);
    } else if(waterLevelPercentage < 50){
      digitalWrite(greenLed, LOW);
      digitalWrite(yellowLed, HIGH);
      digitalWrite(redLed, LOW);
    } else {
      digitalWrite(greenLed, HIGH);
      digitalWrite(yellowLed, LOW);
      digitalWrite(redLed, LOW);
    }
    
//UMIDADE
  if(moisturePercentage <= 50){ //COMEÇA A PISCAR O LED AZUL QUANDO A UMIDADE DO SOLO FICA ABAIXO DE 50%
    BlinkBlueLed();
    Irrigate();
  } else { //DEIXA O LED ACESO QUANDO O NÍVEL DE UMIDADE ESTÁ ACIMA DE 50%
    digitalWrite(blueLed, HIGH);
  }
  
}

//MÉTODOS
  void BlinkBlueLed() {
    digitalWrite(blueLed, HIGH);
    delay(500);
    digitalWrite(blueLed, LOW);   
  }
  
  void Irrigate() {
    digitalWrite(waterBomb, HIGH);
    digitalWrite(whiteLed, HIGH);
    delay(1000);
    digitalWrite(waterBomb, LOW);
    digitalWrite(whiteLed, LOW);
  }
  
  void UpdateIntInFirebase(String key, int value) {
     Firebase.setInt(key, value);
     if (Firebase.failed()) {
      Serial.print("setting failed: ");
      Serial.println(Firebase.error());
      return;
     }
  }
  
  void GetIntInFirebase(String key) {
    Serial.println(Firebase.getInt(key));
  }
  
  void PrintMoisture() { 
    Serial.print("Umidade do solo: "); //IMPRIME O TEXTO NO MONITOR SERIAL
    Serial.print(moisturePercentage);//IMPRIME NO MONITOR SERIAL O PERCENTUAL DE UMIDADE DO SOLO
    Serial.println("%");//IMPRIME NO MONITOR SERIAL O PERCENTUAL DE UMIDADE DO SOLO
    Serial.println(moistureSensorValue);//IMPRIME O VALOR DO SENSOR
    UpdateIntInFirebase("moisturePercentage", moisturePercentage);
  }

  void PrintWaterLevel() {
    Serial.print("Capacidade do tanque: "); //IMPRIME O TEXTO NO MONITOR SERIAL
    Serial.print(waterLevelPercentage);//IMPRIME NO MONITOR SERIAL O PERCENTUAL DE ÁGUA NO TANK
    Serial.println("%");//IMPRIME NO MONITOR SERIAL O PERCENTUAL DE ÁGUA NO TANQUE
    Serial.println(waterLevelSensorValue);//IMPRIME O VALOR DO SENSOR
    UpdateIntInFirebase("waterLevelPercentage", waterLevelPercentage);
  }
