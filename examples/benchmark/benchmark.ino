#include <Arduino.h>
#include "Log.h"

// LOG_BENCHMARK: automatically logs elapsed time when the function returns
void sensorRead()
{
    LOG_BENCHMARK();
    delay(150); // Simulate sensor work
    LOG_INFO("Sensor value: {}", analogRead(5));
}

// LOG_BENCHMARK_BEGIN/END: manually mark sections to time
// Labels are symbols, multiple pairs per scope are supported
void processData()
{
    LOG_INFO("Starting data processing");

    LOG_BENCHMARK_BEGIN(parse);
    delay(50); // Simulate parsing
    LOG_BENCHMARK_END(parse);

    LOG_BENCHMARK_BEGIN(transform);
    delay(75); // Simulate transform
    LOG_BENCHMARK_END(transform);

    LOG_INFO("Processing complete");
}

// LOG_BENCHMARK: works with early returns — always logs on scope exit
bool connectWifi()
{
    LOG_BENCHMARK();

    delay(50); // Simulate connection attempt
    bool connected = false;

    if (!connected)
    {
        LOG_WARN("Connection failed");
        return false; // Benchmark still logs elapsed time
    }

    return true;
}

// LOG_BENCHMARK_BEGIN/END: timing a loop
void sendBatch()
{
    LOG_BENCHMARK_BEGIN(batch);
    for (int i = 0; i < 5; i++)
    {
        delay(20); // Simulate sending
    }
    LOG_BENCHMARK_END(batch);
}

// LOG_STOPWATCH: manual stopwatch for custom use
void calibrate()
{
    auto sw = LOG_CREATE_STOPWATCH();

    delay(100); // Step 1
    LOG_INFO("Step 1 done at {} ms", sw.elapsedMs());

    delay(200); // Step 2
    LOG_INFO("Step 2 done at {} ms", sw.elapsedMs());

    sw.reset();
    delay(50); // Step 3
    LOG_INFO("Step 3 after reset: {}", sw.elapsedTime());
}

// LOG_BENCHMARK_MICRO_BEGIN/END: microsecond precision sections
void fastOperation()
{
    LOG_BENCHMARK_MICRO_BEGIN(compute);
    delayMicroseconds(200); // Simulate fast computation
    LOG_BENCHMARK_MICRO_END(compute);

    LOG_BENCHMARK_MICRO_BEGIN(copy);
    delayMicroseconds(100); // Simulate memory copy
    LOG_BENCHMARK_MICRO_END(copy);
}

// MicroStopwatch: microsecond precision timing
void precisionTiming()
{
    fmtlog::MicroStopwatch usw;

    delayMicroseconds(500);
    LOG_INFO("Elapsed: {} us", usw.elapsedUs());

    delay(10);
    LOG_INFO("Elapsed: {}", usw.elapsedTime()); // HH:MM:SS:mmm:uuu
}

void setup()
{
    LOG_BEGIN(115200);
    delay(3000);

    LOG_INFO("=== Benchmark Examples ===");

    sensorRead();
    processData();
    connectWifi();
    sendBatch();
    calibrate();
    fastOperation();
    precisionTiming();
}

void loop()
{
}
