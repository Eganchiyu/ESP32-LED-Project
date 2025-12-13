#ifndef MOTION_SENSOR
#define MOTION_SENSOR

#include <Arduino.h>
#include "config.h"
#include "LED_Controller.h"

class MotionSensor
{
private:
    // 私有成员变量
    bool lastMotionState;
    bool currentMotionState;

public:
    // 构造函数
    MotionSensor();

    // 公共接口
    void CheckMotion(int force = 0);
};

extern MotionSensor motionsensor;

#endif