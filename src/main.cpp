#include <Arduino.h>
#include <string>

#include "Log.h"
// #include "FormatLog.h"

void setup()
{
  Serial.begin(115200);
  LOG_INFO("C++ version: {}", __cplusplus / 100);

  // std::string buffer;
  // buffer = fmt::format("Hello, world! {}", std::array{1, 2, 3, 4, 5});
  // Serial.println(buffer.c_str());

  LOG_TRACE("Trace message with value: {}", 1);
  LOG_DEBUG("Debug message with value: {}", 42);
  LOG_INFO("Info message with value: {}", 3.14);
  LOG_WARN("Warning message with value: {}", "test");
  LOG_ERROR("Error message with value: {}", std::array{1, 2, 3, 4, 5});
  LOG_ERROR("Error message with String: {}", String("test string"));
  LOG_INFO(3);
}

void loop()
{
  delay(1000);
}