#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include "DHT.h"
#include <Adafruit_Sensor.h>
#include <WiFiManager.h>
#include "config.h"

#define DHTTYPE DHT22

int RelayPum = 4; // D20
double PH = 0;    // D3
int LEDwifi = 2;
int LEDmqtt = 19;
int Reset = 16;
String SWauto;
#define SensorPin A0        // the pH meter Analog output is connected with the Arduino’s Analog
unsigned long int avgValue; // Store the average value of the sensor feedback
float b;
int buf[10], temp;

float humids;
float temps;
// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht[] = {{5, DHTTYPE}};
// Replace the next variables with your SSID/Password combination
// const char *ssid = "******";
// const char *password = "*******";
// Add your MQTT Broker IP address, example:
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
void callback(char *topic, byte *message, unsigned int length);
void setup_wifi();
void resetwifi();
String data_in;
String data_out;
WiFiManager wm;
bool res;
void setup()
{
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  pinMode(RelayPum, OUTPUT);
  pinMode(LEDwifi, OUTPUT);
  pinMode(LEDmqtt, OUTPUT);
  pinMode(Reset, INPUT);
  for (auto &sensor : dht)
  {
    sensor.begin();
  }
}
void setup_wifi()
{
  res = wm.autoConnect(ssid, password);
  if (!res)
  {
    Serial.println("Failed to connect");
  }
  else
  {

    Serial.println("connected");
    Serial.println(WiFi.SSID());
    digitalWrite(LEDwifi, HIGH);
  }
  // res = wm.autoConnect(); // auto generated AP name from chipid
  // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
}
void callback(char *topic, byte *message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  // Feel free to add more if statements to control more GPIOs with MQTT
  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off".
  // Changes the output state according to the message
  if (String(topic) == SUB_PUM)
  {
    Serial.print("Changing output to :");
    Serial.print(messageTemp);
    if (messageTemp == "on")
    {

      digitalWrite(RelayPum, HIGH);
      Serial.println("\nonpum");
    }
    else if (messageTemp == "off")
    {
      digitalWrite(RelayPum, LOW);
      Serial.println("\noffpum");
    }
  }
}
void resetwifi()
{
  if (digitalRead(Reset) == HIGH)
  {
    Serial.println("reset wifi and restart...!");
    wm.resetSettings();
    ESP.restart();
    digitalWrite(LEDwifi, LOW);
  }
}
String macToStr(const uint8_t *mac)
{
  String result;
  for (int i = 0; i < 6; ++i)
  {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    String clientName;
    clientName += "esp8266-";
    uint8_t mac[6];
    WiFi.macAddress(mac);
    clientName += macToStr(mac);
    clientName += "-";
    clientName += String(micros() & 0xff, 16);
    Serial.print("Connecting to ");
    Serial.print(mqtt_server);
    Serial.print(" as ");
    Serial.println(clientName);
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    if (client.connect((char *)clientName.c_str()))
    {
      // if (client.connect((char*) clientName.c_str()), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(SUB_PUM);
      client.subscribe(SUB_AutoSW);
      digitalWrite(LEDmqtt, HIGH);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      digitalWrite(LEDmqtt, LOW);
      // resetwifi();
      delay(5000);
    }
  }
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  // resetwifi();
  long now = millis();
  if (now - lastMsg > 60000)
  {
    lastMsg = now;
    humids = dht[0].readHumidity();
    temps = dht[0].readTemperature();
    String Humin = String(humids).c_str();
    String Temin = String(temps).c_str();
    if (isnan(humids))
    {
      Serial.println("\nHumindity 1 is not detected");
    }
    else
    {
      Serial.printf("\nhumid 1: %f", humids);
      Serial.printf("\ttemp 1: %f", temps);
      data_in = "{\"humidity\":" + Humin + ",\"temperature\":" + Temin + "}";
      Serial.print("\n");
      Serial.print(data_in);
      data_in.toCharArray(msg, (data_in.length() + 1));
      client.publish(PUB_Topic_DHT, msg);
    }
    for (int i = 0; i < 10; i++) // Get 10 sample value from the sensor for smooth the value
    {
      buf[i] = analogRead(SensorPin);
      delay(10);
    }
    for (int i = 0; i < 9; i++) // sort the analog from small to large
    {
      for (int j = i + 1; j < 10; j++)
      {
        if (buf[i] > buf[j])
        {
          temp = buf[i];
          buf[i] = buf[j];
          buf[j] = temp;
        }
      }
    }
    avgValue = 0;
    for (int i = 2; i < 8; i++) // take the average value of 6 center sample
      avgValue += buf[i];
    float phValue = (float)avgValue * 5.0 / 1024 / 6; // convert the analog into millivolt
    phValue = 3.5 * phValue;                          // convert the millivolt into pH value

    if (isnan(phValue))
    {
      Serial.println("\nPHSensor 1 is not detected");
    }
    else
    {
      Serial.print("    pH:");
      Serial.print(phValue, 2);
      Serial.println(" ");
      String PHSensor = String(phValue, 2).c_str();
      data_in = "{\"PH\":" + PHSensor + "}";
      Serial.print("\n");
      Serial.print(data_in);
      data_in.toCharArray(msg, (data_in.length() + 1));
      client.publish(PUB_Topic_PH, msg);
    }
  }
}