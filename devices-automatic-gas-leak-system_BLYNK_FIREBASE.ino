/*New blynk app project
   Home Page
*/
#define Green D6
#define Red D7

//Include the library files
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Firebase_ESP_Client.h>



// firebase
#define DATABASE_URL "https://gas-sensor-mq2-default-rtdb.firebaseio.com/" // Firebase host
#define API_KEY "AIzaSyAc6BQ-WtIK4czF9VOJ-TOcx3eQdCHbdgs" //Firebase Auth code

/* Fill-in your Template ID (only if using Blynk.Cloud) */
#define BLYNK_TEMPLATE_ID "TMPL2aFiaXoC"
#define BLYNK_DEVICE_NAME "GASWARNINGMQ02"
#define BLYNK_AUTH_TOKEN "ffrkXR-d_w8opGXkMRZLhnMCjzqyiOu5"

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "binhlong";//Enter your WIFI name
char pass[] = "thaibinh2008";//Enter your WIFI password

FirebaseData fbdo;
FirebaseAuth AUTH;
FirebaseConfig config;

bool signupOK = false;
int buzzer=5; //D1
int warningLevel=800;
BlynkTimer timer;
int timerID1,timerID2;
int mq2_value = A0;
int sensorvalue;
int button=0; //D3
boolean buttonState=HIGH;
boolean runMode=1;//Turn on/off warning system
boolean warningState=0;
String notification="";
WidgetLED led(V0);

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"




//Get the button value
void handleTimerID1(){
  if(Firebase.ready() && signupOK){
    Serial.print("Connecting successfully!!!");
    sensorvalue = analogRead(mq2_value);
    Firebase.RTDB.setFloat(&fbdo, "MQ2 Sensor/Value",sensorvalue);  
    Blynk.virtualWrite(V1,sensorvalue);
    if(led.getValue()) {
      led.off();
    } else {
      led.on();
    }
    if(runMode==1){
        if(sensorvalue>warningLevel){
          if(warningState==0){
            warningState=1;
            Blynk.logEvent("warning", String("Warning! The gas=" + String(sensorvalue)+" exceeded allow!"));
            timerID2 = timer.setTimeout(60000L,handleTimerID2);
            digitalWrite(Green, LOW);
            digitalWrite(Red, HIGH);
          }
          digitalWrite(buzzer,HIGH);
          Blynk.virtualWrite(V3,HIGH);
          digitalWrite(Green, LOW);
          digitalWrite(Red, HIGH);
          notification = "WARNING!! GAS LEAK";
          Firebase.RTDB.setString(&fbdo, "GAS/STATUS", notification);
          Serial.println("Warning!!! THE GAS LEAK");
          }else{
            digitalWrite(buzzer,LOW);
            Blynk.virtualWrite(V3,LOW);
            digitalWrite(Green, HIGH);
            digitalWrite(Red, LOW);
            notification = "THE GAS NORMAL";
            Firebase.RTDB.setString(&fbdo, "GAS/STATUS", notification);
            Serial.println("THE GAS IN NORMAL STATUS!");
          }
        }else{
          digitalWrite(buzzer,LOW);
          Blynk.virtualWrite(V3,LOW);
          digitalWrite(Green, HIGH);
          digitalWrite(Red, LOW);
          notification = "THE GAS NORMAL";
          Firebase.RTDB.setString(&fbdo, "GAS/STATUS", notification);
          Serial.println("THE GAS IN NORMAL STATUS!");
        }
  }
}
void handleTimerID2(){
  warningState=0;
}
BLYNK_CONNECTED() {
  Blynk.syncVirtual(V2,V4);
}
BLYNK_WRITE(V2) {
  warningLevel = param.asInt();
}
BLYNK_WRITE(V4) {
  runMode = param.asInt();
}

void setup() {
  //Set the LED pin as an output pin
  Serial.begin(115200);
  pinMode(Green, OUTPUT);
  pinMode(Red, OUTPUT);
  pinMode(mq2_value, INPUT);
  pinMode(button,INPUT_PULLUP);
  pinMode(buzzer,OUTPUT);
  digitalWrite(buzzer,LOW); //Turn off buzzer
  //Initialize the Blynk library
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  timerID1 = timer.setInterval(1000L,handleTimerID1);

  // for firebase
  WiFi.begin(ssid, pass);
   Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &AUTH, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &AUTH);
  Firebase.reconnectWiFi(true);
}

void loop() {
  //Run the Blynk library
  Blynk.run();
  timer.run();
  if(digitalRead(button)==LOW){
    if(buttonState==HIGH){
      buttonState=LOW;
      runMode=!runMode;
      Serial.println("Run mode: " + String(runMode));
      Blynk.virtualWrite(V4,runMode);
      delay(200);
    }
  }else{
    buttonState=HIGH;
  }
}

