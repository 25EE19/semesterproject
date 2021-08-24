//TODO MQTT einbinden und Testen
//Oder eventuell auch BLYNK

#include <SPI.h>
#include <MFRC522.h>

#define MAGNETPIN D1
#define LED D2
#define SIREN D5
#define SS_PIN D8
#define RESET_PIN D3

int state;  //0 is off, 1 is ready, 2 is alarm is blocked, 3 is alarm
long accessIDs[] = {1234, 7890}; 

MFRC522 rfid(SS_PIN, RESET_PIN);


void setup() {
  Serial.begin(115200);
  pinMode(MAGNETPIN, INPUT);
  pinMode(SIREN, OUTPUT);

  SPI.begin();

  rfid.PCD_Init();

  //Wenn der Magnetschalter unterbrochen ist, wird beim Starten kein Alarm ausgelöst. Wenn nicht ist die Anlage scharf
  if (digitalRead(MAGNETPIN) == HIGH) {   //Eventuell High und Low tauschen, je nach Aufbau
    state = 0;
  }else{
    state = 1;
  }

}

void loop() {
  switch (state){
    case 0: activateSystem(); break;
    case 1: checkSystem(); break;
    case 3: alarm(); break;
  }

}

//Alarm (Beeper) kann durch autorisierten Chip deaktiviert werden
void alarm(){
  digitalWrite (SIREN, HIGH);
  if (rfid.PICC_IsNewCardPresent()){
    for (int i = 0; i < sizeof(accessIDs); i++){
      if (accessIDs[i] == rfid.PICC_ReadCardSerial()){     //Vermutlich noch parse to int/long oder so
        state = 0;    //inaktiviert das System
        digitalWrite (SIREN, LOW);
      }
    }
  }
}

//Wenn ein autorisierter Chip erkannt wird, wird das System aktiviert
void activateSystem(){
  if (rfid.PICC_IsNewCardPresent()){
    for (int i = 0; i < sizeof(accessIDs); i++){
      if (accessIDs[i] == rfid.PICC_ReadCardSerial()){
        state = 1;
      }
    }
  }
}

//Ueberprüfen des Magnetschalters und Moeglichkeit mit autorisiertem Chip den Alarm auszusetzen
void checkSystem(){
  if (MAGNETPIN == HIGH){
    state = 3;
  }else if(rfid.PICC_IsNewCardPresent()){
    for (int i = 0; i < sizeof(accessIDs); i++){
      if (accessIDs[i] == rfid.PICC_ReadCardSerial()){
        delay(30000);   //Der Alarm wir fuer 30 Sekunden ausgesetzt
      }
    }
  }
}
