#include <Arduino.h>
#include "config.h"
#include "LED_Controller.h"
#include "motion_sensor.h"

// 使用全局实例
extern LEDController ledController;
extern MotionSensor motionsensor;

void setup()
{
    Serial.begin(115200);

    pinMode(Config::BOARD_LED_PIN, OUTPUT);
    pinMode(Config::MOTION_SENSOR_PIN, INPUT);

    Serial.println("====================================");
    Serial.println("双灯环系统启动 - WiFi控制版");
    Serial.println("====================================");

    // 先连接WiFi，再初始化LED控制器
    Serial.println("正在连接WiFi...");

    // 配置静态IP
    if (!WiFi.config(Config::localIP(), Config::gateway(), Config::subnet()))
    {
        Serial.println("STA Failed to configure");
    }

    WiFi.begin(Config::wifiSSID(), Config::wifiPassword());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30)
    {
        delay(500);
        Serial.print(".");
        attempts++;
        digitalWrite(Config::BOARD_LED_PIN, !digitalRead(Config::BOARD_LED_PIN));
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nWiFi连接成功!");
        Serial.print("IP地址: ");
        Serial.println(WiFi.localIP());
        digitalWrite(Config::BOARD_LED_PIN, HIGH);
    }
    else
    {
        Serial.println("\nWiFi连接失败，使用离线模式");
        digitalWrite(Config::BOARD_LED_PIN, LOW);
    }

    // WiFi连接成功后再初始化LED控制器
    ledController.begin();

    if (WiFi.status() == WL_CONNECTED)
    {
        ledController.quickTestLeds();
    }
}

void loop()
{
    // 处理网络请求
    ledController.handleClient();

    // 更新传感器状态
    motionsensor.CheckMotion();

    // 更新LED状态
    ledController.update();
}