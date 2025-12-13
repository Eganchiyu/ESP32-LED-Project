#include "motion_sensor.h"
#include <Arduino.h>

MotionSensor motionsensor;

MotionSensor::MotionSensor()
    : lastMotionState(0),
      currentMotionState(0)
{
}

void MotionSensor::CheckMotion(int force)
{
    // 只有在自动模式的时候才触发这个状态
    /*
    STATE_AUTO_BREATH,
    STATE_AUTO_FADE_IN,
    STATE_AUTO_NORMAL,
    STATE_AUTO_FADE_OUT,
    STATE_AUTO_OFF,
    */
    if (ledController.getState() == STATE_AUTO_NORMAL ||
        ledController.getState() == STATE_AUTO_OFF ||
        force == 1)
    {

        // 读取人体检测模块状态
        lastMotionState = currentMotionState;
        currentMotionState = digitalRead(Config::MOTION_SENSOR_PIN);

        // 检测到状态变化
        if ((currentMotionState != lastMotionState) || force == 1)
        {
            lastMotionState = currentMotionState;
            // lastMotionTime = millis();
            if (currentMotionState)
            {
                Serial.println("🚶 检测到人体移动！");
                // // digitalWrite(Config::BOARD_LED_PIN, HIGH);
                ledController.setState(STATE_AUTO_BREATH);
                ledController.setBreathStep(0);
                ledController.setStartHue(0);
            }
            else
            {
                Serial.println("💤 无人体移动");
                // digitalWrite(Config::BOARD_LED_PIN, LOW);
                ledController.setState(STATE_AUTO_FADE_OUT);
            }
        }
        force = 0;
    }
    return;
}