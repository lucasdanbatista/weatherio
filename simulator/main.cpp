#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <ArduinoMqttClient.h>

DHT dht(22, DHT11);
WiFiClient wifiClient;
MqttClient brokerClient(wifiClient);

void humidity_display_update()
{
    float value = dht.readHumidity();
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%.1f%%", value);
    brokerClient.beginMessage("lucasdanbatista/simulator/humidity");
    brokerClient.print(value);
    brokerClient.endMessage();
}

void temperature_display_update()
{
    float value = dht.readTemperature();
    float percentageValue = (int32_t)((value / 50.0f) * 100);
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%.1f C", value);
    brokerClient.beginMessage("lucasdanbatista/simulator/temperature");
    brokerClient.print(value);
    brokerClient.endMessage();
}

bool connect_wifi()
{
    WiFi.mode(WIFI_MODE_STA);
    WiFi.begin("Wokwi-GUEST", "", 6);
    while (WiFi.status() != WL_CONNECTED)
    {
        if (WiFi.status() == WL_CONNECT_FAILED)
        {
            return false;
        }
    }
    return true;
}

bool connect_broker()
{
    brokerClient.connect("broker.hivemq.com", 1883);
    while (!brokerClient.connected())
    {
        if (brokerClient.connectError())
        {
            return false;
        }
    }
    return true;
}

void setup()
{
    Serial.begin(115200);
    dht.begin();
    if (connect_wifi())
    {
        connect_broker();
    }
}

void loop()
{
    delay(1000);
    humidity_display_update();
    temperature_display_update();
    brokerClient.poll();
}