#include <WiFi.h>

const char* ssid = "uav";      // Wi-Fi network name
const char* password = "aesnr123";  // Wi-Fi network password
const char* server_address = "10.42.0.1";  // Jetson NX hotspot IP address
const int server_port = 80;  // The port number on which to receive messages

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Connect to Wi-Fi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi network");
}

void loop() {
  // Send "123" message
  WiFiClient client;
  if (client.connect(server_address, server_port)) {
    Serial.println("Sending message...");
    client.print("123");
    client.stop();
    Serial.println("Message sent");
  }
  else {
    Serial.println("Unable to connect to server");
  }
  
  delay(1000);  // Wait for 1 second before sending the next message
}
