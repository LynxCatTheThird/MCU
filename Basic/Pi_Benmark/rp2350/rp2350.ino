#include <Arduino.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"
#include "hardware/clocks.h"

// 启用 GCC 最高级别优化 (-Ofast)
// 允许编译器进行激进的数学运算优化（如自动向量化、忽略严格的 IEEE 754 合规性）以极大提升浮点性能
// #pragma GCC optimize("Ofast")

const unsigned long ITERATIONS = 100000000UL; // 总迭代次数 (1亿次)
unsigned long total_rounds = 0;               // 累计运行轮数

// ============== 双核共享变量 ==============
// 使用 volatile 关键字防止编译器将变量缓存于寄存器
// 确保两个核心都能读取到最新的内存状态
volatile bool core1_done = false;
volatile float pi_core1 = 0.0f;

// ============== Core 1 任务 (后台计算) ==============
// 负责计算迭代区间的后 50%
void __not_in_flash_func(core1_task)() {
  float pi = 0.0f;
  
  // 确定后半程起点的数学状态
  // 计算第 (ITERATIONS / 2) 次迭代时的符号位
  float sign = (ITERATIONS / 2) % 2 == 0 ? 1.0f : -1.0f;
  // 计算第 (ITERATIONS / 2) 次迭代时的分母
  float denom = 1.0f + (ITERATIONS / 2) * 2.0f;

  for (unsigned long i = 0; i < ITERATIONS / 2; i++) {
    pi += sign * (4.0f / denom);
    sign = -sign;
    denom += 2.0f;
  }

  pi_core1 = pi;
  
  // 内存屏障 (Memory Barrier)
  // 强制 CPU 等待前面的写入指令完成，确保 pi_core1 在 core1_done 置位前已写入 RAM
  __sync_synchronize();
  core1_done = true;
}

// ============== 硬件监测与辅助函数 ==============
// 读取 RP2350 内部温度传感器
float read_internal_temp() {
  adc_init();
  adc_set_temp_sensor_enabled(true);
  adc_select_input(4); // RP2040/RP2350 内部温度传感器固定连接到 ADC 通道 4
  uint16_t raw = adc_read();
  const float conversion_factor = 3.3f / (1 << 12); // 12-bit ADC -> 3.3V
  float voltage = raw * conversion_factor;
  // 计算公式参考 RP2350 数据手册
  float temp = 27.0f - (voltage - 0.706f) / 0.001721f;
  return temp;
}

// 获取芯片唯一 ID (Flash UUID)
String get_chip_id() {
  pico_unique_board_id_t id;
  pico_get_unique_board_id(&id);
  String out = "";
  for (int i = 0; i < 8; i++) {
    if (id.id[i] < 16) out += "0";
    out += String(id.id[i], HEX);
  }
  out.toUpperCase();
  return out;
}

// ============== 系统初始化 ==============
void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10); // 等待 USB 串口连接
  delay(1000);

  // 打印系统信息与测试参数
  Serial.println(R"(
==============================================
         MCU Benchmark - Pi Calculation
==============================================)");
  Serial.printf("Board:         %s\n", "Raspberry Pi Pico 2");
  Serial.printf("Chip:          %s\n", "RP2350");
  Serial.printf("Architecture:  %s\n", "ARM Cortex-M33 (Dual Core)");
  Serial.printf("CPU Freq:      %lu MHz\n", clock_get_hz(clk_sys) / 1000000);
  Serial.printf("Core Temp:     %.2f C (Idle)\n", read_internal_temp());
  Serial.println(R"(----------------------------------------------
Mode:   Dual-Core Range Split (Safe Mode)
Opt:    -Ofast + Hardware FPU
Target: 100,000,000 Iterations
----------------------------------------------)");
}

// ============== 双核基准测试主逻辑 ==============
// __not_in_flash_func: 将函数加载到 SRAM 中运行
// 避免 Flash 读取等待状态 (Wait States) 影响基准测试的准确性
void __not_in_flash_func(run_benchmark)() {
  // 重置 Core 1 状态标志
  core1_done = false;
  pi_core1 = 0.0f;

  // 将任务分派给 Core 1 并在后台启动
  multicore_launch_core1(core1_task);

  unsigned long start_time = millis();

  // Core 0 (主核) 任务：计算前 50%
  float pi = 0.0f;
  float sign = 1.0f;
  float denom = 1.0f;
  for (unsigned long i = 0; i < ITERATIONS / 2; i++) {
    pi += sign * (4.0f / denom);
    sign = -sign;
    denom += 2.0f;
  }

  // 阻塞等待 Core 1 计算完成
  // tight_loop_contents() 是编译器提示，优化忙等待循环的指令生成
  while (!core1_done) {
    tight_loop_contents();
  }

  // 汇总两个核心的计算结果
  pi += pi_core1;

  unsigned long duration = millis() - start_time;

  // 统计与格式化逻辑
  total_rounds++;
  unsigned long uptime = millis();
  unsigned long h = uptime / 3600000;
  unsigned long m = (uptime % 3600000) / 60000;
  unsigned long s = (uptime % 60000) / 1000;
  float current_temp = read_internal_temp();

  // 打印单轮测试结果 (包含 Pi 值以验证正确性)
  Serial.printf("[%02lu:%02lu:%02lu] Round:%-6lu | Time:%-5lu ms | Temp:%-4.1f C | Pi:%.7f\n",
                h, m, s,
                total_rounds,
                duration,
                current_temp,
                pi);

  // 复位 Core 1 堆栈，为下一轮 multicore_launch_core1 做好准备
  // 必须复位，否则再次 launch 会因为 Core 1 仍在运行完的状态而失败
  multicore_reset_core1();
}

void loop() {
  run_benchmark();
}
