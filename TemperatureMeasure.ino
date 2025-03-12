#include <SPI.h>
#include <WiFiNINA.h>
#include "secrets.h"
#include "ThingSpeak.h" 
#include "DHT.h"

#define DHTPIN 2     // Digital pin 2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

char ssid[] = SECRET_SSID;   // my network SSID (name) 
char pass[] = SECRET_PASS;   // my network password
int keyIndex = 0;            // my network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

// Initialize our values
String myStatus = "";

void setup() {
  Serial.begin(115200);      // Initialize serial 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }
  
  ThingSpeak.begin(client);  // Initialize ThingSpeak 

  dht.begin();
}

void loop() {

  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }

  delay(2000);

  float h = dht.readHumidity();  
  float t = dht.readTemperature(); // Read temperature as Celsius (the default)  
  float f = dht.readTemperature(true); // Read temperature as Fahrenheit (isFahrenheit = true)

  
  if (isnan(h) || isnan(t) || isnan(f)) { // Check if any reads failed and exit early (to try again)
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  
  float hif = dht.computeHeatIndex(f, h); // Calculate heat index in Fahrenheit (the default)  
  float hic = dht.computeHeatIndex(t, h, false); // Calculate heat index in Celsius (isFahreheit = false)

  
  ThingSpeak.setField(1, t); // set the fields with the values
  ThingSpeak.setField(2, h);
  ThingSpeak.setField(3, f);
  
  Serial.println("Temperature: "+ String(t) + " | Humidity: " + String(h) + " | Index rate: " + String(h));
  
  // write to the ThingSpeak channel 
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  
  delay(20000); // Wait 20 seconds to update the channel again
}
