#include <Arduino.h>
#include <string>

#include "Log.h"

// #include "FormatLog.h"

void setup()
{
  Serial.begin(115200);
  // delay(3000);
  // LOG_INFO("C++ standard version: {}", __cplusplus / 100);

  std::string buffer;
  // buffer = fmt::format("Hello, world! {} {:.4f} {}", 1, 3.145435, "gds");
  buffer = fmt::format("Hello, world! {}", std::array{1, 2, 3, 4, 5});
  Serial.println(buffer.c_str());

  LOG_INFO("{} {} {} {}", 1, 2.3, "test", 4.5);

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