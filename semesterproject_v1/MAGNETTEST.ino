//const int LED = D6; // LED auf PIN D6, die anzeigt, wenn der Alarm scharf geschaltet ist
const int SENSOR = D1; //Magnetschalter der den Alarm auslöst, sobald er unterbrochen ist und die Alarmanlage scharf ist
const int BUZZER = D2; //for ESP8266 Microcontroller
int STATE; // 0 close - 1 open wwitch
boolean RFID = false;

void setup()
{

//pinMode(LED, OUTPUT);
  pinMode(SENSOR, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  Serial.begin (115200);
}

void loop()
{
  STATE = digitalRead(SENSOR);

do
{
  if (STATE == HIGH) {
   // digitalWrite (LED, LOW);
    digitalWrite (BUZZER, HIGH);
    Serial.println("Open");
  }
  else {
//    digitalWrite (LED, HIGH);
    digitalWrite (BUZZER, LOW);
    Serial.println("Closed");
  }
  delay(300);
} while (RFID=false);

//delay(600000);
}






/*void loop(){
 
 if (MAGNET = digitalRead(MAGNET)==1)
 {
  digitalWrite(LED, HIGH); 
 }

 else if (MAGNET = digitalRead(MAGNET)==0)
 {
   digitalWrite(LED, LOW); 
 }*/
