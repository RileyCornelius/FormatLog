#include <Arduino.h>
#include "FmtLog.h"

void setup()
{
    LOG_BEGIN(115200);
    LOG_INFO("FormatLog basic example started");
}

void loop()
{
    static uint32_t iteration = 0;

    LOG_TRACE("Trace message with value: {}", iteration);
    LOG_DEBUG("Debug message with value: {}", 42);
    LOG_INFO("Info message with value: {}", 3.14);
    LOG_WARN("Warning message with value: v{}.{}.{}", 1, 12, 5);
    LOG_ERROR("Error message with String: {}", String("critical"));

    iteration++;
    delay(5000);
}
