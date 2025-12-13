#include "LED_Controller.h"
#include "motion_sensor.h"
// #include "motion_sensor.cpp"
#include <ArduinoJson.h>
#include <Arduino.h>

// 初始化静态成员
LEDController ledController;
// MotionSensor motionsensor;

// 构造函数
LEDController::LEDController()
    : server(Config::serverPort()),
      currentState(STATE_OFF),
      previousMillis(0),
      breatheStep(0),
      startHue(0),
      ringHue(0),
      manualBlue(255),
      manualGreen(255),
      manualRed(255),
      targetBrightness(80),
      globalBrightness(80)
{
}

void LEDController::begin()
{
  // 初始化LED
  FastLED.addLeds<WS2812B, Config::MAIN_LED_PIN, GRB>(mainLeds, Config::MAIN_NUM_LEDS);
  FastLED.addLeds<WS2812B, Config::RING_LED_PIN, GRB>(ringLeds, Config::RING_NUM_LEDS);
  FastLED.setBrightness(0);
  FastLED.clear();
  stableShow();

  // 设置服务器路由
  server.on("/", [this]()
            { this->handleRoot(); });
  server.on("/control", [this]()
            { this->handleControl(); });
  server.onNotFound([this]()
                    { this->handleNotFound(); });
  server.begin();
}

void LEDController::stableShow()
{
  delayMicroseconds(50);
  FastLED.show();
  delayMicroseconds(50);
}

bool LEDController::fadeOut()
{
  static uint32_t startTime = 0;
  static uint8_t startBrightness = 0;

  if (startTime == 0)
  {
    startTime = millis();
    startBrightness = FastLED.getBrightness();
  }

  uint32_t elapsedTime = millis() - startTime;
  if (elapsedTime >= Config::FADE_OUT_MS)
  {
    FastLED.setBrightness(0);
    stableShow();
    startTime = 0;
    return true;
  }

  uint8_t brightness = startBrightness * (Config::FADE_OUT_MS - elapsedTime) / Config::FADE_OUT_MS;
  FastLED.setBrightness(brightness);
  stableShow();
  return false;
}

bool LEDController::fadeIn()
{
  static uint32_t startTime = 0;

  if (startTime == 0)
  {
    startTime = millis();
    startHue = 0;
    fill_rainbow(mainLeds, Config::MAIN_NUM_LEDS, startHue, 255 / Config::MAIN_NUM_LEDS);
    fill_rainbow(ringLeds, Config::RING_NUM_LEDS, startHue + 64, 255 / Config::RING_NUM_LEDS);
    ringHue = startHue;
  }

  uint32_t elapsedTime = millis() - startTime;
  if (elapsedTime >= Config::FADE_IN_MS)
  {
    FastLED.setBrightness(LEDController::targetBrightness);
    stableShow();
    startTime = 0;
    return true;
  }

  uint8_t brightness = LEDController::targetBrightness * elapsedTime / Config::FADE_IN_MS;
  FastLED.setBrightness(brightness);
  fill_rainbow(mainLeds, Config::MAIN_NUM_LEDS, startHue, 255 / Config::MAIN_NUM_LEDS);
  fill_rainbow(ringLeds, Config::RING_NUM_LEDS, startHue + 64, 255 / Config::RING_NUM_LEDS);
  stableShow();
  return false;
}

void LEDController::setManualColor(uint8_t r, uint8_t g, uint8_t b)
{
  manualRed = r;
  manualGreen = g;
  manualBlue = b;

  if (currentState == STATE_MANUAL)
  {
    fill_solid(mainLeds, Config::MAIN_NUM_LEDS, CRGB(r, g, b));
    fill_solid(ringLeds, Config::RING_NUM_LEDS, CRGB(r, g, b));
    FastLED.setBrightness(globalBrightness);
    stableShow();
  }
}

void LEDController::setBrightness(uint8_t brightness)
{
  globalBrightness = brightness;
  targetBrightness = brightness;
  FastLED.setBrightness(brightness);
  stableShow();
  Serial.println("亮度已设置为: " + String(brightness) + "%");
}

void LEDController::setMode(const String &mode)
{
  Serial.println("设置模式: " + mode);

  if (mode == "off")
  {
    lastState = currentState;
    currentState = STATE_FADE_OUT;
    FastLED.setBrightness(0);
    stableShow();
  }
  else if (mode == "breathe")
  {
    lastState = currentState;
    currentState = STATE_BREATHE_LOOP;
    breatheStep = 0;
  }
  else if (mode == "rainbow")
  {
    lastState = currentState;
    currentState = STATE_FADE_IN;
    startHue = 0;
    ringHue = 0;
  }
  else if (mode == "manual")
  {
    lastState = currentState;
    currentState = STATE_MANUAL;
    setManualColor(manualRed, manualGreen, manualBlue);
  }
  else if (mode == "auto")
  {

    motionsensor.CheckMotion(1);//Force check
    // currentMotionState = digitalRead(Config::MOTION_SENSOR_PIN);
    // if (currentMotionState) {
    //   currentState = STATE_BREATHE;
    //   breatheStep = 0;
  }
}

void LEDController::setState(SystemState NewState) {
    lastState = currentState;
    currentState = NewState;
}

SystemState LEDController::getState() const {
    return currentState;
}

void LEDController::setBreathStep(uint16_t NewBreathStep) {
    breatheStep = NewBreathStep;
}

void LEDController::setStartHue(uint8_t hue) {
    startHue = hue;
    ringHue = hue;
}

void LEDController::quickTestLeds()
{
  Serial.println("快速测试灯环...");

  fill_solid(mainLeds, Config::MAIN_NUM_LEDS, CRGB::Blue);
  fill_solid(ringLeds, Config::RING_NUM_LEDS, CRGB::Blue);
  FastLED.setBrightness(50);
  stableShow();
  delay(500);

  fill_solid(mainLeds, Config::MAIN_NUM_LEDS, CRGB::Green);
  fill_solid(ringLeds, Config::RING_NUM_LEDS, CRGB::Green);
  stableShow();
  delay(500);

  fill_solid(mainLeds, Config::MAIN_NUM_LEDS, CRGB::Red);
  fill_solid(ringLeds, Config::RING_NUM_LEDS, CRGB::Red);
  stableShow();
  delay(500);

  FastLED.setBrightness(0);
  FastLED.clear();
  stableShow();

  Serial.println("灯环测试完成");
}

void LEDController::handleRoot()
{
  Serial.println("收到网页请求");

  String html = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset='UTF-8'>
  <style>
    body { 
      font-family: Arial; 
      text-align: center; 
      margin: 0 auto; 
      padding: 20px;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
    }
    .container { 
      max-width: 400px; 
      margin: 0 auto; 
      background: rgba(255,255,255,0.1);
      padding: 20px;
      border-radius: 15px;
      backdrop-filter: blur(10px);
    }
    .btn { 
      background-color: #4CAF50; 
      border: none; 
      color: white; 
      padding: 12px 24px; 
      text-align: center; 
      text-decoration: none; 
      display: inline-block; 
      font-size: 16px; 
      margin: 4px 2px; 
      cursor: pointer; 
      border-radius: 8px;
      width: 100%;
    }
    .btn-off { background-color: #f44336; }
    .btn-auto { background-color: #2196F3; }
    .slider-container { 
      margin: 20px 0; 
      text-align: left;
    }
    .slider { 
      width: 100%; 
      height: 25px; 
      background: #ddd;
      outline: none;
      border-radius: 12px;
    }
    .color-picker {
      width: 100%;
      height: 50px;
      border: none;
      border-radius: 8px;
      margin: 10px 0;
    }
    .status {
      background: rgba(0,0,0,0.3);
      padding: 10px;
      border-radius: 8px;
      margin: 10px 0;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>💡 LED灯光控制</h1>
    
    <div class="status">
      <p>IP地址: )rawliteral" +
                WiFi.localIP().toString() + R"rawliteral(</p>
      <p>状态: <span id="status">)rawliteral" +
                (currentState == STATE_OFF ? "关闭" : currentState == STATE_BREATHE ? "呼吸模式"
                                                  : currentState == STATE_NORMAL    ? "彩虹模式"
                                                  : currentState == STATE_MANUAL    ? "手动调色"
                                                                                    : "自动模式") +
                R"rawliteral(</span></p>
    </div>

    <h3>模式选择</h3>
    <button class="btn btn-off" onclick="setMode('off')">关闭</button>
    <button class="btn" onclick="setMode('breathe')">呼吸模式</button>
    <button class="btn" onclick="setMode('rainbow')">彩虹模式</button>
    <button class="btn" onclick="setMode('manual')">手动调色</button>
    <button class="btn btn-auto" onclick="setMode('auto')">自动模式</button>

    <div class="slider-container">
      <h3>亮度控制: <span id="brightnessValue">)rawliteral" +
                String(map(globalBrightness, 0, 255, 0, 100)) + R"rawliteral(</span>%</h3>
      <input type="range" min="0" max="100" value=")rawliteral" +
                String(map(globalBrightness, 0, 255, 0, 100)) + R"rawliteral(" class="slider" id="brightnessSlider" onchange="setBrightness(this.value)">
    </div>

    <div id="colorControl" style="display: )rawliteral" +
                (currentState == STATE_MANUAL ? "block" : "none") + R"rawliteral(;">
      <h3>颜色选择</h3>
      <input type="color" class="color-picker" id="colorPicker" onchange="setColor(this.value)" value="#ffffff">
    </div>
  </div>

  <script>
    function setMode(mode) {
      fetch('/control?mode=' + mode)
        .then(response => response.json())
        .then(data => {
          document.getElementById('status').innerText = data.status;
          document.getElementById('colorControl').style.display = 
            (mode === 'manual') ? 'block' : 'none';
        });
    }

    function setBrightness(value) {
      document.getElementById('brightnessValue').innerText = value;
      fetch('/control?brightness=' + value)
        .then(response => response.json());
    }

    function setColor(color) {
      const r = parseInt(color.substr(1,2), 16);
      const g = parseInt(color.substr(3,2), 16);
      const b = parseInt(color.substr(5,2), 16);
      fetch('/control?r=' + r + '&g=' + g + '&b=' + b)
        .then(response => response.json());
    }
  </script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}

void LEDController::handleClient()
{
  server.handleClient();
}

void LEDController::handleControl()
{
  Serial.println("收到控制请求");

  String message = "";

  if (server.hasArg("mode"))
  {
    String mode = server.arg("mode");
    setMode(mode);
    message = "模式已设置为: " + mode;
  }

  if (server.hasArg("brightness"))
  {
    int brightness = server.arg("brightness").toInt();
    setBrightness(map(brightness, 0, 100, 0, 255));
  }

  if (server.hasArg("r") && server.hasArg("g") && server.hasArg("b"))
  {
    uint8_t r = server.arg("r").toInt();
    uint8_t g = server.arg("g").toInt();
    uint8_t b = server.arg("b").toInt();
    setManualColor(r, g, b);
    message += " 颜色已设置";
  }

  String statusText = "";
  switch (currentState)
  {
  case STATE_OFF:
    statusText = "关闭";
    break;
  case STATE_BREATHE:
    statusText = "呼吸模式";
    break;
  case STATE_NORMAL:
    statusText = "彩虹模式";
    break;
  case STATE_MANUAL:
    statusText = "手动调色";
    break;
  default:
    statusText = "自动模式";
    break;
  }

  String json = "{\"status\":\"" + statusText + "\",\"message\":\"" + message + "\",\"brightness\":" + String(map(globalBrightness, 0, 255, 0, 100)) + "}";
  server.send(200, "application/json", json);

  Serial.println("控制响应: " + message);
}

void LEDController::handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void LEDController::update()
{
  // 状态机处理
  switch (currentState)
  {
  case STATE_OFF:
    if (lastState != STATE_OFF)
    {
      if (fadeOut())
      {
        lastState = currentState;
        breatheStep = 0;
      }
    }
    break;

  case STATE_BREATHE:
  case STATE_AUTO_BREATH:
    if (breatheStep < Config::BREATHE_STEPS)
    {
      Serial.println("in this loop once");
      FastLED.setBrightness(255);
      FastLED.clear();

      uint8_t mainPos = (breatheStep < Config::MAIN_NUM_LEDS) ? breatheStep : (2 * Config::MAIN_NUM_LEDS - 1 - breatheStep);
      uint8_t mainBrightness;
      if (breatheStep < Config::MAIN_NUM_LEDS)
      {
        mainBrightness = (uint16_t)breatheStep * 255 / Config::MAIN_NUM_LEDS;
      }
      else
      {
        mainBrightness = (uint16_t)(Config::BREATHE_STEPS - breatheStep) * 255 / Config::MAIN_NUM_LEDS;
      }
      mainLeds[mainPos] = CRGB(mainBrightness, mainBrightness, mainBrightness);

      uint8_t ringPos = (breatheStep * Config::RING_NUM_LEDS) / Config::BREATHE_STEPS;
      ringPos = ringPos % Config::RING_NUM_LEDS;
      fill_solid(ringLeds, Config::RING_NUM_LEDS, CRGB::Black);
      ringLeds[ringPos] = CRGB(mainBrightness, mainBrightness, mainBrightness);

      stableShow();
      delay(Config::BREATHE_DURATION_MS / Config::BREATHE_STEPS);
      breatheStep++;
    }
    else
    {
      if (currentState == STATE_BREATHE)
      {
        //lastState = currentState;
        //currentState = STATE_FADE_IN;
        startHue = 0;
        breatheStep = 0;
      }
      else if (currentState == STATE_AUTO_BREATH)
      {
        lastState = currentState;
        currentState = STATE_AUTO_FADE_IN;
        startHue = 0;
        breatheStep = 0;
      }
    }
    break;

  // case STATE_BREATHE_LOOP:
  // {
  //   FastLED.clear();

  //   uint16_t currentStep = breatheStep % Config::BREATHE_STEPS;
  //   uint8_t mainPos = currentStep % Config::MAIN_NUM_LEDS;
  //   uint8_t ringPos = currentStep % Config::RING_NUM_LEDS;

  //   mainLeds[mainPos] = CRGB::White;
  //   fill_solid(ringLeds, Config::RING_NUM_LEDS, CRGB::Black);
  //   ringLeds[ringPos] = CRGB::White;

  //   stableShow();
  //   delay(Config::BREATHE_DURATION_MS / Config::BREATHE_STEPS);
  //   breatheStep++;
  //   break;
  // }

  case STATE_FADE_IN:
  case STATE_AUTO_FADE_IN:
    if (fadeIn())
    {
      if (currentState == STATE_FADE_IN)
      {
        lastState = currentState;
        currentState = STATE_NORMAL;
        // lastModeChangeTime = millis();
        ringHue = startHue;
      }
      else if (currentState == STATE_AUTO_FADE_IN)
      {
        lastState = currentState;
        currentState = STATE_AUTO_NORMAL;
        // currentState = STATE_NORMAL;
        // lastModeChangeTime = millis();
        ringHue = startHue;
      }
    }
    break;

  case STATE_NORMAL:
  case STATE_AUTO_NORMAL:
  {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= Config::NORMAL_UPDATE_INTERVAL)
    {
      previousMillis = currentMillis;
      fill_rainbow(mainLeds, Config::MAIN_NUM_LEDS, ringHue, 255 / Config::MAIN_NUM_LEDS);
      fill_rainbow(ringLeds, Config::RING_NUM_LEDS, ringHue + 64, 255 / Config::RING_NUM_LEDS);
      ringHue += 2;
      stableShow();
    }
  }
  break;

  case STATE_FADE_OUT:
  case STATE_AUTO_FADE_OUT:
    if (fadeOut())
    {
      
      if (currentState == STATE_FADE_OUT){
        lastState = currentState;
        currentState = STATE_OFF;
        breatheStep = 0;
      }
      else if(currentState == STATE_AUTO_FADE_OUT){
        lastState = currentState;
        currentState = STATE_AUTO_OFF;
        breatheStep = 0;
      }
    }
    
    break;

  case STATE_MANUAL:
    break;
  }
}