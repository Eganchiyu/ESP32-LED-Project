#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include "config.h"

class LEDController
{
private:
    // 私有成员变量
    WebServer server;
    bool currentMotionState;
    unsigned long previousMillis;
    uint16_t breatheStep;
    uint8_t startHue;
    uint8_t ringHue;
    uint8_t targetBrightness;
    uint8_t globalBrightness;
    uint8_t manualRed;
    uint8_t manualGreen; 
    uint8_t manualBlue;
    CRGB mainLeds[Config::MAIN_NUM_LEDS];
    CRGB ringLeds[Config::RING_NUM_LEDS];

    // 私有方法
    void stableShow();
    bool fadeOut();
    bool fadeIn();
    void setManualColor(uint8_t r, uint8_t g, uint8_t b);

public:

    SystemState currentState;
    SystemState lastState;
    // 构造函数
    LEDController();

    // 公共接口
    void begin();
    void setBrightness(uint8_t brightness);
    void setMode(const String &mode);
    void update();
    void handleClient();
    void quickTestLeds();

    // 网页处理函数
    void handleRoot();
    void handleControl();
    void handleNotFound();

    //处理跨文件资源访问
    SystemState getState() const;
    void setState(SystemState NewState);
    void setBreathStep(uint16_t NewBreathStep);
    void setStartHue(uint8_t hue);
};

// 全局实例声明
extern LEDController ledController;

#endif