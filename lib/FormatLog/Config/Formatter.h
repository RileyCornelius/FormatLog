using MemoryBuffer = fmt::basic_memory_buffer<char, LOG_STATIC_BUFFER_SIZE>;

struct LogMessage
{
    LogMessage(uint32_t logTime,
               SourceLocation loc,
               LogLevel lvl,
               fmt::string_view msg) : time(logTime),
                                       source(loc),
                                       level(lvl),
                                       payload(msg)
    {
    }

    LogLevel level = LogLevel::OFF;
    uint32_t time;

    // wrapping the formatted text with color (updated by pattern_formatter).
    size_t color_range_start{0};
    size_t color_range_end{0};

    SourceLocation source;
    fmt::string_view payload;
};

class Formatter
{
public:
    virtual ~Formatter() = default;
    virtual void format(const LogMessage &msg, MemoryBuffer &dest) = 0;
    virtual Formatter clone() const = 0;
};