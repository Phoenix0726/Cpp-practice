#include "log.h"
#include <map>
#include <iostream>
#include <functional>
#include <time.h>


namespace sylar {

class MessageFormatItem : public LogFormatter::FormatItem {
public:
    MessageFormatItem(const std::string& str="") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getContent();
    }
};

class LevelFormatItem : public LogFormatter::FormatItem {
public:
    LevelFormatItem(const std::string& str="") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << LogLevel::toString(level);
    }
};

class ElapseFormatItem : public LogFormatter::FormatItem {
public:
    ElapseFormatItem(const std::string& str="") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getElapse();
    }
};

class NameFormatItem : public LogFormatter::FormatItem {
public:
    NameFormatItem(const std::string& str="") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << logger->getName();
    }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
    ThreadIdFormatItem(const std::string& str="") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getThreadId();
    }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
    FiberIdFormatItem(const std::string& str="") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFiberId();
    }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
private:
    std::string m_format;
public:
    DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S") 
        : m_format(format) {
        if (m_format.empty()) {
            m_format = "%Y-%m-%d %H:%M:%S";
        }
    }

    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }
};

class FilenameFormatItem : public LogFormatter::FormatItem {
public:
    FilenameFormatItem(const std::string& str="") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFile();
    }
};

class LineFormatItem : public LogFormatter::FormatItem {
public:
    LineFormatItem(const std::string& str="") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getLine();
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    NewLineFormatItem(const std::string& str="") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << std::endl;
    }
};

class TabFormatItem : public LogFormatter::FormatItem {
public:
    TabFormatItem(const std::string& str="") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << "\t";
    }
};

class StringFormatItem : public LogFormatter::FormatItem {
private:
    std::string m_string;
public:
    StringFormatItem(const std::string& str) 
        : m_string(str) {}

    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << m_string;
    }
};


Logger::Logger(const std::string& name) 
    : m_name(name), m_level(LogLevel::DEBUG) {
    m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        auto self = shared_from_this();
        for (auto& appender : m_appenders) {
            appender->log(self, level, event);
        }
    }
}

void Logger::debug(LogEvent::ptr event) {
    log(LogLevel::DEBUG, event);
}

void Logger::info(LogEvent::ptr event) {
    log(LogLevel::INFO, event);
}

void Logger::warn(LogEvent::ptr event) {
    log(LogLevel::WARN, event);
}

void Logger::error(LogEvent::ptr event) {
    log(LogLevel::ERROR, event);
}

void Logger::fatal(LogEvent::ptr event) {
    log(LogLevel::FATAL, event);
}

void Logger::addAppender(LogAppender::ptr appender) {
    if (!appender->getFormatter()) {
        appender->setFormatter(m_formatter);
    }
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
    for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it) {
        if (*it == appender) {
            m_appenders.erase(it);
            break;
        }
    }
}

LogLevel::Level Logger::getLevel() const {
    return m_level;
}

void Logger::setLevel(LogLevel::Level level) {
    m_level = level;
}

const std::string& Logger::getName() const {
    return m_name;
}


LogEvent::LogEvent(Logger::ptr logger, LogLevel::Level level, const char* file, int32_t line, uint32_t elapse, uint32_t threadId, uint32_t fiberId, uint64_t time) 
    : m_logger(logger), m_level(level), m_file(file), m_line(line), m_elapse(elapse), m_threadId(threadId), m_fiberId(fiberId), m_time(time) {
}

Logger::ptr LogEvent::getLogger() const {
    return m_logger;
}

LogLevel::Level LogEvent::getLevel() const {
    return m_level;
}

const char* LogEvent::getFile() const {
    return m_file;
}

int32_t LogEvent::getLine() const {
    return m_line;
}

uint32_t LogEvent::getElapse() const {
    return m_elapse;
}

uint32_t LogEvent::getThreadId() const {
    return m_threadId;
}

uint32_t LogEvent::getFiberId() const {
    return m_fiberId;
}

uint64_t LogEvent::getTime() const {
    return m_time;
}

std::string LogEvent::getContent() const {
    return m_ss.str();
}

std::stringstream& LogEvent::getSS() {
    return m_ss;
}

void LogEvent::format(const char* fmt, ...) {
    va_list al;
    va_start(al, fmt);
    format(fmt, al);
    va_end(al);
}

void LogEvent::format(const char* fmt, va_list al) {
    char* buf = nullptr;
    int len = vasprintf(&buf, fmt, al);
    if (len != -1) {
        m_ss << std::string(buf, len);
        free(buf);
    }
}


LogEventWrap::LogEventWrap(LogEvent::ptr e) : 
    m_event(e) {
}

LogEventWrap::~LogEventWrap() {
    m_event->getLogger()->log(m_event->getLevel(), m_event);
}

LogEvent::ptr LogEventWrap::getEvent() const {
    return m_event;
}

std::stringstream& LogEventWrap::getSS() {
    return m_event->getSS();
}


const char* LogLevel::toString(LogLevel::Level level) {
    switch (level) {
#define XX(name) \
    case LogLevel::name: \
        return #name; \
        break;

    XX(DEBUG);
    XX(INFO);
    XX(WARN);
    XX(ERROR);
    XX(FATAL);
#undef XX
    default:
        return "UNKNOW";
    }
    return "UNKNOW";
}


void LogAppender::setFormatter(LogFormatter::ptr formatter) {
    m_formatter = formatter;
}

LogFormatter::ptr LogAppender::getFormatter() const {
    return m_formatter;
}

void LogAppender::setLevel(LogLevel::Level level) {
    m_level = level;
}

LogLevel::Level LogAppender::getLevel() const {
    return m_level;
}

void StdoutLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        std::cout << m_formatter->format(logger, level, event);
    }
}

FileLogAppender::FileLogAppender(const std::string& filename) 
    : m_filename(filename) {
    reopen();
}

void FileLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        m_filestream << m_formatter->format(logger, level, event);
    }
}

bool FileLogAppender::reopen() {
    if (m_filestream) {
        m_filestream.close();
    }
    m_filestream.open(m_filename, std::ios::app);
    return !!m_filestream;
}


LogFormatter::LogFormatter(const std::string& pattern)
    : m_pattern(pattern) {
    init();
}

std::string LogFormatter::format(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) {
    std::stringstream ss;
    for (auto& item : m_items) {
        item->format(ss, logger, level, event);
    }
    return ss.str();
}

void LogFormatter::init() {
    // str, format, type
    std::vector<std::tuple<std::string, std::string, int>> vec;
    std::string nstr;

    for (size_t i = 0; i < m_pattern.size(); ++i) {
        if (m_pattern[i] != '%') {
            nstr.append(1, m_pattern[i]);
            continue;
        }

        if ((i + 1) < m_pattern.size()) {
            if (m_pattern[i + 1] == '%') {
                nstr.append(1, '%');
                continue;
            }
        }

        size_t n = i + 1;
        int fmt_status = 0;
        size_t fmt_begin = 0;
        std::string str;
        std::string fmt;

        while (n < m_pattern.size()) {
            char ch = m_pattern[n];
            if (!fmt_status && (!isalpha(ch) && ch != '{' && ch != '}')) {
                str = m_pattern.substr(i + 1, n - i - 1);
                break;
            }

            if (fmt_status == 0) {
                if (ch == '{') {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    fmt_status = 1;
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            } else if (fmt_status == 1) {
                if (ch == '}') {
                    fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                    fmt_status = 0;
                    ++n;
                    break;
                }
            }

            ++n;

            if (n == m_pattern.size()) {
                if (str.empty()) {
                    str = m_pattern.substr(i + 1);
                }
            }
        }

        if (fmt_status == 0) {
            if (!nstr.empty()) {
                vec.push_back({nstr, std::string(), 0});
                nstr.clear();
            }
            vec.push_back({str, fmt, 1});
            i = n - 1;
        } else if (fmt_status == 1) {
            std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
            vec.push_back({"<<pattern_error>>", fmt, 0});
        }
    }

    if (!nstr.empty()) {
        vec.push_back({nstr, "", 0});
    }

    static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)>> s_format_items = {
#define XX(str, C) \
    {#str, [](const std::string& fmt) { return FormatItem::ptr(new C(fmt)); }}

    XX(m, MessageFormatItem),
    XX(p, LevelFormatItem),
    XX(r, ElapseFormatItem),
    XX(c, NameFormatItem),
    XX(t, ThreadIdFormatItem),
    XX(F, FiberIdFormatItem),
    XX(d, DateTimeFormatItem),
    XX(f, FilenameFormatItem),
    XX(l, LineFormatItem),
    XX(n, NewLineFormatItem),
    XX(T, TabFormatItem),
#undef XX
    };

    for (auto v : vec) {
        if (std::get<2>(v) == 0) {
            m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(v))));
        } else {
            auto it = s_format_items.find(std::get<0>(v));
            if (it == s_format_items.end()) {
                m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(v) + ">>")));
            } else {
                m_items.push_back(it->second(std::get<1>(v)));
            }
        }

        // std::cout << std::get<0>(v) << " - " << std::get<1>(v) << " - " << std::get<2>(v) << std::endl;
    }
}


LoggerManager::LoggerManager() {
    m_root.reset(new Logger);
    m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));
}

Logger::ptr LoggerManager::getLogger(const std::string& name) {
    auto it = m_loggers.find(name);
    return it == m_loggers.end() ? m_root : it->second;
}

Logger::ptr LoggerManager::getRoot() const {
    return m_root;
}

};
