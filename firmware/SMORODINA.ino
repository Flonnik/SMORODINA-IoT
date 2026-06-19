#include <ESP8266WiFiMulti.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include "GyverBME280.h"
#include <WiFiManager.h>

WiFiManager wm;
GyverBME280 bme;
ESP8266WiFiMulti wifiMulti;

#define LED_PIN 14

// WiFi AP SSID
#define WIFI_SSID "nalichnaya"
// WiFi password
#define WIFI_PASSWORD "annadomini"
// InfluxDB v2 server url, e.g. https://eu-central-1-1.aws.cloud2.influxdata.com (Use: InfluxDB UI -> Load Data -> Client Libraries)
#define INFLUXDB_URL "https://us-east-1-1.aws.cloud2.influxdata.com"
// InfluxDB v2 server or cloud API token (Use: InfluxDB UI -> Data -> API Tokens -> Generate API Token)
#define INFLUXDB_TOKEN "v372leg5SgZHm9a06KJdUQ_DcOha98o_e_aE6iYd1AVgPixNFcZc7q1_rILYGjHXm38ITf5TpgMJIB4TWmeJ8g=="
// InfluxDB v2 organization id (Use: InfluxDB UI -> User -> About -> Common Ids )
#define INFLUXDB_ORG "mousegerman09@gmail.com"
// InfluxDB v2 bucket name (Use: InfluxDB UI ->  Data -> Buckets)
#define INFLUXDB_BUCKET "chernika"

// Set timezone string according to https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
// Examples:
//  Pacific Time: "PST8PDT"
//  Eastern: "EST5EDT"
//  Japanesse: "JST-9"
//  Central Europe: "CET-1CEST,M3.5.0,M10.5.0/3"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
Point sensor("bmeshka");

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(115200);

  bme.begin();

  Serial.println("Starting WiFiManager...");

  WiFi.mode(WIFI_STA);

  if (!wm.autoConnect("SMORODINA_SETUP")) {
    Serial.println("Failed to connect");
    ESP.restart();
  }

  Serial.println("WiFi connected!");
  Serial.println(WiFi.SSID());

  Serial.println("Syncing time...");

  unsigned long t0 = millis();
  while (millis() - t0 < 3000) {   // visual sync window
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    delay(50);
  }

  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
  Serial.println("Time synced!");

  Serial.println("Connecting InfluxDB...");

  while (!client.validateConnection()) {
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    delay(50);
  }

  Serial.println("Influx connected!");
}

void loop() {

  if (wifiMulti.run() != WL_CONNECTED) {
  Serial.println("WiFi lost!");
  return;
}
  digitalWrite(LED_PIN, HIGH);
  Serial.println(" ");

  bme.oneMeasurement();

  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;

  Serial.println("Measurement done:");
  Serial.print("Temp: "); Serial.println(temperature);
  Serial.print("Humid: "); Serial.println(humidity);
  Serial.print("Pressure: "); Serial.println(pressure);
  Serial.print("RSSI: "); Serial.println(WiFi.RSSI());
  
  sensor.clearFields();
  sensor.addField("temperature", temperature);
  sensor.addField("humidity", humidity);
  sensor.addField("pressure", pressure);
  sensor.addField("rssi", WiFi.RSSI());

  Serial.println("Sending...");

  digitalWrite(LED_PIN, LOW);   // reset before send blink


  // blink pattern to show transmission result
  if (client.writePoint(sensor)) {
    Serial.println("Data sent yay");
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    }
  else {
    Serial.println("Failed bro!");
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
  }

  ESP.deepSleep(10e6);
}
