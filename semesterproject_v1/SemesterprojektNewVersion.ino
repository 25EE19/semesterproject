#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN 15
#define RST_PIN 0
#define SENSOR D1 //Magnetschalter der den Alarm auslöst, sobald er unterbrochen ist und die Alarmanlage scharf ist
#define BUZZER D2 // Pieper der einen Signalton von sich gibt
#define LED D4
int state; //Zustand des Systems
MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup(){
 if (digitalRead(SENSOR) == HIGH) {   //Eventuell High und Low tauschen, je nach Aufbau
    state = 0;
  }else{
    digitalWrite(LED,HIGH);
    state = 1;
  }
  pinMode(SENSOR, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
  pinMode (LED, OUTPUT); // Der Pin 4 ist jetzt ein Ausgang und die LED ist dort angeschlossen
}

void loop(){
  long code = chipID();
  switch (state){
    case 0: activateSystem(code); break;
    case 1: checkSystem(code); break;
    case 3: alarm(code); break;
  }
  Serial.println(state);
} 

//Auslesen (und Zurueckgeben) der ID auf dem Chip
long chipID(){
  long result = 0;
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()){
    for (byte i = 0; i < mfrc522.uid.size; i++){
      result = ((result+mfrc522.uid.uidByte[i])*10);
    }
  }
  return result;
}

//Alarm-Methode
void alarm(long id){
  digitalWrite (BUZZER, HIGH);
  if (id == 1786920){
    state = 0;    //inaktiviert das System
    digitalWrite (BUZZER, LOW);
    digitalWrite(LED,LOW);
    delay(2000);
  }
}

//Wenn ein autorisierter Chip erkannt wird, wird das System aktiviert
void activateSystem(long id){
  if (id == 1786920){
      state = 1;
      digitalWrite(LED,HIGH);
      delay(1000);
  }
}

//Ueberprüfen des Magnetschalters und Moeglichkeit mit autorisiertem Chip den Alarm auszusetzen
void checkSystem(long id){
  if (digitalRead(SENSOR) == HIGH){
    state = 3;
  }else if(id == 1786920){
      delay(30000);   //Der Alarm wir fuer 30 Sekunden ausgesetzt  
  }
}
