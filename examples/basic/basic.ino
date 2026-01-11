#include <Arduino.h>
#include "FmtLog.h"

void setup()
{
    LOG_BEGIN(115200);
}

void loop()
{
    LOG_TRACE("Trace message with value: {}", 1);
    LOG_DEBUG("Debug message with value: {}", 42);
    LOG_INFO("Info message with value: {}", 3.14);
    LOG_WARN("Warning message with value: {}", "char *");
    LOG_ERROR("Error message with String: {}", String("String"));

    ASSERT_M(1 + 1 == 3, "Math is broken!"); // Set LOG_ASSERT_ENABLE to 0 to disable assertions
    delay(5000);
}