#include <Arduino.h>
#include <string>

#include "Log.h"
// #include "FormatLog.h"

void setup()
{
  Serial.begin(115200);
  LOG_INFO("C++ version: {}", __cplusplus / 100);

  // std::string buffer;
  // buffer = fmt::format("Hello, world! {} {:.4f} {}", 1, 3.145435, "gds");
  // buffer = fmt::format("Hello, world! {}", std::array{1, 2, 3, 4, 5});
  // Serial.println(buffer.c_str());

  LOG_TRACE("Trace message with value: {}", 1);
  LOG_DEBUG("Debug message with value: {}", 42);
  LOG_INFO("Info message with value: {}", 3.14);
  LOG_WARN("Warning message with value: {}", "test");
  LOG_ERROR("Error message with value: {}", std::array{1, 2, 3, 4, 5});
  // LOG_INFO("{} {} {} {}", 1, 2.3, "test", 4.5);
  // LOG_INFO(3);

  // constexpr const char *msg = "Hello, {} {} {} {}";
  // LOG_INFO(msg, 1, 2.3, "test", 4.5);
  // LOG_INFO("Hello, {} {} {} {}", 2, 2.3, "test", 4.5);
  // LOG_DEBUG("msg");
  // LOG_TRACE(3);
  // LOG_INFO(std::string("Hello, {} {} {} {}"), 3, 2.3, "test", 4.5);
  // LOG_INFO(String("Hello, {} {} {} {}"), 4, 2.3, "test", 4.5);
  // log(5);
}

void loop()
{
  // loggerLoop();
  delay(1000);
}