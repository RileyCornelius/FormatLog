#pragma once

#include "Benchmark/Stopwatch.h"

namespace fmtlog
{

    class Benchmark
    {
    private:
        const char *_label;
        Stopwatch _sw;

    public:
        Benchmark(const char *label) : _label(label) {}

        void reset() { _sw.reset(); }

        const char *label() const { return _label; }

        uint32_t elapsedMs() const { return _sw.elapsedMs(); }
    };

    class MicroBenchmark
    {
    private:
        const char *_label;
        MicroStopwatch _sw;

    public:
        MicroBenchmark(const char *label) : _label(label) {}

        void reset() { _sw.reset(); }

        const char *label() const { return _label; }

        uint32_t elapsedUs() const { return _sw.elapsedUs(); }
    };

    class ScopedBenchmark
    {
    public:
        using Callback = void (*)(const char *label, uint32_t elapsedMs);

    private:
        Benchmark _bench;
        Callback _callback;

    public:
        ScopedBenchmark(const char *label, Callback callback) : _bench(label), _callback(callback) {}

        ~ScopedBenchmark()
        {
            if (_callback)
                _callback(_bench.label(), _bench.elapsedMs());
        }
    };

} // namespace fmtlog
