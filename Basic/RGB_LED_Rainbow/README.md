## RGB LED 可以变成彩虹 🌈

### 兼容的开发板

理论上所有拥有 RGB LED 的开发板，使用 ESP32-S3 自带的 LED 测试通过。

### 元器件

需要一个 RGB LED，直接连接到开发板的对应引脚即可。

### 介绍

单引脚的版本哦。

### 补充说明

截至本项目完工的时间（2025 年 10 月 10 日），依赖库 Adafruit_NeoPixel 版本 1.15.1 的 esp32 实现存在问题：

``` cpp
/home/lynx/Program/MCU/libraries/Adafruit_NeoPixel/esp.c: In function 'espShow':
/home/lynx/Program/MCU/libraries/Adafruit_NeoPixel/esp.c:56:11: warning: suggest parentheses around assignment used as truth value [-Wparentheses]
   56 |       if (led_data = (rmt_data_t *)malloc(requiredSize * sizeof(rmt_data_t))) {
      |           ^~~~~~~~
```
该问题[已经修复](https://github.com/adafruit/Adafruit_NeoPixel/commit/16284db0c4fdfaf0e3ccae14dceb220a48f7a614)但未发版，请注意手动更新。
