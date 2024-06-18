#ifndef _SYLAR_LOG_H_
#define _SYLAR_LOG_H_

#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdarg.h>
#include <map>
#include "singleton.h"


#define SYLAR_LOG_LEVEL(logger, level) \
    if (logger->getLevel() <= level) \
        sylar::LogEventWrap(sylar::LogEvent::ptr(new sylar::LogEvent(logger, level, __FILE__, __LINE__, 0, syscall(SYS_gettid), 0, time(0)))).getSS()

#define SYLAR_LOG_DEBUG(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::DEBUG)
#define SYLAR_LOG_INFO(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::INFO)
#define SYLAR_LOG_WARN(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::WARN)
#define SYLAR_LOG_ERROR(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::ERROR)
#define SYLAR_LOG_FATAL(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::FATAL)

#define SYLAR_LOG_FMT_LEVEL(logger, level, fmt, ...) \
    if (logger->getLevel() <= level) \
        sylar::LogEventWrap(sylar::LogEvent::ptr(new sylar::LogEvent(logger, level, __FILE__, __LINE__, 0, syscall(SYS_gettid), 0, time(0)))).getEvent()->format(fmt, __VA_ARGS__)

#define SYLAR_LOG_FMT_DEBUG(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_INFO(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::INFO, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_WARN(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::WARN, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_ERROR(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::ERROR, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_FATAL(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::FATAL, fmt, __VA_ARGS__)


namespace sylar {

class Logger;


class LogLevel {
public:
    enum Level {
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5,
    };

    static const char* toString(LogLevel::Level level);
};


class LogEvent {
private:
    std::shared_ptr<Logger> m_logger;
    LogLevel::Level m_level;
    const char* m_file = nullptr;
    int32_t m_line = 0;
    uint32_t m_elapse = 0;
    uint32_t m_threadId = 0;
    uint32_t m_fiberId = 0;
    uint64_t m_time = 0;
    std::stringstream m_ss;
public:
    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file, int32_t line, uint32_t elapse, uint32_t threadId, uint32_t fiberId, uint64_t time);

    std::shared_ptr<Logger> getLogger() const;
    LogLevel::Level getLevel() const;
    const char* getFile() const;
    int32_t getLine() const;
    uint32_t getElapse() const;
    uint32_t getThreadId() const;
    uint32_t getFiberId() const;
    uint64_t getTime() const;
    std::string getContent() const;
    std::stringstream& getSS();
    void format(const char* fmt, ...);
    void format(const char* fmt, va_list al);
};


class LogEventWrap {
private:
    LogEvent::ptr m_event;
public:
    LogEventWrap(LogEvent::ptr e);
    ~LogEventWrap();
    LogEvent::ptr getEvent() const;
    std::stringstream& getSS();
};


class LogFormatter {
public:
    class FormatItem {
    public:
        typedef std::shared_ptr<FormatItem> ptr;
        virtual ~FormatItem() {}
        virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    };
private:
    std::string m_pattern;
    std::vector<FormatItem::ptr> m_items;
public:
    typedef std::shared_ptr<LogFormatter> ptr;

    LogFormatter(const std::string& pattern);
    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
private:
    void init();
};


class LogAppender {
protected:
    LogLevel::Level m_level = LogLevel::DEBUG;
    LogFormatter::ptr m_formatter;
public:
    typedef std::shared_ptr<LogAppender> ptr;
    virtual ~LogAppender() {}

    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;

    void setFormatter(LogFormatter::ptr formatter);
    LogFormatter::ptr getFormatter() const;
    void setLevel(LogLevel::Level level);
    LogLevel::Level getLevel() const;
};


class Logger : public std::enable_shared_from_this<Logger> {
private:
    std::string m_name;
    LogLevel::Level m_level;
    std::list<LogAppender::ptr> m_appenders;
    LogFormatter::ptr m_formatter;
public:
    typedef std::shared_ptr<Logger> ptr;
    
    Logger(const std::string& name = "root");
    void log(LogLevel::Level level, LogEvent::ptr event);

    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    LogLevel::Level getLevel() const;
    void setLevel(LogLevel::Level level);
    const std::string& getName() const;
};


class StdoutLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;

    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
};


class FileLogAppender : public LogAppender {
private:
    std::string m_filename;
    std::ofstream m_filestream;
public:
    typedef std::shared_ptr<FileLogAppender> ptr;

    FileLogAppender(const std::string& filename);
    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;

    bool reopen();
};


class LoggerManager {
private:
    std::map<std::string, Logger::ptr> m_loggers;
    Logger::ptr m_root;
public:
    LoggerManager();
    Logger::ptr getLogger(const std::string& name);
    Logger::ptr getRoot() const;
    void init();
};

typedef sylar::Singleton<LoggerManager> LoggerMgr;

}

#endif
