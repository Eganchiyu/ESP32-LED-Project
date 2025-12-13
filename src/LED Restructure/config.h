#ifndef CONFIG_H
#define CONFIG_H

#include <FastLED.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>

class Config
{
public:
  // WiFi配置
  static const char *wifiSSID() { return "基沃托斯"; }
  static const char *wifiPassword() { return "1111111q"; }

  // 网络配置
  static IPAddress localIP() { return IPAddress(192, 168, 31, 100); }
  static IPAddress gateway() { return IPAddress(192, 168, 31, 1); }
  static IPAddress subnet() { return IPAddress(255, 255, 255, 0); }
  static uint16_t serverPort() { return 80; }

  // 硬件引脚
  static constexpr int MAIN_LED_PIN = 19;
  static constexpr int RING_LED_PIN = 18;
  static constexpr int MOTION_SENSOR_PIN = 15;
  static constexpr int BOARD_LED_PIN = 2;

  // LED数量
  static constexpr int MAIN_NUM_LEDS = 60;
  static constexpr int RING_NUM_LEDS = 16;

  // 动画参数
  static constexpr uint16_t BREATHE_STEPS = 120;
  static constexpr uint16_t BREATHE_DURATION_MS = 750;
  static constexpr uint16_t FADE_IN_MS = 1000;
  static constexpr uint16_t FADE_OUT_MS = 1000;
  static constexpr long NORMAL_UPDATE_INTERVAL = 30;
};

// 状态枚举
enum SystemState
{
  STATE_AUTO_BREATH,
  STATE_AUTO_FADE_IN,
  STATE_AUTO_NORMAL,
  STATE_AUTO_FADE_OUT,
  STATE_AUTO_OFF,

  // STATE_BREATHE_LOOP,
  STATE_OFF,
  STATE_BREATHE,
  STATE_FADE_IN,
  STATE_NORMAL,
  STATE_FADE_OUT,
  STATE_MANUAL
  
};
#endif