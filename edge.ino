#include <DHT.h>
#include <PubSubClient.h>
#include <WiFi.h>

#define DHTPIN 5
#define DHTTYPE DHT22
#define FAN 4
#define LAMP 2

// Tambah buffer size yang lebih besar
#define MQTT_MAX_PACKET_SIZE 512

DHT dht(DHTPIN, DHTTYPE);

const char *ssid = "TP";
const char *password = "123@khalid";
const char *mqtt_server = "192.168.1.7";

WiFiClient espClient;
PubSubClient client(espClient);

// ✅ Gunakan millis() bukan delay()
unsigned long lastPublish = 0;
const unsigned long publishInterval = 5000;

void callback(char *topic, byte *message, unsigned int length) {
  Serial.println("\n📩 MQTT Message Masuk");

  char payload[128];
  if (length >= sizeof(payload))
    length = sizeof(payload) - 1;
  for (unsigned int i = 0; i < length; i++)
    payload[i] = (char)message[i];
  payload[length] = '\0';

  Serial.print("📌 Topic: ");
  Serial.println(topic);
  Serial.print("📌 Message: ");
  Serial.println(payload);

  if (strcmp(topic, "kampus/kipas") == 0) {
    if (strcmp(payload, "ON") == 0) {
      digitalWrite(FAN, HIGH);
      Serial.println("🌀 FAN: ON");
    } else {
      digitalWrite(FAN, LOW);
      Serial.println("🌀 FAN: OFF");
    }
  }

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

void reconnect() {
  while (!client.connected()) {
    Serial.print("🔄 Menghubungkan ke MQTT...");
    // ✅ Gunakan unique client ID
    String clientId = "ESP32-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("✅ Connected!");
      // ✅ Subscribe ulang setiap reconnect
      bool subResult = client.subscribe("kampus/#");
      Serial.print("📡 Subscribe kampus/# : ");
      Serial.println(subResult ? "OK" : "GAGAL");
    } else {
      Serial.print("❌ Gagal, rc=");
      Serial.print(client.state());
      Serial.println(" coba lagi 2 detik...");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(FAN, OUTPUT);
  pinMode(LAMP, OUTPUT);
  dht.begin();
  delay(2000);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connected!");
  Serial.println(WiFi.localIP());

  // ✅ Set buffer size lebih besar
  client.setBufferSize(512);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected())
    reconnect();

  // ✅ client.loop() selalu jalan tanpa diblokir delay
  client.loop();

  unsigned long now = millis();
  if (now - lastPublish >= publishInterval) {
    lastPublish = now;

    float suhu = dht.readTemperature();
    float hum = dht.readHumidity();

    if (isnan(suhu) || isnan(hum)) {
      suhu = random(200, 350) / 10.0;
      hum = random(600, 900) / 10.0;
    }

    String payload = "{\"temperature\":";
    payload += suhu;
    payload += "}";

    Serial.print("📤 Publish suhu: ");
    Serial.println(payload);
    client.publish("kampus/suhu", payload.c_str());
  }
}