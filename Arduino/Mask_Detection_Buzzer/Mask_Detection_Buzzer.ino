#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Ethernet shield configuration
byte mac[] = { 0xDE, 0xAD, 0xBE, 38, 0xFE, 0xED };
IPAddress ip(172, 16, 0, 100); // Change this to your MQTT broker's IP address
IPAddress server(44, 195, 202, 69);

// MQTT broker details
const char* mqtt_topic = "mask_alert";

// Buzzer setup
const int buzzerPin = 5; // GP1 in WIZnet modules is typically mapped to Arduino digital pin 1

EthernetClient ethClient;
PubSubClient client(ethClient);


void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Received message: ");
  Serial.println(message);

  if (message == "Mask not detected!") {
    // Buzz the buzzer for 2 seconds (adjust as needed)
    digitalWrite(buzzerPin, HIGH);
    delay(2000);
    digitalWrite(buzzerPin, LOW);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect("arduinoClient38")) {
      Serial.println("Connected to MQTT broker");
        // Once connected, publish an announcement...
      client.publish("Out", "hello world");
      // ... and resubscribe
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("Connection failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void setup() {
  
  Serial.begin(57600);
  Ethernet.init(17);
  client.setServer(server, 1883);
  client.setCallback(callback);

  Ethernet.begin(mac);
  delay(1500);

  pinMode(buzzerPin, OUTPUT);
  
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}