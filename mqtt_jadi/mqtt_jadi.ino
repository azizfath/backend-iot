#include <PubSubClient.h>

#include <WiFi.h> //USE WiFi.h for ESP32
 // #include <ESP8266WiFi.h>

const char * ssid = "Kost Santri 3";
const char * password = "wifinyarusak";
WiFiClient espClient;
PubSubClient mqttClient(espClient);

const char * mqtt_server = "mqtt.azizfath.my.id";
const int mqtt_port = 1883;
const char * mqtt_user = "iot";
const char * mqtt_pass = "kel8";
const char * device_id = "IotFp";
const char * topic1 = "fpiot/moisture";
const char * topic2 = "fpiot/ph";
const char * topic3 = "fpiot/relay_water";
const char * topic4 = "fpiot/relay_acid";
const char * topic5 = "fpiot/relay_base";
const char * topic6 = "fpiot/mode";
String state_relay_water = "0";
String state_relay_acid = "0";
String state_relay_base = "0";
String mode = "otomatis";

//millis var
long lastMS1 = 0;
long lastMS2 = 0;

//input pin
#define analogPhPin 32 //Pin Ph sensor
#define analogSoilPin 33 //Pin Soil Sensor

//pin pompa

int pump_acid = 25;
int pump_base = 26;
int pump_water = 27;

//variable Sensor
int ADCPhValue = 0; //ADC value from ph sensor
float phValue = 0.0; //pH value after conversion
int ADCSoilValue = 0; //ADC value from Soil sensor
float moisture = 0.0; //ADC Value from soil moisture sensor
String s_moisture, s_ph;

//Rata-rata dan standard deviasi untuk PH
double mAPh = 5.18614, sdAPh = 0.44691;
double mNPh = 7.26833, sdNPh = 0.40494;
double mBPh = 8.05706, sdBPh = 0.12776;
double pHAsam, pHNormal, pHBasa;

double mKLembab = 74.90909, sdKLembab = 3.40190;
double mNLembab = 84.12195, sdNLembab = 2.94275;
double lKering, lNormal;

void setup() {
    pinMode(pump_water, OUTPUT);
    pinMode(pump_acid, OUTPUT);
    pinMode(pump_base, OUTPUT);

    digitalWrite(pump_water, 1);
    digitalWrite(pump_acid, 1);
    digitalWrite(pump_base, 1);

    Serial.begin(115200);
    Serial.print("WiFi Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
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
            mqttClient.subscribe(topic6);
        } else {
            Serial.print("failed, reconnect= ");
            Serial.print(mqttClient.state());
            Serial.print(" try again in 3 seconds");
            delay(3000);
        }
    }
}

void loop() {
    if (!mqttClient.connected()) {
        reconnect();
    }
    mqttClient.loop();

    long now1 = millis();
    if (now1 - lastMS1 > 1000) {
        lastMS1 = now1;
        phSensor();
        soilSensor();
        naiveBayes();
        program();
    }

    long now2 = millis();
    if (now2 - lastMS2 > 10000) {
        lastMS2 = now2;
        if (mode == "otomatis") {
            mqttClient.publish(topic3, (char * ) state_relay_water.c_str());
            mqttClient.publish(topic4, (char * ) state_relay_acid.c_str());
            mqttClient.publish(topic5, (char * ) state_relay_base.c_str());
        }
    }

}

void mqttSub(char * topic, byte * payload, unsigned int length) {
    String message;

    for (int i = 0; i < length; i++) {
        message += (char) payload[i];
    }
    // PRINT KE SERIAL UNTUK NEW MESSAGE
    //  Serial.print("Message arrived on topic: ");
    //  Serial.print(topic);
    //  Serial.print(". Message: ");
    //  Serial.println(message);

    if (String(topic) == topic6) {
        mode = message;
        if (mode == "manual") {
            digitalWrite(pump_water, 1);
            digitalWrite(pump_acid, 1);
            digitalWrite(pump_base, 1);
        }
    }

    //relay water
    if (String(topic) == topic3) {
        if (mode == "manual") {
            if (message == "1") {
                digitalWrite(pump_water, 0);
                state_relay_water = "1";
            } else if (message == "0") {
                digitalWrite(pump_water, 1);
                state_relay_acid = "0";
            }
        }
    }
    if (String(topic) == topic4) {
        if (mode == "manual") {
            if (message == "1") {
                digitalWrite(pump_acid, 0);
                state_relay_acid = "1";
            } else if (message == "0") {
                digitalWrite(pump_acid, 1);
                state_relay_acid = "0";
            }
        }
    }
    if (String(topic) == topic5) {
        if (mode == "manual") {
            if (message == "1") {
                digitalWrite(pump_base, 0);
                state_relay_base = "1";
            } else if (message == "0") {
                digitalWrite(pump_base, 1);
                state_relay_base = "0";
            }
        }
    }

}

void phSensor() {
    //read the analog in value:
    ADCPhValue = analogRead(analogPhPin);

    //Mathematical conversion from ADC to pH
    //rumus didapat berdasarkan datasheet 
    //  phValue = (0.00659*ADCPhValue)+5.60423;
    phValue = (0.00523 * ADCPhValue) + 3.40597;

    s_ph = String(phValue);
}

void soilSensor() {
    //  ADCSoilValue = analogRead(analogSoilPin);

    digitalWrite(analogSoilPin, HIGH); // Turn the sensor ON
    delay(10); // Allow power to settle
    int val = analogRead(analogSoilPin); // Read the analog value form sensor
    digitalWrite(analogSoilPin, LOW); // Turn the sensor OFF   

    //Rumus konversi ADC ke persen
    int persen = (100 - ((val / 4095.00) * 100));
    //  moisture = map(persen,0,78,0,100);
    moisture = persen;
    s_moisture = String(moisture);
}

void naiveBayes() {
    phSensor();
    soilSensor();

    //PH
    pHAsam = (1 / (sqrt(2 * 3.14 * sdAPh))) * exp(-((pow((phValue - mAPh), 2))) / (2 * (pow((sdAPh), 2))));
    pHNormal = (1 / (sqrt(2 * 3.14 * sdNPh))) * exp(-((pow((phValue - mNPh), 2))) / (2 * (pow((sdNPh), 2))));
    pHBasa = (1 / (sqrt(2 * 3.14 * sdBPh))) * exp(-((pow((phValue - mBPh), 2))) / (2 * (pow((sdBPh), 2))));

    //kelembaban
    lKering = (1 / (sqrt(2 * 3.14 * sdKLembab))) * exp(-((pow((moisture - mKLembab), 2))) / (2 * (pow((sdKLembab), 2))));
    lNormal = (1 / (sqrt(2 * 3.14 * sdNLembab))) * exp(-((pow((moisture - mNLembab), 2))) / (2 * (pow((sdNLembab), 2))));
}

void program() {
    naiveBayes();

    Serial.print("ADC Sensor PH= ");
    Serial.print(ADCPhValue);
    Serial.print("  output Ph= ");
    Serial.println((phValue));
    Serial.print("ADC Sensor SoilMoisture= ");
    Serial.print(ADCSoilValue);
    Serial.print(" Kelembapan = ");
    Serial.print(moisture);
    Serial.println("%");

    mqttClient.publish(topic1, (char * ) s_moisture.c_str());
    mqttClient.publish(topic2, (char * ) s_ph.c_str());

    if (mode == "otomatis") {
        if (pHAsam > pHNormal && pHAsam > pHBasa) {
            digitalWrite(pump_base, 0);
            state_relay_acid = "0";
            state_relay_base = "1";
            mqttClient.publish(topic4, (char * ) state_relay_acid.c_str());
            mqttClient.publish(topic5, (char * ) state_relay_base.c_str());
            Serial.println("Tanah Asam");
        }

        if (pHNormal > pHAsam && pHNormal > pHBasa) {
            digitalWrite(pump_acid, 1);
            digitalWrite(pump_base, 1);
            state_relay_base = "0";
            state_relay_acid = "0";
            mqttClient.publish(topic4, (char * ) state_relay_acid.c_str());
            mqttClient.publish(topic5, (char * ) state_relay_base.c_str());
            Serial.println("Tanah Subur");
        }

        if (pHBasa > pHNormal && pHBasa > pHAsam) {
            digitalWrite(pump_acid, 0);
            state_relay_acid = "1";
            state_relay_base = "0";
            mqttClient.publish(topic4, (char * ) state_relay_acid.c_str());
            mqttClient.publish(topic5, (char * ) state_relay_base.c_str());
            Serial.println("Tanah Basa");
        }

        if (lKering > lNormal) {
            digitalWrite(pump_water, 0);
            state_relay_water = "1";
            mqttClient.publish(topic3, (char * ) state_relay_base.c_str());
        }

        if (lNormal > lKering) {
            digitalWrite(pump_water, 1);
            state_relay_water = "0";
            mqttClient.publish(topic3, (char * ) state_relay_base.c_str());
        }
    }
}