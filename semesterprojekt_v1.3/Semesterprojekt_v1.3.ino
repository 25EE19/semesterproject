#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 15
#define RST_PIN 0
#define SENSOR D1 //Magnetschalter der den Alarm auslöst, sobald er unterbrochen ist und die Alarmanlage scharf ist
#define BUZZER D2 //Pieper der einen Signalton von sich gibt
#define LED D4    //Die LED die leuchtet, während der Alarm scharf ist

//folgende vier Parameter anpassen / Topic gegebenenfalls ändern und die Anmeldedaten sowie den Namen des Routers angeben
const char* mqtt_topic_publish = "Alarmanlage";
const char* mqtt_topic_subscribe = "DatenHandy";
const char* ssid = "xxxxxx"; //Name des Netzwerks / Routers
const char* password = "xxxxxxx"; //Passwort vom Netzwerk / Router

//Paramter für die Kommunikation mit dem Server
const char* mqtt_server = "mqtt.iot.informatik.uni-oldenburg.de";
const int mqtt_port = 2883;
const char* mqtt_user = "sutk";
const char* mqtt_pw = "SoftSkills";

long lastMsg = 0;
int val = 0;
int state; //Zustand des Systems
MFRC522 mfrc522(SS_PIN, RST_PIN);
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {

  delay(10);
  //Verbindung zu einem Netzwerk...
  Serial.println();
  Serial.print("Verbindung zu ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi verbunden");
  Serial.println("IP Adresse: ");
  Serial.println(WiFi.localIP());
}

//Diese Methode wird aufgerufen, sobald es neue Nachrichten gibt, die über das Topic "DatenHandy" versendet wurden.
void callback(char* topic, byte* payload, unsigned int length) {
  char receivedPayload[length];
  Serial.print("Nachricht angekommen [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    receivedPayload[i] = (char) payload[i];
  }
}

  void reconnect() {
  //Schleife bis die Verbindung wieder hergestellt ist
  while (!client.connected()) {
    Serial.print("Versuche MQTT-Verbindung herzustellen...");
    
    //Erzeuge eine zufällige client ID: Client ID MUSS inviduell sein, da der MQTT Broker nicht mehrere Clients mit derselben ID bedienen kann
    String clientId = "Client-";
    clientId += String(random(0xffff), HEX);
    
    //Versuche zu verbinden
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pw)) {
      Serial.println("Verbunden");
      client.subscribe(mqtt_topic_subscribe);
    } else {
      Serial.print("fehlgeschlagen, rc=");
      Serial.print(client.state());
      Serial.println(" erneuter Versuch in 5 Sekunden");
      delay(5000);
    }
  }
}

void setup(){
  Serial.begin(115200);
  
  pinMode(SENSOR, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT); //Der Pin 4 ist jetzt ein Ausgang und die LED ist dort angeschlossen
  
  setup_wifi();
  SPI.begin();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  mfrc522.PCD_Init();
  
  if (digitalRead(SENSOR) == HIGH) {   //Magnetschalter wird ausgelesen
    state = 0;
  }else{
    digitalWrite(LED,HIGH);
    state = 1;
  }
}

void loop(){

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
    
  long code = chipID(); //Die Zahlenkombination des RFID-Chips (Die ID des Chips) 
  switch (state){
    case 0: activateSystem(code); break;
    case 1: checkSystem(code); break;
    case 3: alarm(code); break;
  }
  Serial.println(state);
}

//Auslesen (und Zurueckgeben) der ID auf dem Chip. Die Zahlenkombination wird als HEXA-Zahl angegeben, deshalb muss diese umgerechnet werde, um eine Dezimalzahl zu erhalten
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
    state = 0;    //deaktiviert das System
    digitalWrite (BUZZER, LOW);
    digitalWrite(LED,LOW);
    //Publish Char Array (einfachste Methode)
    client.publish(mqtt_topic_publish, "1");
    delay(2000);
  }
}

//Wenn ein autorisierter Chip erkannt wird, wird das System aktiviert
void activateSystem(long id){
  if (id == 1786920){
      state = 1;
      digitalWrite(LED,HIGH);
      //Publish Char Array (einfachste Methode)
    client.publish(mqtt_topic_publish, "2");
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
