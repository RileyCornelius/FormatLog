// More information about PlatformIO Unit Testing:
// https://docs.platformio.org/en/latest/advanced/unit-testing/index.html

#include <Arduino.h>
#include <string>
#include <cstring>
#include "unity.h"

// Forward declare custom halt used by ASSERT to prevent infinite loop
void testPanic();

/*------------------------------------------------------------------------------
 * Test Stream to capture output
 *----------------------------------------------------------------------------*/

class TestStream : public Stream
{
private:
    std::string buffer;

public:
    size_t write(uint8_t ch) override
    {
        buffer.push_back(static_cast<char>(ch));
        return 1;
    }
    size_t write(const uint8_t *data, size_t size) override
    {
        buffer.append(reinterpret_cast<const char *>(data), size);
        return size;
    }
    int available() override { return 0; }
    int read() override { return -1; }
    int peek() override { return -1; }
    void flush() override {}

    void clear() { buffer.clear(); }
    const std::string &str() const { return buffer; }
    const char *c_str() const { return buffer.c_str(); }
};

TestStream gStream;

// Redirect logger defaults to our test stream and custom halt before including logger headers
#define LOG_STREAM gStream
#define LOG_PANIC_HANDLER testPanic
#define LOG_LEVEL LOG_LEVEL_TRACE
#define LOG_COLOR LOG_COLOR_DISABLE
#define LOG_TIME LOG_TIME_DISABLE
#define LOG_FILENAME LOG_FILENAME_DISABLE

#include "FormatLog.h"

#include "Config/Preamble.h"

bool gHalted = false;
void testPanic() { gHalted = true; }

/*------------------------------------------------------------------------------
 * TESTS FOR FormatLog
 *----------------------------------------------------------------------------*/

void test_log_trace_basic_string()
{
    gStream.clear();
    LOG_SET_LOG_LEVEL(LogLevel::TRACE);

    LOG_TRACE("Hello {}", 42);

    const std::string &out = gStream.str();
    TEST_ASSERT_FALSE_MESSAGE(out.empty(), "TRACE log produced no output");
    TEST_ASSERT_NOT_NULL(strstr(out.c_str(), "Hello 42"));
}

void test_log_level_filtering()
{
    gStream.clear();
    LOG_SET_LOG_LEVEL(LogLevel::ERROR);

    LOG_WARN("ShouldNotAppear");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, (unsigned int)gStream.str().size(), "WARN should be filtered out");

    gStream.clear();
    LOG_ERROR("Boom");
    const std::string &out = gStream.str();
    TEST_ASSERT_NOT_NULL(strstr(out.c_str(), "Boom"));
}

void test_print_and_println()
{
    gStream.clear();
    LOG_SET_LOG_LEVEL(LogLevel::TRACE);

    LOG_PRINT("ABC");
    TEST_ASSERT_EQUAL_STRING("ABC", gStream.c_str());

    gStream.clear();
    LOG_PRINTLN("X{}Y", 7);
    const std::string &out = gStream.str();
    TEST_ASSERT_NOT_NULL(strstr(out.c_str(), "X7Y"));
    const size_t eolLen = strlen(LOG_EOL);
    TEST_ASSERT_TRUE_MESSAGE(out.size() >= eolLen, out.c_str());
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, memcmp(out.c_str() + out.size() - eolLen, LOG_EOL, eolLen), out.c_str());
}

void test_assertion_outputs_and_halts()
{
    gStream.clear();
    gHalted = false;

    ASSERT(false, "Oops");

    const std::string &out = gStream.str();
    // Starts with CRLF then [ASSERT]
    TEST_ASSERT_TRUE_MESSAGE(out.find("\r\n[ASSERT] ") == 0, "ASSERT prefix missing");
    TEST_ASSERT_NOT_NULL(strstr(out.c_str(), "test_log.cpp"));
    TEST_ASSERT_NOT_NULL(strstr(out.c_str(), "Oops"));
    TEST_ASSERT_TRUE_MESSAGE(gHalted, "Assertion did not invoke halt function");
}

void test_log_runtime_level_api()
{
    gStream.clear();

    LOG_SET_LOG_LEVEL(LogLevel::WARN);
    TEST_ASSERT_EQUAL(static_cast<int>(LogLevel::WARN), static_cast<int>(LOG_GET_LOG_LEVEL()));

    LOG_INFO("Hidden message");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, (unsigned int)gStream.str().size(), "INFO should be filtered when WARN level set");

    gStream.clear();
    LOG_SET_LOG_LEVEL(LogLevel::DEBUG);
    TEST_ASSERT_EQUAL(static_cast<int>(LogLevel::DEBUG), static_cast<int>(LOG_GET_LOG_LEVEL()));

    LOG_DEBUG("Visible {}", 12);
    const std::string &out = gStream.str();
    TEST_ASSERT_FALSE_MESSAGE(out.empty(), "DEBUG log produced no output");
    TEST_ASSERT_NOT_NULL(strstr(out.c_str(), "Visible 12"));
}

void test_log_level_stepdown()
{
    gStream.clear();

    LOG_SET_LOG_LEVEL(LogLevel::INFO);
    LOG_DEBUG("Filtered debug");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, (unsigned int)gStream.str().size(), "DEBUG should be filtered when INFO level set");

    gStream.clear();
    LOG_INFO("Shown {}", 99);
    const std::string &infoOut = gStream.str();
    TEST_ASSERT_FALSE_MESSAGE(infoOut.empty(), "INFO log produced no output at INFO level");
    TEST_ASSERT_NOT_NULL(strstr(infoOut.c_str(), "Shown 99"));

    gStream.clear();
    LOG_WARN("Warning message");
    const std::string &warnOut = gStream.str();
    TEST_ASSERT_FALSE_MESSAGE(warnOut.empty(), "WARN log produced no output at INFO level");
    TEST_ASSERT_NOT_NULL(strstr(warnOut.c_str(), "Warning message"));
}

void test_direct_value_logging()
{
    gStream.clear();
    LOG_SET_LOG_LEVEL(LogLevel::TRACE);

    const SourceLocation loc(__FILE__, __LINE__, __FUNCTION__);
    FormatLog::instance().trace(loc, 12345);

    const std::string &out = gStream.str();
    TEST_ASSERT_FALSE_MESSAGE(out.empty(), "Direct trace value produced no output");
    TEST_ASSERT_NOT_NULL(strstr(out.c_str(), "12345"));
}

void test_preamble_helpers()
{
    TEST_ASSERT_EQUAL_STRING("TRAC", preamble::logLevelText(LogLevel::TRACE, LogLevelTextFormat::SHORT));
    TEST_ASSERT_EQUAL_STRING("test_FormatLog", preamble::formatFilename("/path/to/test_FormatLog.cpp", 123,
                                                                        "irrelevant", LogFilename::ENABLE));

    const char *withLineFunc = preamble::formatFilename("test_FormatLog.cpp", 77, "fn", LogFilename::LINENUMBER_FUNCTION_ENABLE);
    TEST_ASSERT_NOT_NULL(strstr(withLineFunc, "test_FormatLog.cpp"));
    TEST_ASSERT_NOT_NULL(strstr(withLineFunc, "77"));
    TEST_ASSERT_NOT_NULL(strstr(withLineFunc, "fn"));
}

void test_assertion_pass_does_not_halt()
{
    gStream.clear();
    gHalted = false;

    ASSERT(true);

    TEST_ASSERT_FALSE_MESSAGE(gHalted, "Assertion with true condition should not halt");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, (unsigned int)gStream.str().size(), "Assertion with true condition should not log");
}

void test_long_message_logging()
{
    gStream.clear();
    LOG_SET_LOG_LEVEL(LogLevel::TRACE);

    const size_t payloadLen = static_cast<size_t>(LOG_STATIC_BUFFER_SIZE) + 32;
    std::string payload(payloadLen, 'X');
    payload.replace(0, 5, "BEGIN");
    payload.replace(payload.size() - 5, 5, "END!!");

    LOG_TRACE("{}", payload);

    const std::string &out = gStream.str();
    TEST_ASSERT_FALSE_MESSAGE(out.empty(), "Long TRACE message produced no output");
    const std::string::size_type payloadPos = out.find(payload);
    TEST_ASSERT_NOT_EQUAL_MESSAGE(std::string::npos, payloadPos, "Long TRACE message was truncated");
    const size_t eolLen = strlen(LOG_EOL);
    TEST_ASSERT_TRUE_MESSAGE(out.size() >= payload.size() + eolLen, out.c_str());
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, memcmp(out.c_str() + out.size() - eolLen, LOG_EOL, eolLen), out.c_str());
}

/*------------------------------------------------------------------------------
 * SETUP AND TEST RUNNER
 *----------------------------------------------------------------------------*/

void setUp(void)
{
    // Ensure buffer is clean
    gStream.clear();
    LOG_SET_LOG_LEVEL(LogLevel::TRACE);
}

void tearDown(void)
{
    // Nothing to clean
}

void tests()
{
    RUN_TEST(test_log_trace_basic_string);
    RUN_TEST(test_log_level_filtering);
    RUN_TEST(test_print_and_println);
    RUN_TEST(test_assertion_outputs_and_halts);
    RUN_TEST(test_log_runtime_level_api);
    RUN_TEST(test_log_level_stepdown);
    RUN_TEST(test_direct_value_logging);
    RUN_TEST(test_preamble_helpers);
    RUN_TEST(test_assertion_pass_does_not_halt);
    RUN_TEST(test_long_message_logging);
}

void setup()
{
    // NOTE!!! Wait for >2 secs if board doesn't support software reset via Serial.DTR/RTS
    delay(4000);

    UNITY_BEGIN();
    tests();
    UNITY_END();
}

void loop()
{
}
