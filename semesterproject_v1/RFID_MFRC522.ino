
// Basic MFRC522 RFID Reader Code by cooper @ my.makesmart.net
// Released under Creative Commons - CC by cooper@my.makesmart.net

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN          15         // Pin: D8
#define RST_PIN         0          // Pin: D3

long chipID;


// MFRC522-Instanz erstellen
MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);

  // SPI-Bus initialisieren
  SPI.begin();
  Serial.println("SPI Bus initialisiert");

  // MFRC522 initialisieren
  mfrc522.PCD_Init();
  Serial.println("MFRC522 initialisiert");

  //Kurze Pause nach dem Initialisieren   
  delay(10);
  Serial.println("");

  // Details vom MFRC522 RFID READER / WRITER ausgeben
  mfrc522.PCD_DumpVersionToSerial();  

  
  Serial.println("RFID-Chip auflegen, um UID anzuzeigen...");
}

void loop() {
  
  // Sobald ein Chip aufgelegt wird startet diese Abfrage
  if (mfrc522.PICC_IsNewCardPresent()){
  
    //CardID resetten
    chipID = 0;
    
    mfrc522.PICC_ReadCardSerial();

    // Hier wird die ID des Chips in die Variable chipID geladen
    for (byte i = 0; i < mfrc522.uid.size; i++){
      chipID=((chipID+mfrc522.uid.uidByte[i])*10);
    }

    //... und anschließend ausgegeben
    Serial.println(chipID);

    // Danach 5 Sekunden pausieren, um mehrfaches lesen /ausführen zu verhindern
    delay(5000);
      
  }

  

}
