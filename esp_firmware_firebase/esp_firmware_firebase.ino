#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

typedef struct Pin {
  bool en = false;
  int pin = -1;
  int level = -1;
};

// List of inputs
const uint8_t inputs[] = { D5, D6, D7 };
const char inLabels[][5] = {"LED1", "LED2", "LED3"};

// List of outputs
// Usually LED_BUILTIN turn on with level LOW on pin D4
const uint8_t outputs[] = { D3, LED_BUILTIN };
const char outLabels[][5] = {"OUT1", "OUT2"};

/* 1. Define the WiFi credentials */
#define WIFI_SSID "xxxxxxx"
#define WIFI_PASSWORD "xxxxxx"

/* 2. Define the API Key */
#define API_KEY "xxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
/* 3. Define the RTDB URL */
#define DATABASE_URL "https://xxxxxxxxxxxxx-rtdb.europe-west1.firebasedatabase.app/"
/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "xxxxxxxx@email.email"
#define USER_PASSWORD "xxxxxxxxxxxxxxxxx"

// Define FirebaseESP8266 data object
FirebaseData fbdo1;
FirebaseData fbdo2;

FirebaseAuth auth;
FirebaseConfig config;
const char* path = "/esp-stream";

void updateGpioList() {
    String jsonStr;
    FirebaseJson gpios;
    size_t pos = 0;
    for (uint8_t i=0; i < sizeof(inputs); i++) {
        FirebaseJson gpio;
        gpio.add("pin", String(inputs[i]));
        gpio.add("label", String(inLabels[i]));
        gpio.add("type", "input");
        gpio.add("level", digitalRead(inputs[i]) ? true : false);
        String key = "gpios/[" + String(pos++) + "]";
        gpios.set(key, gpio);
    }
    for (uint8_t i=0; i < sizeof(outputs); i++) {
        FirebaseJson gpio;
        gpio.add("pin", String(outputs[i]));
        gpio.add("label", String(outLabels[i]));
        gpio.add("type", "output");
        gpio.add("level", digitalRead(outputs[i]) ? true : false);
        String key = "gpios/[" + String(pos++) + "]";
        gpios.set(key, gpio);
    }
    if(Firebase.ready()) {
      if (!Firebase.RTDB.setJSONAsync(&fbdo2, path, &gpios)) {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo2.errorReason());      
        Serial.println();
      }
    }
}


bool updateGpioState(){
  // With this variable, we will check if some of defined gpios has changed
  uint16_t gpioState;
  static uint16_t gpioLast;
  size_t pos = 0;
  for (uint8_t i=0; i < sizeof(inputs); i++) {
      gpioState = (digitalRead(inputs[i]) << pos++) | gpioState;
  }
  for (uint8_t i=0; i < sizeof(outputs); i++) {
      gpioState = (digitalRead(outputs[i]) << pos++) | gpioState;
  }
  if(gpioState != gpioLast) {
    gpioLast = gpioState;
    return true;
  }
  return false; 
}



void streamCallback(FirebaseStream data) {
    Serial.println("\nStream Data1 available...");
    Serial.println("STREAM PATH: " + data.streamPath());
    Serial.println("EVENT PATH: " + data.dataPath());
    Serial.println("DATA TYPE: " + data.dataType());
    Serial.println("EVENT TYPE: " + data.eventType());
    
    if (data.dataType() == "json") {
        FirebaseJson &json = data.jsonObject();
        size_t len = json.iteratorBegin();
        String key, value = "";
        int type = 0;
        Pin pin;
        for (size_t i = 0; i < len; i++) {
            json.iteratorGet(i, type, key, value);
            //Serial.printf("\"%s\":\"%s\"\n", key.c_str(), value.c_str());            
            if(key.equals("cmd") && value.equals("writeOut")) {
                pin.en = true;
            }
            // writeOut command received, set the pin structure ready to be valorized
            if(pin.en) { 
              if(key.equals("pin"))
                pin.pin = value.toInt();
              if(key.equals("level"))
                pin.level = value.equals("true") ? 1 : 0;

              // All variables valorized with parsed values, set output and clear pin structure
              if(pin.level > -1 && pin.pin > -1) {
                Serial.printf("Set output pin %d: %d\n", pin.pin, pin.level);
                digitalWrite(pin.pin, pin.level);
                pin.en = false;
                pin.pin = -1;
                pin.level = -1;
              }
            }
        }
        json.iteratorEnd();
    }
   
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout)
  {
    Serial.println();
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }
}

void setup() {
  // Set pinMode for input and output pins
  for (uint8_t i = 0; i < sizeof(inputs); i++)
    pinMode(inputs[i], INPUT_PULLUP);
  for (uint8_t i = 0; i < sizeof(outputs); i++)
    pinMode(outputs[i], OUTPUT);
    
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.print("\nConnected with IP: ");
  Serial.println(WiFi.localIP());

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

#if defined(ESP8266)
  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  fbdo1.setBSSLBufferSize(1024, 1024);
#endif

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  fbdo1.setResponseSize(1024);

#if defined(ESP8266)
  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  fbdo2.setBSSLBufferSize(1024, 1024);
#endif

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  fbdo2.setResponseSize(1024);

  //The data under the node being stream (parent path) should keep small
  //Large stream payload leads to the parsing error due to memory allocation.
  if (!Firebase.RTDB.beginStream(&fbdo1, path)) {
    Serial.println("\nCan't begin stream connection...");
    Serial.println("REASON: " + fbdo1.errorReason());
    Serial.println("------------------------------------\n");
  }
  
  Firebase.RTDB.setStreamCallback(&fbdo1, streamCallback, streamTimeoutCallback);
  updateGpioList();
}

void loop() {
  if( updateGpioState()) {
    updateGpioList();
  }
}
