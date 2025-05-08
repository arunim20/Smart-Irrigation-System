#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTPIN 4       // Adjust as needed
#define DHTTYPE DHT11
#define SOIL_PIN 34     // Analog pin for soil sensor
#define LED_PIN 2       // LED as actuator (irrigation)

const char* ssid = "YOUR SSID";
const char* password = "YOUR PASSWORD";
const char* mqttServer = "broker.emqx.io";
const int mqttPort = 1883;
const char* dataTopic = "esp32/agri/data";
const char* commandTopic = "esp32/agri/command";

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

//System Parameters
float soilVolume = 100.0; // mL
float minMoisturePercent = 40.0;
float lastMoisture = 0.0;
bool irrigation = false;
unsigned long lastPublish = 0;
const unsigned long publishInterval = 2000; // 2 seconds
bool manualMode = false;
unsigned long irrigationStart = 0;
bool waitingForIncrease = false;
float minRequiredWater = (minMoisturePercent / 100.0) * soilVolume;
int flag=0;
void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("esp32_client")) {
      Serial.println("connected");
      client.subscribe(commandTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  Serial.println("Recieved Command : ");
  
  for (int i = 0; i < length; i++) msg += (char)payload[i];
  Serial.println(msg);
  if (String(topic) == commandTopic && msg == "resume") {
    manualMode = false;
    Serial.println("🔁 Resume command received");
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  dht.begin();
  pinMode(LED_PIN, OUTPUT);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("WiFi connected");

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();
  client.setCallback(callback);
  unsigned long now = millis();
  if (now - lastPublish >= publishInterval) {
    lastPublish = now;

    float temp = dht.readTemperature();
    float humid = dht.readHumidity();
    int soil_raw = analogRead(SOIL_PIN);
    int soil = 100 - (soil_raw / 4095.0 * 100);
    
    if (isnan(temp) || isnan(humid)) {
      Serial.println("Sensor error");
      return;
    }

    float evap_ml = 0.2 * (temp - 15) * (1.0 - humid / 100.0);
    static float previousSoil = soil;
    float deltaMoisture = soil - previousSoil;
    float waterInSoil = (soil / 100.0) * soilVolume;
    if(irrigation && flag == 0){
      irrigationStart = millis();
      flag = 1;  
    } else if(!irrigation){
      flag = 0;
    }

    
    // Irrigation logic
    // bool irrigation = false;
    if (!manualMode && !irrigation && waterInSoil < (minRequiredWater + evap_ml)) {
      digitalWrite(LED_PIN, HIGH);
      irrigation = true;
      irrigationStart = millis();
      waitingForIncrease = true;
      lastMoisture = soil;
      Serial.println("💧 Starting irrigation");
    } else if(!manualMode && irrigation && soil>previousSoil){
      waitingForIncrease = false;
      if(waterInSoil > (minRequiredWater + evap_ml)){
        irrigation = false;
        Serial.println("✅ Moisture increased. Irrigation complete.");
        digitalWrite(LED_PIN, LOW);
      }
    } else if(!manualMode && irrigation && waitingForIncrease && millis() - irrigationStart > 5000) {
      // digitalWrite(LED_PIN, LOW);
      manualMode = true;
      irrigation = false;
      digitalWrite(LED_PIN, LOW);
      Serial.println("❌ No increase in soil moisture. Entering manual mode.");
    }


    float waterPoured = 0.0;
    if (!waitingForIncrease && !manualMode && irrigation) {
      if (deltaMoisture > 0) {
        waterPoured = (deltaMoisture / 100.0) * soilVolume;
      }
    }
    previousSoil = soil;

    // Compose JSON payload
    String payload = "{";
    payload += "\"temperature\": " + String(temp, 1) + ",";
    payload += "\"humidity\": " + String(humid, 1) + ",";
    payload += "\"soil_moisture\": " + String(soil) + ",";
    payload += "\"evaporation_ml\": " + String(evap_ml, 1) + ",";
    payload += "\"water_poured\":" + String(waterPoured, 1)+",";
    payload += "\"irrigation\": " + String(irrigation ? "true" : "false") + ",";
    payload += "\"manual_mode\": " + String(manualMode ? "true" : "false");
    payload += "}";

    client.publish(dataTopic, payload.c_str());
    Serial.println(payload);
  }

  delay(10); // Yield CPU to avoid watchdog reset
}
