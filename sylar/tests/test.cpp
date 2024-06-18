#include <sys/syscall.h>
#include <sys/types.h>

#include <iostream>
#include <unistd.h>

#include "sylar/log.h"


int main()
{
    sylar::Logger::ptr logger(new sylar::Logger);

    logger->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));
    
    sylar::FileLogAppender::ptr fileAppender(new sylar::FileLogAppender("log.txt"));
    sylar::LogFormatter::ptr fmt(new sylar::LogFormatter("%d%T%p%T%m%n"));
    fileAppender->setFormatter(fmt);
    fileAppender->setLevel(sylar::LogLevel::ERROR);
    logger->addAppender(fileAppender);

    // sylar::LogEvent::ptr event(new sylar::LogEvent(logger, sylar::LogLevel::DEBUG, __FILE__, __LINE__, 0, syscall(SYS_gettid), 2, time(0)));

    // logger->log(sylar::LogLevel::DEBUG, event);


    SYLAR_LOG_INFO(logger) << "hello sylar, " << "this is a log";

    SYLAR_LOG_FMT_ERROR(logger, "custom format %s", "dpmn");

    auto lgr = sylar::LoggerMgr::getInstance()->getLogger("root?");
    SYLAR_LOG_INFO(lgr) << "lalala" << std::endl;

    return 0;
}
