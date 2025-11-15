#include <Arduino.h>
#include <DHT.h>
#include <esp32_smartdisplay.h>
#include <ui/ui.h>
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
    lv_label_set_text(uic_humidityValue, buffer);
    lv_bar_set_value(uic_humidityBar, value, LV_ANIM_ON);
    brokerClient.beginMessage("graduacao/iot/grupo_5/umidade");
    brokerClient.print(value);
    brokerClient.endMessage();
}

void temperature_display_update()
{
    float value = dht.readTemperature();
    float percentageValue = (int32_t)((value / 50.0f) * 100);
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%.1f C", value);
    lv_label_set_text(uic_temperatureValue, buffer);
    lv_bar_set_value(uic_temperatureBar, percentageValue, LV_ANIM_ON);
    brokerClient.beginMessage("graduacao/iot/grupo_5/temperatura");
    brokerClient.print(value);
    brokerClient.endMessage();
}

void set_green_led()
{
    smartdisplay_set_led_color(lv_color32_t({.ch = {.blue = 0, .green = 255, .red = 0}}));
}

void set_red_led()
{
    smartdisplay_set_led_color(lv_color32_t({.ch = {.blue = 0, .green = 0, .red = 255}}));
}

void set_intermitent_led()
{
    smartdisplay_set_led_color(lv_color32_t({.ch = {.blue = 0, .green = 165, .red = 255}}));
    delay(1000);
    smartdisplay_set_led_color(lv_color32_t({.ch = {.blue = 0, .green = 0, .red = 0}}));
    delay(1000);
}

bool connect_wifi()
{
    WiFi.mode(WIFI_MODE_STA);
    WiFi.begin("weatherio", "weatherio");
    while (WiFi.status() != WL_CONNECTED)
    {
        set_intermitent_led();
        if (WiFi.status() == WL_CONNECT_FAILED)
        {
            set_red_led();
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
        set_intermitent_led();
        if (brokerClient.connectError())
        {
            set_red_led();
            return false;
        }
    }
    return true;
}

void setup()
{
    Serial.begin(115200);
    smartdisplay_init();
    ui_init();
    dht.begin();
    if (connect_wifi())
    {
        connect_broker();
    }
}

void check_status()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        set_intermitent_led();
    }
    else if (!brokerClient.connected())
    {
        set_red_led();
    }
    else
    {
        set_green_led();
    }
}

void loop()
{
    delay(10);
    check_status();
    lv_timer_handler();
    humidity_display_update();
    temperature_display_update();
    brokerClient.poll();
}
