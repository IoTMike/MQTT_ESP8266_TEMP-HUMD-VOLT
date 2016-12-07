/* Sketch to collect temp and send via MQTT to web.  

  Downloaded 11/18/2016 ML From:
  https://www.hackster.io/mtashiro/temp-sensor-connected-to-esp8266-and-upload-data-using-mqtt-5e05c9

  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  MQTT code Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
  
*/

/* 11/18/2016 ML
 *  Revised for WeMos using the DHT Shield.
 *  Compiles with 1.6.5 and 1.6.12.
 *  Got it working!!!!  Yeahoo :)
 *  
 *  Copyright Michael Lance, 2016.
 *  MIT license, all text above must be included in any redistribution
 */


#include <ESP8266WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <Adafruit_Sensor.h>    // This is called by DHT.h, but didn't get called!
#include <DHT.h>

/* DHT SENSOR SETUP */

#define DHTTYPE DHT11
#define DHTPIN  2   // D4 on WeMos Module
DHT dht(DHTPIN, DHTTYPE); 

/* WIFI SETUP */

#define WLAN_SSID       "<SSID>"  //Put your SSID here
#define WLAN_PASS       "<PASS>"  //Put you wifi password here

/* ADAFRUIT IO SETUP */

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883         // use 8883 for SSL
#define AIO_USERNAME    "<USERNAME"      //Put your Adafruit userid here
#define AIO_KEY         "YOUR_ADAFRUIT_KEY"    //Put your Adafruit IO key here

float temp_f;  // Values read from sensor
float humd_f;
int sensorValue;  // Reading from ADC - Analog(A0)

String webString="";     // String to display
// Generally, you should use "unsigned long" for variables that hold time
unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 2300;              // interval at which to read sensor

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
//WiFiClient client;
// or... use WiFiFlientSecure for SSL
WiFiClientSecure client;

// Store the MQTT server, username, and password in flash memory.
// ML - Which, for some reason, is an issue with WeMos!

//const char MQTT_SERVER[] PROGMEM    = AIO_SERVER;
//const char MQTT_USERNAME[] PROGMEM  = AIO_USERNAME;
//const char MQTT_PASSWORD[] PROGMEM  = AIO_KEY;
const char MQTT_SERVER[]     = AIO_SERVER;
const char MQTT_USERNAME[]   = AIO_USERNAME;
const char MQTT_PASSWORD[]   = AIO_KEY;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, AIO_SERVERPORT, MQTT_USERNAME, MQTT_PASSWORD);

/************************* Feeds ***************************************/

// Setup a feed called 'temp' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
const char TEMP_FEED[] = AIO_USERNAME "/feeds/temperature";
Adafruit_MQTT_Publish temp = Adafruit_MQTT_Publish(&mqtt, TEMP_FEED);

const char HUMD_FEED[] = AIO_USERNAME "/feeds/humidity";
Adafruit_MQTT_Publish humd = Adafruit_MQTT_Publish(&mqtt, HUMD_FEED);

const char VOLT_FEED[] = AIO_USERNAME "/feeds/voltage";
Adafruit_MQTT_Publish volt = Adafruit_MQTT_Publish(&mqtt, VOLT_FEED);


/*********************** Cycle Time ************************************/
//int delayTime = 300000;  //Wait 5 minutes before sending data to web
int delayTime = 30000;  //Wait 30 seconds before sending data to web
int startDelay = 0;

/********************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
//void MQTT_connect();


void setup() {
  Serial.begin(115200);
  delay(10);

  dht.begin();           // initialize temperature sensor

  //Serial.println(F("Mike's revised MQTT Temperature/Humidity"));
  Serial.println("Mike's revised MQTT Temperature/Humidity/Voltage");

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected to:  ");
  Serial.println(WLAN_SSID);
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Get initial temp
  temp_f = dht.readTemperature(true);   // (true) returns Farenheit
  humd_f = dht.readHumidity();
  Serial.println();
  Serial.print("Initial Temperature: ");
  Serial.println(temp_f);
  Serial.print("Initial Humidity:    ");
  Serial.println(humd_f);
  Serial.println();
  int sensorValue = analogRead(A0);
  float volts = sensorValue;
  Serial.print("Initial Voltage:     ");
  Serial.println(volts / 243.8);
  Serial.println();

}

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // Now we can publish stuff!  
  if (millis() - startDelay < delayTime) {
    //Serial.println ("waiting delaytime");
    } else {
    temp_f = dht.readTemperature(true);               //Get temp in Farenheit
    //startDelay = millis();
    Serial.print(F("\nSending temperature: "));
    Serial.print(temp_f);
    Serial.print("...");
    if (! temp.publish(temp_f)) {                     //Publish to Adafruit
      Serial.println(F("Failed"));
      } else {
      Serial.println(F("Sent!"));
      }

    humd_f = dht.readHumidity();               //Get Humidity
    startDelay = millis();
    Serial.print(F("\nSending humidity: "));
    Serial.print(humd_f);
    Serial.print("...");
    if (! humd.publish(humd_f)) {                     //Publish to Adafruit
      Serial.println(F("Failed"));
      } else {
      Serial.println(F("Sent!"));
      }

    sensorValue = analogRead(A0);
    Serial.print(F("\nSending voltage: "));
    float volts = sensorValue;
    volts = volts / 243.8;
    Serial.print(volts);
    Serial.print("...");
    if (! volt.publish(volts)) {                    //Publish to Adafruit
      Serial.println(F("Failed"));
      } else {
      Serial.println(F("Sent!"));
      }

  }
  
  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds

  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
}


// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
