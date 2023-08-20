#include <Ethernet.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "HardwareSerial.h"

const char* device_unique_id = "MaskDetection_" __DATE__ "_" __TIME__;

#define SERIAL_OUTPUT
#ifdef SERIAL_OUTPUT
#define PRINT(x)   Serial.print(x)
#define PRINTLN(x) Serial.println(x)
#else
#define PRINT(x)   
#define PRINTLN(x) 
#endif

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 38, 0x77, 0xF7
};
IPAddress ip(192, 168, 0, 77);
//IPAddress ip(10, 5, 15, 109);

// Enter the IP address of the server you're connecting to:
//IPAddress server(192, 168, 100, 2);
//IPAddress server(10, 5, 15, 78);
IPAddress server(10, 21, 70, 16);
IPAddress myDns(192, 168, 0, 1);

IPAddress mqtt_server(10, 21, 70, 16);
// IPAddress mqtt_server(44, 195, 202, 69);

// MQTT broker details
const char* mqtt_topic = "mask_alert";

const int buzzerPin = 15;

EthernetClient mqttClient;
PubSubClient mqtt_client(mqttClient);

//===================================================================================================


void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.println(message);

  if (message.equals("Mask not detected!")) {
    Serial.println("Activating buzzer.");
    digitalWrite(buzzerPin, HIGH);
    delay(1000);
    digitalWrite(buzzerPin, LOW);
    Serial.println("Buzzer deactivated.");
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    PRINT("MQTT: Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt_client.connect(device_unique_id)) {
      PRINTLN("MQTT: connected");
      // ... and resubscribe
      mqtt_client.subscribe(mqtt_topic);
    } else {
      PRINT("MQTT: failed, rc=");
      PRINT(mqtt_client.state());
      PRINTLN(" MQTT: try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(2000);
    }
  }
}

//===================================================================================================

void setup() {
    // Open serial communications and wait for port to open:
  Serial3.setRx(PC11);
  Serial3.setTx(PC10);  
  delay(50);
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  pinMode(buzzerPin, OUTPUT);

   // start the Ethernet connection:
  PRINTLN("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    PRINTLN("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      PRINTLN("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      PRINTLN("Ethernet cable is not connected.");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
  } else {
    PRINT("  DHCP assigned IP ");
    PRINTLN(Ethernet.localIP());
  }
  
  // give the Ethernet shield a second to initialize:
  delay(1000);

  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setCallback(callback);

  // give the Ethernet shield a second to initialize:
  delay(1000);

}

void loop() {

  if (!mqtt_client.connected()) {
    reconnect();
  }
  mqtt_client.loop();

}