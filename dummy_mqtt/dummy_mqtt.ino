#include <PubSubClient.h>
#include <ESP8266WiFi.h> //USE WiFi.h for ESP32

const char* ssid = "Kost Santri 3";
const char* password = "wifinyarusak";
WiFiClient espClient;
PubSubClient mqttClient(espClient);

const char* mqtt_server = "";
const int mqtt_port = 1883;
const char* mqtt_user = "";
const char* mqtt_pass = "";
const char* device_id = "";
const char* topic1 = "fpiot/moisture";
const char* topic2 = "fpiot/ph";
const char* topic3 = "fpiot/relay_water";
const char* topic4 = "fpiot/relay_acid";
const char* topic5 = "fpiot/relay_base";

long lastMS = 0;

void setup() {
  Serial.begin(115200);
  Serial.print("WiFi Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("-");
  }

  Serial.println("");
  Serial.println("Wifi connected");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());

  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttSub);

}

void mqttSub(char* topic, byte* payload, unsigned int length){
//UBAH BYTE MESSAGE MENJADI STRING>
  String message;

  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

 
// PRINT KE SERIAL UNTUK NEW MESSAGE
//  Serial.print("Message arrived on topic: ");
//  Serial.print(topic);
//  Serial.print(". Message: ");
//  Serial.println(message);


// TRIGGER JIKA ADA PERUBAHAN STATE DARI WEB / ANDROID MASUKAN SINI

//  if (String(topic) == "esp32/output") {
//    Serial.print("Changing output to ");
//    if(message == "on"){
//      Serial.println("on");
//      digitalWrite(relayOne, LOW);
//      mqttClient.publish("esp32/lightSensor/state", "on");
//    }
//    else if(message == "off"){
//      Serial.println("off");
//      digitalWrite(relayOne, HIGH);
//      mqttClient.publish("esp32/lightSensor/state", "off");
//    }
//  }
}

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.println("Attempting MQTT connection ...");
    
    if (mqttClient.connect(device_id, mqtt_user, mqtt_pass)) {
      Serial.println("MQTT Connected");
      mqttClient.subscribe(topic1);
      mqttClient.subscribe(topic2);
      mqttClient.subscribe(topic3);
      mqttClient.subscribe(topic4);
      mqttClient.subscribe(topic5);
    } else {
      Serial.print("failed, reconnect= ");
      Serial.print(mqttClient.state());
      Serial.print(" try again in 3 seconds");
      delay(3000);

    }
  }
}

void loop() {
  if (!mqttClient.connected()){
    reconnect();
  }
  mqttClient.loop();
  long now = millis();
  if(now - lastMS > 2000){
    lastMS = now;
    mqttClient.publish(topic1, "dummy moisture");
    mqttClient.publish(topic2, "dummy ph");
    mqttClient.publish(topic3, "dummy relay_water");
    mqttClient.publish(topic4, "dummy relay_acid");
    mqttClient.publish(topic5, "dummy relay_base");
  }
}
