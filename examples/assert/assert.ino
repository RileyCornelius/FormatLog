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

int divide(int a, int b)
{
    // Logs a warning and returns -1 if the condition fails
    CHECK_OR_RETURN_VALUE(b != 0, -1, "Division by zero");
    return a / b;
}

void connectToSensor(int pin)
{
    // Logs a warning and returns if the condition fails
    CHECK_OR_RETURN(pin >= 0, "Invalid pin number");
    LOG_INFO("Connected to sensor on pin {}", pin);
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

    // CHECK_OR_RETURN_VALUE: logs warning and returns -1 on failure
    int result = divide(10, 0);
    LOG_INFO("divide(10, 0) = {}", result);

    // CHECK_OR_RETURN: logs warning and returns void on failure
    connectToSensor(-1);

    // ASSERT: logs error + calls panic handler on failure (halts by default)
    ASSERT(1 + 1 == 3, "Math is broken!"); // LOG_ASSERT_ENABLE 0 - to disable assertion
}

void loop()
{
}