#include <Breath_Starlight.h>

//生成实例
BreathStarlight breathStarlight;
//构造实例
BreathStarlight::BreathStarlight()
    : WARM_WHITE_HUE(30),
    WARM_WHITE_SATURATION(50),
    lastStarSpawn(0),
    STAR_SPAWN_INTERVAL(800), // 每800毫秒尝试生成一个新星
    WAKE_UP_DURATION(3000),
    FADE_OUT_DURATION(2000),
    TARGET_BRIGHTNESS(63),
    UPDATE_INTERVAL(10), // 更快的更新，使动画更平滑
    previousMillis(0)
{   
}

void BreathStarlight::begin(CRGB* main, CRGB* ring) {
    mainLeds = main;
    ringLeds = ring;
    initStarSystem();
}

// 稳定的暖白色调->返回CRGB值
CRGB BreathStarlight::getWarmWhite() {
  return CHSV(WARM_WHITE_HUE, WARM_WHITE_SATURATION, TARGET_BRIGHTNESS);
}

// 初始化星光系统->将星光点活动性全部设成false
void BreathStarlight::initStarSystem() {
  for (int i = 0; i < MAX_STARS; i++) {
    stars[i].active = false;
  }
}

// 获取活跃星光点数->uint8_t
uint8_t BreathStarlight::getActiveStarCount() {
  uint8_t count = 0;
  for (int i = 0; i < MAX_STARS; i++) {
    if (stars[i].active) count++;
  }
  return count;
}

// 生成新的星光点->包含位置判断和star结构体更新
void BreathStarlight::spawnStar() {
  // 寻找空闲的星光槽位
  for (int i = 0; i < MAX_STARS; i++) {
    if (!stars[i].active) {
      // 随机位置，避免与现有星光太近
      bool positionValid = false;
      int attempts = 0;
      
      while (!positionValid && attempts < 20) {
        stars[i].position = random16(Config::RING_NUM_LEDS);
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
      stars[i].targetBrightness = 100 + random8(55); // 200-255亮度
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

// 尝试生成新星->按时间带概率生成新星
void BreathStarlight::trySpawnStar() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastStarSpawn > STAR_SPAWN_INTERVAL) {
    lastStarSpawn = currentTime;
    
    uint8_t activeCount = getActiveStarCount();
    
    // 根据当前活跃星数决定生成概率
    uint8_t spawnChance = 0;
    if (activeCount < 3) spawnChance = 60;      // 星少时高概率生成
    else if (activeCount < 5) spawnChance = 50; // 中等数量中等概率
    else if (activeCount < 7) spawnChance = 30; // 星多时低概率
    
    if (random8(100) < spawnChance) {
      spawnStar();
    }
  }
}

// 更新星光点状态->星点阶段演变
void BreathStarlight::updateStars() {
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

// 渲染星光点到环形灯->包含了渲染到灯环操作，不包括FastLED.show()
void BreathStarlight::renderStars() {
  // 先清除环形灯
  fill_solid(ringLeds, Config::RING_NUM_LEDS, CRGB::Black);
  
  // 渲染所有活跃的星光点
  for (int i = 0; i < MAX_STARS; i++) {
    if (stars[i].active) {
      ringLeds[stars[i].position] = CHSV(stars[i].hue, stars[i].saturation, stars[i].brightness);
    }
  }
  // stableShow();
}

//FastLED.show()
void BreathStarlight::stableShow()
{
  delayMicroseconds(50);
  FastLED.show();
  delayMicroseconds(50);
}

// 淡出效果
bool BreathStarlight::fadeOut() {
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
bool BreathStarlight::wakeUp() {
  static uint32_t startTime = 0;
  
  // 初始化星光系统，清除灯光，填充黑色
  if (startTime == 0) {
    startTime = millis();
    fill_solid(mainLeds, Config::MAIN_NUM_LEDS, CRGB::Black);
    fill_solid(ringLeds, Config::RING_NUM_LEDS, CRGB::Black);
    initStarSystem(); 
  }
  
  uint32_t elapsedTime = millis() - startTime;

  //函数结束，返回true
  if (elapsedTime >= WAKE_UP_DURATION) {
    FastLED.setBrightness(TARGET_BRIGHTNESS);
    stableShow();
    startTime = 0;
    lastStarSpawn = millis();
    random16_set_seed(millis());
    return true;
  }
  
  //按照设定时间线性渐亮（调高系统亮度到TARGET_BRIGHTNESS）
  uint8_t brightness = TARGET_BRIGHTNESS * elapsedTime / WAKE_UP_DURATION;
  FastLED.setBrightness(brightness);
  
  // 从中心向外扩散
  uint8_t progress = (elapsedTime * 256) / WAKE_UP_DURATION;
  uint8_t litLeds = scale8(Config::MAIN_NUM_LEDS, progress);
  
  fill_solid(mainLeds, Config::MAIN_NUM_LEDS, CRGB::Black);
  for (int i = 0; i < litLeds; i++) {
    int pos1 = (Config::MAIN_NUM_LEDS/2) + i/2;
    int pos2 = (Config::MAIN_NUM_LEDS/2) - i/2;
    if (pos1 < Config::MAIN_NUM_LEDS) mainLeds[pos1] = getWarmWhite();
    if (pos2 >= 0) mainLeds[pos2] = getWarmWhite();
  }
  
  // 环形灯保持黑色，星光效果在NORMAL状态才开始
  fill_solid(ringLeds, Config::RING_NUM_LEDS, CRGB::Black);
  
  stableShow();
  return false;
}

void BreathStarlight::STATE_normal(){
  unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= UPDATE_INTERVAL) {
      
      previousMillis = currentMillis;

      // 主灯环 - 稳定暖白色
      fill_solid(mainLeds, Config::MAIN_NUM_LEDS, getWarmWhite());

      // 星光系统更新
      trySpawnStar();  // 尝试生成新星
      updateStars();   // 更新所有星光状态
      renderStars();   // 渲染到环形灯

      stableShow();
    }
}

/*
下一步内容：

*/