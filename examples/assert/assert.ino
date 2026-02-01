#include <Arduino.h>
#include "Log.h"

void panic()
{
    LOG_PRINT("Panic! System halted.");

    const int ledPin = LED_BUILTIN;
    pinMode(ledPin, OUTPUT);
    while (true)
    {
        LOG_PRINT(".");
        digitalWrite(ledPin, HIGH);
        delay(1000);
        digitalWrite(ledPin, LOW);
        delay(1000);
    }
}

void setup()
{
    LOG_BEGIN(115200);
    LOG_SET_PANIC_HANDLER(panic); // Set custom panic handler at runtime

    delay(3000); // Wait for serial to initialize

    LOG_TRACE("Trace message with value: {}", 1);
    LOG_DEBUG("Debug message with value: {}", 42);
    LOG_INFO("Info message with value: {}", 3.14);
    LOG_WARN("Warning message with value: {}", "char *");
    LOG_ERROR("Error message with String: {}", String("String"));

    ASSERT(1 + 1 == 3, "Math is broken!"); // LOG_ASSERT_ENABLE 0 - to disable assertion
}

void loop()
{
}