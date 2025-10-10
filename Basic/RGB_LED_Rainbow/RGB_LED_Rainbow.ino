// 在 ESP32-S3 上测试

#include <Adafruit_NeoPixel.h>  // 引用库

// 定义 LED 连接的引脚，我这个是 GPIO 48
#define LED_PIN 48

// 定义 LED 的数量（板载通常只有 1 个）
#define LED_COUNT 1

// 创建一个 NeoPixel 对象
// 参数1: LED 数量
// 参数2: Arduino 引脚号
// 参数3: 像素类型和速度标志。对于 ESP32-S3 上的 WS2812，NEO_GRB + NEO_KHZ800 是最常见的组合
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

uint8_t r = 255, g = 0, b = 0;  //我选择了全局变量，感觉在 loop 里 uint8_t 怪怪的？

void setup() {
  strip.begin();            // 初始化 NeoPixel 库
  strip.show();             // 更新 LED 状态（初始为关闭）
  strip.setBrightness(50);  // 设置亮度 (0-255)，建议不要太高，以免刺眼
}

void loop() {
  while (r > 0) {  // 红 -> 红绿 -> 绿
    r--;
    g = 255 - r;
    strip.setPixelColor(0, strip.Color(r, g, b));  // 设置颜色
    strip.show();                                  // 居然要额外播放一下，这个操作有点迷
    delay(10);                                     //稍作等待，不然颜色变化太快了
  }
  while (g > 0) {  // 绿 -> 绿蓝 -> 蓝
    g--;
    b = 255 - g;
    strip.setPixelColor(0, strip.Color(r, g, b));
    strip.show();
    delay(10);
  }
  while (b > 0) {  // 蓝 -> 蓝红 -> 红
    b--;
    r = 255 - b;
    strip.setPixelColor(0, strip.Color(r, g, b));
    strip.show();
    delay(10);
  }
}
