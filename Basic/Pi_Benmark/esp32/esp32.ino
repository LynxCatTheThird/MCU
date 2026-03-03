#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

// 启用 GCC 最高级别优化 (-Ofast)
// 允许编译器进行激进的数学运算优化（如自动向量化、忽略严格的 IEEE 754 合规性）以极大提升浮点性能
// #pragma GCC optimize("Ofast")

const unsigned long ITERATIONS = 100000000UL; // 总计算迭代次数 (1亿次)
unsigned long total_rounds = 0;               // 累计运行轮数

// ============== 双核共享变量 ==============
// volatile 必不可少：指示编译器每次都从内存读取该值，防止被优化到寄存器中导致死循环
volatile bool core1_done = false;
float pi_core1 = 0.0f; // 存储 Core 1 的计算结果

// ============== Core 1 任务 (后台计算) ==============
// FreeRTOS 任务函数签名必须为 void func(void* param)
void core1_task(void* param) {
  float pi = 0.0f;
  
  // 数学逻辑：接力计算后 50% 的区间
  // 确定迭代中点的初始符号和分母
  float sign = (ITERATIONS / 2) % 2 == 0 ? 1.0f : -1.0f;
  float denom = 1.0f + (ITERATIONS / 2) * 2.0f;

  for (unsigned long i = 0; i < ITERATIONS / 2; i++) {
    pi += sign * (4.0f / denom);
    sign = -sign;
    denom += 2.0f;
  }

  pi_core1 = pi;
  
  // 在设置标志位前，通常不需要内存屏障指令，
  // 因为 ESP32 的 FreeRTOS 调度和 volatile 机制通常足以保证可见性，
  // 但在极高频访问下需注意写入顺序。此处逻辑简单，直接赋值即可。
  core1_done = true;
  
  // 任务自杀：计算完成后必须删除自身，释放任务栈内存
  vTaskDelete(NULL);
}

// ============== 硬件监测辅助函数 ==============
// 读取芯片内置温度传感器 (ESP32-P4 特性)
float read_internal_temp() {
  return temperatureRead();
}

// 通过读取 eFuse MAC 地址生成唯一芯片 ID
String get_chip_id() {
  uint64_t mac = ESP.getEfuseMac();
  String out = "";
  for (int i = 5; i >= 0; i--) {
    uint8_t b = (mac >> (i * 8)) & 0xFF;
    if (b < 16) out += "0";
    out += String(b, HEX);
  }
  out.toUpperCase();
  return out;
}

// ============== 系统初始化 ==============
void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10); // 等待 USB CDC 就绪
  delay(1000);

  // 打印详细的系统概况
  Serial.println(R"(
==============================================
        MCU Benchmark - EPS Calculation
==============================================)");

  // ESP32-P4 是基于 RISC-V 核心的高性能 MCU
  Serial.printf("Board:         %s\n", "ESP32-P4 DevKit");
  Serial.printf("Chip:          %s\n", ESP.getChipModel());
  Serial.printf("Architecture:  %s\n", "RISC-V HP (Dual Core)");
  Serial.printf("Cores:         %d\n", ESP.getChipCores());
  Serial.printf("CPU Freq:      %lu MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Chip UID:      %s\n", get_chip_id().c_str());
  Serial.printf("Flash Size:    %lu MB\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf("PSRAM Size:    %lu MB\n", ESP.getPsramSize() / (1024 * 1024));
  Serial.printf("Core Temp:     %.2f C (Idle)\n", temperatureRead());

  Serial.println(R"(----------------------------------------------
Mode:   Dual-Core Range Split (Safe Mode)
Opt:    -Ofast + Hardware FPU
Target: 100,000,000 Iterations
----------------------------------------------)");
}

// ============== 双核基准测试主逻辑 ==============
// IRAM_ATTR: 强制将函数加载到指令 RAM (SRAM) 中运行，而不是 Flash。
// 这对于基准测试至关重要，避免了 Flash Cache Miss 造成的随机延迟。
void IRAM_ATTR run_benchmark() {
  // 重置同步信号
  core1_done = false;
  pi_core1 = 0.0f;

  // 使用 FreeRTOS API 将任务“钉”在 Core 1 上运行
  xTaskCreatePinnedToCore(
    core1_task,                // 任务函数
    "Core1Task",               // 任务名称（调试用）
    4096,                      // 栈大小 (Words/Bytes 取决于具体移植，ESP32通常是Byte)
    NULL,                      // 任务参数
    configMAX_PRIORITIES - 1,  // 优先级：设为极高，防止被系统空闲任务抢占
    NULL,                      // 任务句柄
    1                          // 核心索引：1 代表 Core 1 (App Core)
  );

  unsigned long start = millis();

  // Core 0 (Protocol Core) 任务：计算前 50%
  float pi = 0.0f;
  float sign = 1.0f;
  float denom = 1.0f;
  for (unsigned long i = 0; i < ITERATIONS / 2; i++) {
    pi += sign * (4.0f / denom);
    sign = -sign;
    denom += 2.0f;
  }

  // 忙等待 (Spinlock style wait)
  // 这里故意不使用 vTaskDelay()。
  // vTaskDelay 会导致上下文切换（Yield），这对毫秒级计时的基准测试引入了调度开销。
  // 我们希望 Core 0 保持全速运转，直到看到 Core 1 修改变量。
  while (!core1_done) {
    // 空循环，占用 CPU 周期等待内存标志位变更
  }

  // 合并结果
  pi += pi_core1;

  unsigned long duration = millis() - start;
  
  // 统计与日志输出
  total_rounds++;
  unsigned long uptime = millis();
  unsigned long h = uptime / 3600000;
  unsigned long m = (uptime % 3600000) / 60000;
  unsigned long s = (uptime % 60000) / 1000;
  float current_temp = temperatureRead();
  
  Serial.printf("[%02lu:%02lu:%02lu] Round:%-6lu | Time:%-5lu ms | Temp:%-4.1f C | Pi:%.7f\n",
                h, m, s,
                total_rounds,
                duration,
                current_temp,
                pi);
                
  // 注意：Core 1 的任务已在内部通过 vTaskDelete 删除，无需在此处额外处理
}

void loop() {
  run_benchmark();
}
