#include <FastLED.h>

// 灯环配置
#define MAIN_LED_PIN     19
#define MAIN_NUM_LEDS    60
#define RING_LED_PIN     18
#define RING_NUM_LEDS    16

#define CHIPSET     WS2812B
#define COLOR_ORDER GRB

CRGB mainLeds[MAIN_NUM_LEDS];
CRGB ringLeds[RING_NUM_LEDS];

// 人体传感器引脚
#define MOTION_SENSOR_PIN 15
const int BOARD_LED_PIN = 2;

// 状态定义
enum SystemState {
  STATE_OFF,
  STATE_WAKE_UP,
  STATE_NORMAL,
  STATE_FADE_OUT
};

SystemState currentState = STATE_OFF;

// 变量声明
bool lastMotionState = false;
bool currentMotionState = false;
unsigned long lastMotionTime = 0;

// 亮度参数
const uint16_t WAKE_UP_DURATION = 2500;
const uint16_t FADE_OUT_DURATION = 2000;
const uint8_t TARGET_BRIGHTNESS = 120;

// 非阻塞延迟变量
unsigned long previousMillis = 0;
const long UPDATE_INTERVAL = 30; // 更快的更新，使动画更平滑

// 自然光参数
const uint8_t WARM_WHITE_HUE = 30;
const uint8_t WARM_WHITE_SATURATION = 50;

// 星光系统参数
const uint8_t MAX_STARS = 8; // 最大星光点数
unsigned long lastStarSpawn = 0;
const long STAR_SPAWN_INTERVAL = 800; // 每800毫秒尝试生成一个新星

// 星光点数据结构 - 完整的生命周期管理
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

// 信号稳定性增强
void stableShow() {
  delayMicroseconds(50);
  FastLED.show();
  delayMicroseconds(50);
}

// 稳定的暖白色调
CRGB getWarmWhite() {
  return CHSV(WARM_WHITE_HUE, WARM_WHITE_SATURATION, TARGET_BRIGHTNESS);
}

// 初始化星光系统
void initStarSystem() {
  for (int i = 0; i < MAX_STARS; i++) {
    stars[i].active = false;
  }
}

// 获取活跃星光点数
uint8_t getActiveStarCount() {
  uint8_t count = 0;
  for (int i = 0; i < MAX_STARS; i++) {
    if (stars[i].active) count++;
  }
  return count;
}

// 生成新的星光点
void spawnStar() {
  // 寻找空闲的星光槽位
  for (int i = 0; i < MAX_STARS; i++) {
    if (!stars[i].active) {
      // 随机位置，避免与现有星光太近
      bool positionValid = false;
      int attempts = 0;
      
      while (!positionValid && attempts < 20) {
        stars[i].position = random16(RING_NUM_LEDS);
        positionValid = true;
        
        // 检查是否与现有活跃星光太近
        for (int j = 0; j < MAX_STARS; j++) {
          if (j != i && stars[j].active) {
            int distance = abs(stars[i].position - stars[j].position);
            if (distance <= 2) { // 至少间隔2个LED
              positionValid = false;
              break;
            }
          }
        }
        attempts++;
      }
      
      // 随机属性
      stars[i].hue = WARM_WHITE_HUE + random8(20) - 10; // 暖白色调附近轻微变化
      stars[i].saturation = 20 + random8(30); // 低饱和度，更接近白色
      stars[i].brightness = 0;
      stars[i].targetBrightness = 200 + random8(55); // 200-255亮度
      stars[i].birthTime = millis();
      stars[i].lifeDuration = 3000 + random16(7000); // 3-10秒生命周期
      stars[i].fadeInDuration = 800 + random16(1200); // 0.8-2秒淡入
      stars[i].fadeOutDuration = 1000 + random16(2000); // 1-3秒淡出
      stars[i].phase = 0; // 淡入阶段
      stars[i].active = true;
      
      Serial.print("✨ 新生星点 #");
      Serial.print(i);
      Serial.print(" 位置:");
      Serial.print(stars[i].position);
      Serial.print(" 寿命:");
      Serial.print(stars[i].lifeDuration);
      Serial.println("ms");
      break;
    }
  }
}

// 更新星光点状态
void updateStars() {
  unsigned long currentTime = millis();
  
  for (int i = 0; i < MAX_STARS; i++) {
    if (stars[i].active) {
      unsigned long starAge = currentTime - stars[i].birthTime;
      
      // 检查生命周期是否结束
      if (starAge > stars[i].lifeDuration) {
        stars[i].active = false;
        continue;
      }
      
      // 根据阶段更新亮度
      switch (stars[i].phase) {
        case 0: // 淡入阶段
          if (starAge < stars[i].fadeInDuration) {
            // 计算淡入进度 (0-255)
            uint16_t progress = (starAge * 256) / stars[i].fadeInDuration;
            stars[i].brightness = (stars[i].targetBrightness * progress) / 256;
          } else {
            // 淡入完成，进入稳定阶段
            stars[i].brightness = stars[i].targetBrightness;
            stars[i].phase = 1;
          }
          break;
          
        case 1: // 稳定阶段
          // 保持目标亮度，直到需要开始淡出
          if (starAge > stars[i].lifeDuration - stars[i].fadeOutDuration) {
            stars[i].phase = 2; // 开始淡出
          }
          break;
          
        case 2: // 淡出阶段
          {
            unsigned long timeInFadeOut = starAge - (stars[i].lifeDuration - stars[i].fadeOutDuration);
            uint16_t progress = (timeInFadeOut * 256) / stars[i].fadeOutDuration;
            stars[i].brightness = stars[i].targetBrightness - ((stars[i].targetBrightness * progress) / 256);
          }
          break;
      }
    }
  }
}

// 渲染星光点到环形灯
void renderStars() {
  // 先清除环形灯
  fill_solid(ringLeds, RING_NUM_LEDS, CRGB::Black);
  
  // 渲染所有活跃的星光点
  for (int i = 0; i < MAX_STARS; i++) {
    if (stars[i].active) {
      ringLeds[stars[i].position] = CHSV(stars[i].hue, stars[i].saturation, stars[i].brightness);
    }
  }
}

// 尝试生成新星
void trySpawnStar() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastStarSpawn > STAR_SPAWN_INTERVAL) {
    lastStarSpawn = currentTime;
    
    uint8_t activeCount = getActiveStarCount();
    
    // 根据当前活跃星数决定生成概率
    uint8_t spawnChance = 0;
    if (activeCount < 3) spawnChance = 80;      // 星少时高概率生成
    else if (activeCount < 5) spawnChance = 50; // 中等数量中等概率
    else if (activeCount < 7) spawnChance = 20; // 星多时低概率
    
    if (random8(100) < spawnChance) {
      spawnStar();
    }
  }
}

// 淡出效果
bool fadeOut() {
  static uint32_t startTime = 0;
  static uint8_t startBrightness = 0;
  
  if (startTime == 0) {
    startTime = millis();
    startBrightness = FastLED.getBrightness();
    // 淡出时重置星光系统
    initStarSystem();
  }
  
  uint32_t elapsedTime = millis() - startTime;
  if (elapsedTime >= FADE_OUT_DURATION) {
    FastLED.setBrightness(0);
    stableShow();
    startTime = 0;
    return true;
  }
  
  uint8_t brightness = startBrightness * (FADE_OUT_DURATION - elapsedTime) / FADE_OUT_DURATION;
  FastLED.setBrightness(brightness);
  stableShow();
  return false;
}

// 淡入效果
bool wakeUp() {
  static uint32_t startTime = 0;
  
  if (startTime == 0) {
    startTime = millis();
    fill_solid(mainLeds, MAIN_NUM_LEDS, CRGB::Black);
    fill_solid(ringLeds, RING_NUM_LEDS, CRGB::Black);
    initStarSystem(); // 初始化星光系统
  }
  
  uint32_t elapsedTime = millis() - startTime;
  if (elapsedTime >= WAKE_UP_DURATION) {
    FastLED.setBrightness(TARGET_BRIGHTNESS);
    stableShow();
    startTime = 0;
    return true;
  }
  
  uint8_t brightness = TARGET_BRIGHTNESS * elapsedTime / WAKE_UP_DURATION;
  FastLED.setBrightness(brightness);
  
  // 从中心向外扩散
  uint8_t progress = (elapsedTime * 256) / WAKE_UP_DURATION;
  uint8_t litLeds = scale8(MAIN_NUM_LEDS, progress);
  
  fill_solid(mainLeds, MAIN_NUM_LEDS, CRGB::Black);
  for (int i = 0; i < litLeds; i++) {
    int pos1 = (MAIN_NUM_LEDS/2) + i/2;
    int pos2 = (MAIN_NUM_LEDS/2) - i/2;
    if (pos1 < MAIN_NUM_LEDS) mainLeds[pos1] = getWarmWhite();
    if (pos2 >= 0) mainLeds[pos2] = getWarmWhite();
  }
  
  // 环形灯保持黑色，星光效果在NORMAL状态才开始
  fill_solid(ringLeds, RING_NUM_LEDS, CRGB::Black);
  
  stableShow();
  return false;
}

void setup() {
  Serial.begin(115200);
  
  pinMode(BOARD_LED_PIN, OUTPUT);
  pinMode(MOTION_SENSOR_PIN, INPUT);
  
  Serial.println("====================================");
  Serial.println("双灯环系统 - 高级星光模式");
  Serial.println("等待人体移动...");
  Serial.println("====================================");
  
  // 初始化两个灯环
  FastLED.addLeds<CHIPSET, MAIN_LED_PIN, COLOR_ORDER>(mainLeds, MAIN_NUM_LEDS);
  FastLED.addLeds<CHIPSET, RING_LED_PIN, COLOR_ORDER>(ringLeds, RING_NUM_LEDS);
  
  FastLED.setBrightness(0);
  FastLED.clear();
  stableShow();
  
  // 初始化星光系统
  initStarSystem();
  
  // 设置随机种子
  random16_set_seed(millis());
}

void loop() {
  // 读取人体检测模块状态
  currentMotionState = digitalRead(MOTION_SENSOR_PIN);
  
  // 检测到状态变化
  if (currentMotionState != lastMotionState) {
    lastMotionState = currentMotionState;
    lastMotionTime = millis();
    
    if (currentMotionState) {
      Serial.println("🚶 检测到人体移动！");
      digitalWrite(BOARD_LED_PIN, HIGH);
      
      if (currentState == STATE_OFF || currentState == STATE_FADE_OUT) {
        currentState = STATE_WAKE_UP;
      }
    } else {
      Serial.println("💤 无人体移动");
      digitalWrite(BOARD_LED_PIN, LOW);
      
      if (currentState == STATE_NORMAL) {
        currentState = STATE_FADE_OUT;
      }
    }
  }
  
  // 状态机处理
  switch (currentState) {
    case STATE_OFF:
      break;
      
    case STATE_WAKE_UP:
      if (wakeUp()) {
        currentState = STATE_NORMAL;
        lastStarSpawn = millis(); // 开始生成星光
      }
      break;
      
    case STATE_NORMAL:
      {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= UPDATE_INTERVAL) {
          previousMillis = currentMillis;
          
          // 主灯环 - 稳定暖白色
          fill_solid(mainLeds, MAIN_NUM_LEDS, getWarmWhite());
          
          // 星光系统更新
          trySpawnStar();  // 尝试生成新星
          updateStars();   // 更新所有星光状态
          renderStars();   // 渲染到环形灯
          
          stableShow();
        }
      }
      break;
      
    case STATE_FADE_OUT:
      if (fadeOut()) {
        currentState = STATE_OFF;
      }
      break;
  }
  
  // 如果检测到移动，定期输出状态
  if (currentMotionState && currentState == STATE_NORMAL) {
    unsigned long currentTime = millis();
    if (currentTime - lastMotionTime > 15000) {
      uint8_t starCount = getActiveStarCount();
      Serial.print("📍 人体仍在检测范围内，活跃星点:");
      Serial.println(starCount);
      lastMotionTime = currentTime;
    }
  }
}