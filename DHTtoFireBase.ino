#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#include <DallasTemperature.h>
#include <OneWire.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#define ONE_WIRE_BUS D5  
#define DHTPIN D3 
#define DHTTYPE DHT11
#define pump D6

#define WIFI_SSID "HAHAHAH"
#define WIFI_PASSWORD "123456778"
#define API_KEY "AIzaSyDe9f04S1jstRY-v6ZNcg3ZEFvr8vlDK5Y" ////Replace With you API KEY on Firebase
// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://xxxxxx.firebaseio.com/"  //Replace With you URL Firebase
//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire); 
LiquidCrystal_I2C lcd(0x27, 16, 2);  

const int AirValue = 620;   
const int WaterValue = 310;  
int soilMoistureValue = 0;
int soilmoist=0;  
int humi, temp; 
//=====================================
void setup(void)
{
  lcd.begin(16,2);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("   Smart Farm  ");
  lcd.setCursor(0, 1);
  lcd.print(" Gusti Swandana ");
  sensors.begin();
  dht.begin();    
  delay(1500);
  lcd.clear();
  lcd.print("Mst=   %, T=   C");    
  lcd.setCursor(0, 1);
  lcd.print("Hum=   %, P= OFF");

  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
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
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}
//=====================================
void loop(void)
{ 
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    //STORE TO RTDB
    sensors.requestTemperatures();  
    temp=sensors.getTempCByIndex(0);
     
    soilMoistureValue = analogRead(A0);  
    soilmoist= map(soilMoistureValue, AirValue, WaterValue, 0, 100);
    if(soilmoist >= 100)
      {
        soilmoist=100;
      }
    else if(soilmoist <=0)
      {
        soilmoist=0;
      }
     
    humi = dht.readHumidity();  
    if (isnan(humi) ) {
      humi = 0;
      return;
    }
    Serial.print("Temp :");
    Serial.println(temp);
    lcd.setCursor(12,0);
    lcd.print(temp); 

    Serial.print("Soil Moisture :");
    Serial.print(soilmoist);
    Serial.println("%");
    lcd.setCursor(4,0);
    lcd.print(soilmoist); 
    lcd.print(" "); 
    Serial.print("Humi:");
    Serial.println(humi);  
    Serial.println(); 
    lcd.setCursor(4,1);
    lcd.print(humi);


    if (Firebase.RTDB.setInt(&fbdo, "Sensor/tempt", temp)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setInt(&fbdo, "Sendor/soilmoist", soilmoist)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setInt(&fbdo, "Sendor/humi", humi)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }               
  
}
