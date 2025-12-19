#include <Arduino.h>
#include <config.h>
#include <LED_Controller.h>
class BreathStarlight {
public:
    BreathStarlight();
    void begin();
    void update(uint32_t now);

private:

    // 自然光参数
    const uint8_t WARM_WHITE_HUE;
    const uint8_t WARM_WHITE_SATURATION;
    CRGB* mainLeds;
    CRGB* ringLeds;
    const uint16_t WAKE_UP_DURATION;
    const uint16_t FADE_OUT_DURATION;
    const uint8_t TARGET_BRIGHTNESS;
    // 星光系统参数
    static const uint8_t MAX_STARS = 8; // 最大星光点数
    unsigned long lastStarSpawn;
    const long STAR_SPAWN_INTERVAL; // 每800毫秒尝试生成一个新星
    // float basePhase;
    // uint32_t lastSparkleTime;

    struct Star {
        int position;           // 在环形灯上的位置
        uint8_t hue;            // 色调 (可以有些微变化)
        uint8_t saturation;     // 饱和度
        uint8_t brightness;     // 当前亮度
        uint8_t targetBrightness; // 目标亮度
        unsigned long birthTime; // 生成时间
        unsigned long lifeDuration; // 生命周期时长(毫秒)
        uint16_t fadeInDuration;   // 淡入时长
        uint16_t fadeOutDuration;  // 淡出时长
        bool active;            // 是否活跃
        uint8_t phase;          // 0:淡入, 1:稳定, 2:淡出
    };

    Star stars[MAX_STARS];
    void begin();
    CRGB getWarmWhite();
    void initStarSystem();
    uint8_t getActiveStarCount();
    void spawnStar();
    void updateStars();
    void renderStars();
    void trySpawnStar();
    bool fadeOut();
    bool wakeUp();
    void stableShow();
};

extern BreathStarlight breathStarlight;