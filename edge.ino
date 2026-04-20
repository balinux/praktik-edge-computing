#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTPIN 5       // D1 (AMAN)
#define DHTTYPE DHT22

#define FAN 4          // D2
#define LAMP 2         // D4 (LED built-in)

DHT dht(DHTPIN, DHTTYPE);

// const char* ssid = "Tomoro Selong";
// const char* password = "tomunity123";

// const char* mqtt_server = "192.168.1.192";

const char* ssid = "TP";
const char* password = "123@khalid";

const char* mqtt_server = "192.168.1.7";

WiFiClient espClient;
PubSubClient client(espClient);

// ================= CALLBACK MQTT =================
void callback(char* topic, byte* message, unsigned int length) {

  Serial.println("\n📩 MQTT Message Masuk");
  Serial.print("Raw topic: '"); Serial.print(topic); Serial.print("' len="); Serial.println(strlen(topic));
  Serial.print("Raw payload (hex): ");
  for (unsigned int i=0;i<length;i++){ Serial.print(message[i], HEX); Serial.print(" "); }
  Serial.println();

  // build null-terminated payload buffer
  char payload[128];
  if (length >= sizeof(payload)) length = sizeof(payload)-1;
  for (unsigned int i=0;i<length;i++) payload[i] = (char)message[i];
  payload[length] = '\0';

  Serial.print("📌 Topic: "); Serial.println(topic);
  Serial.print("📌 Message: "); Serial.println(payload);

  // === KONTROL FAN ===
  if (strcmp(topic, "kampus/kipas") == 0) {
    if (strcmp(payload, "ON") == 0) {
      digitalWrite(FAN, HIGH);
      Serial.println("🌀 FAN: ON");
    } else {
      digitalWrite(FAN, LOW);
      Serial.println("🌀 FAN: OFF");
    }
  }

  // === KONTROL LAMP ===
  if (strcmp(topic, "kampus/lampu") == 0) {
    if (strcmp(payload, "ON") == 0) {
      digitalWrite(LAMP, HIGH);
      Serial.println("💡 LAMP: ON");
    } else {
      digitalWrite(LAMP, LOW);
      Serial.println("💡 LAMP: OFF");
    }
  }
}

// ================= RECONNECT MQTT =================
void reconnect() {

  while (!client.connected()) {

    Serial.print("🔄 Menghubungkan ke MQTT...");

    if (client.connect("ESP32")) {
      Serial.println("✅ Connected!");

      client.subscribe("kampus/#");

      Serial.println("📡 Subscribe ke topik:");
      Serial.println(" - kampus/#");

    } else {
      Serial.print("❌ Gagal, rc=");
      Serial.print(client.state());
      Serial.println(" coba lagi 2 detik...");
      delay(2000);
    }
  }
}

// ================= SETUP =================
void setup() {

  Serial.begin(115200);
  Serial.println("\n🚀 ESP32 Starting...");

  pinMode(FAN, OUTPUT);
  pinMode(LAMP, OUTPUT);

  dht.begin();
  delay(2000);

  // === CONNECT WIFI ===
  Serial.print("📶 Menghubungkan ke WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ WiFi Connected!");
  Serial.print("📍 IP Address: ");
  Serial.println(WiFi.localIP());

  // === MQTT SETUP ===
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  Serial.println("🛰️ MQTT Server: ");
  Serial.println(mqtt_server);
}

// ================= LOOP =================
void loop() {

  if (!client.connected()) reconnect();

  client.loop();

  float suhu = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(suhu) || isnan(hum)) {
    Serial.println("⚠️ Gagal membaca sensor DHT!");
    // return;
    suhu = random(200,350) / 10.0; // 20.0 – 34.9
    hum  = random(600,900) / 10.0; // 60% - 90%
  }

  Serial.print("🌡️ Suhu: ");
  Serial.print(suhu);
  Serial.print(" °C | 💧 Humidity: ");
  Serial.print(hum);
  Serial.println(" %");

  String payload = "{";
  payload += "\"temperature\":";
  payload += suhu;
  payload += "}";

  Serial.print("📤 Publish ke MQTT (kampus/suhu): ");
  Serial.println(payload);

  client.publish("kampus/suhu", payload.c_str());

  Serial.println("⏳ Delay 5 detik...\n");

  delay(5000);
}