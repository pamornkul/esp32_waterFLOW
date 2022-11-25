
#include <WiFi.h>
#include "PubSubClient.h"

WiFiClient wifiClient;
PubSubClient client(wifiClient); 
const char* ssid = "xxxxxx";                 // Your personal network SSID
const char* wifi_password = "xxxxxxx"; // Your personal network password

const char* mqtt_server = "broker.hivemq.com";  // IP of the MQTT broker

const char* topic = "UTCCMakerspace/Flow0001";
const char* mqtt_username = ""; // MQTT username
const char* mqtt_password = ""; // MQTT password
const char* clientID = "makerspaceUTCC0001"; // MQTT client ID


#define PULSE_PIN 36

volatile long pulseCount=0;
float calibrationFactor = 4.5;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
float totalLitres;
unsigned long oldTime;

void ICACHE_RAM_ATTR pulseCounter()
{
  pulseCount++;
}


void setup() 
{
  Serial.begin(115200);

  WiFi.begin(ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("."); 
  }
  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, 1883);
  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0; 
                                    
  pinMode(PULSE_PIN, INPUT);
  attachInterrupt(PULSE_PIN, pulseCounter, FALLING);       

}

void loop() 
{
    if((millis() - oldTime) > 5000)    // Only process counters once per second
  {
    detachInterrupt(PULSE_PIN);

    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    oldTime = millis();
    flowMilliLitres = (flowRate / 60) * 1000;
    totalMilliLitres += flowMilliLitres;     
    totalLitres = totalMilliLitres * 0.001;
    unsigned int frac;   
    Serial.print("flowrate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print(".");             // Print the decimal point
    frac = (flowRate - int(flowRate)) * 10;
    Serial.print(frac, DEC) ;      // Print the fractional part of the variable
    Serial.print("L/min");
    Serial.print("  Current Liquid Flowing: ");             // Output separator
    Serial.print(flowMilliLitres);
    Serial.print("mL/Sec");
    Serial.print("  Output Liquid Quantity: ");             // Output separator
    Serial.print(totalLitres);
    Serial.println("L");
    pulseCount = 0;
    attachInterrupt(PULSE_PIN, pulseCounter, FALLING);
    String cs=""+ String(flowMilliLitres)+"";
   
    client.loop();
      while (!client.connect(clientID,mqtt_username,mqtt_password)) {
       delay(2000);
      }
    if (client.publish(topic, String(cs).c_str())) {
    Serial.println("Flow sent!");
    Serial.println(String(cs).c_str());
    }
    client.disconnect();
  }

}
