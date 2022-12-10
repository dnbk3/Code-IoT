#define BLYNK_TEMPLATE_ID "TMPLayAoj8IE"
#define BLYNK_DEVICE_NAME "IoTanalog"
#define BLYNK_AUTH_TOKEN "QliQr_pbw2FmvOkJ5dNP1EHqI-ZqS-Ly"
// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial


#include <ESP8266WiFi.h>
#include <string.h>
#include <BlynkSimpleEsp8266.h>

char ssid[] = "realme5Pro";
char pass[] = "00000000";
char auth[] = "QliQr_pbw2FmvOkJ5dNP1EHqI-ZqS-Ly";





int button = 0;
int led = 5;
int SR505_1 = 4;
int SR505_2 = 12;

int TT_SR505_1 = 0;
int TT_SR505_2 = 0;
int val = 0;
float t_val = 0;

void setup() {
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  Serial.println("connected");
  pinMode(led, OUTPUT);
  pinMode(SR505_1, INPUT);
  pinMode(SR505_2, INPUT);


}
BLYNK_WRITE(V2) {
  button = param.asInt();
  //  if(button == 0){
  //    digitalWrite(led,HIGH);
  //  }
  //  else{
  //    digitalWrite(led,LOW);
  //  }
}
void loop() {
  Blynk.run();
  Serial.print(digitalRead(SR505_2));
  Serial.print(" ");
  Serial.print(digitalRead(SR505_1));
  Serial.print(" ");
  Serial.print(val);
  Serial.println("       ");
  if (button == 1) {
    if (digitalRead(SR505_1) == HIGH ) {
      if (TT_SR505_1 == 0) {
        if (digitalRead(led) == 0) {
          digitalWrite(led, HIGH);
          Blynk.virtualWrite(V1, 1);
          t_val = getTime();
          val = 1;
        }
        else {
          digitalWrite(led, LOW);
          Blynk.virtualWrite(V1, 0);
          push(getTime() - t_val);
          val = 0;
        }
      }

    }


    if (digitalRead(SR505_2) == HIGH ) {
      if (TT_SR505_2 == LOW) {
        if (digitalRead(led) == 0) {
          digitalWrite(led, HIGH);
          Blynk.virtualWrite(V1, 1);
          t_val = getTime();
          val = 1;
        }
        else {
          digitalWrite(led, LOW);
          Blynk.virtualWrite(V1, 0);
          push(getTime() - t_val);
          val = 0;
        }
      }
    }
  }
  else{
    digitalWrite(led, LOW);
    Blynk.virtualWrite(V1, 0);
    val = 0;
  }
  TT_SR505_2 = digitalRead(SR505_2);
  TT_SR505_1 = digitalRead(SR505_1);

}

float getTime() {
  return millis() / 1000.0;
}

void push(float t) {
  Blynk.virtualWrite(V0, t);

}
