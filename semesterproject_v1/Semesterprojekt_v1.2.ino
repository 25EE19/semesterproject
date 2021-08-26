#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>

//folgende vier Parameter anpassen.
//--------------------------------
const char* mqtt_topic_publish = "Alarmanlage";
const char* mqtt_topic_subscribe = "DatenHandy";
const char* ssid = "WLAN-270354";
const char* password = "56077513195230013547";
//--------------------------------

//folgende vier Parameter nicht ändern
//--------------------------------
const char* mqtt_server = "mqtt.iot.informatik.uni-oldenburg.de";
const int mqtt_port = 2883;
const char* mqtt_user = "sutk";
const char* mqtt_pw = "SoftSkills";
//--------------------------------

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
int val = 0;

#define SS_PIN 15
#define RST_PIN 0
#define SENSOR D1 //Magnetschalter der den Alarm auslöst, sobald er unterbrochen ist und die Alarmanlage scharf ist
#define BUZZER D2 // Pieper der einen Signalton von sich gibt
#define LED D4
int state; //Zustand des Systems
MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//Diese Methode wird aufgerufen, sobald es neue Nachrichten gibt, die über das Topic "wetterID/2" versendet wurden.
void callback(char* topic, byte* payload, unsigned int length) {
  char receivedPayload[length];
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    receivedPayload[i] = (char) payload[i];
  }
}

  void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // Create a random client ID: Client ID MUSS inviduell sein, da der MQTT Broker nicht mehrere Clients mit derselben ID bedienen kann
    String clientId = "Client-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pw)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.subscribe(mqtt_topic_subscribe);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup(){

pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
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

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
    
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
    // Publish Char Array (einfachste Methode)
    client.publish(mqtt_topic_publish, "1");
    delay(2000);
  }
}

//Wenn ein autorisierter Chip erkannt wird, wird das System aktiviert
void activateSystem(long id){
  if (id == 1786920){
      state = 1;
      digitalWrite(LED,HIGH);
      // Publish Char Array (einfachste Methode)
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
