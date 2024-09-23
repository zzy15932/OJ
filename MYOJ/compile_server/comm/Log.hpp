#pragma once
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>


enum PrintMethods
{
    Screen = 1,
    Onefile,
    Classfile
};

enum LogLevel
{
    Info = 0,
    Debug,
    Warning,
    Error,
    Fatal
};

std::string defaultPath = "./Log/";

class Log
{
public:
    // 初始化日志，默认向屏幕上输出，输出文件路径默认设置为当前路径下Log.txt文件
    Log(const int printMethod = Screen, const std::string &path = defaultPath)
        : _path(path), _printMethod(printMethod)
    {
    }

    // 改变日志的输出方式
    void Enable(int newMethod)
    {
        _printMethod = newMethod;
    }

    std::string LevelToString(int level)
    {
        switch (level)
        {
        case Info:
            return "Info";
            break;
        case Debug:
            return "Debug";
            break;
        case Warning:
            return "Warning";
            break;
        case Error:
            return "Error";
            break;
        case Fatal:
            return "Fatal";
            break;
        default:
            return "None";
            break;
        }
    }

    void PrintLog(const std::string &LogMessage, int level)
    {
        switch (_printMethod)
        {
        case Screen:
            printf("%s", LogMessage.c_str());
            break;
        case Onefile:
            printOnefile("log.txt", LogMessage);
            break;
        case Classfile:
            printClassFile(LogMessage, level);
            break;
        default:
            break;
        }
    }

    void printOnefile(const std::string &logName, const std::string &LogMessage)
    {
        std::string fileName = _path;
        fileName += logName;
        int fd = open(fileName.c_str(), O_APPEND | O_WRONLY | O_CREAT, 0666);
        if (fd < 0)
        {
            perror("printOnefile open");
            return;
        }
        write(fd, LogMessage.c_str(), LogMessage.size());
        close(fd);
    }

    void printClassFile(const std::string &LogMessage, int level)
    {
        std::string fileName = "log.txt.";
        fileName += LevelToString(level);
        printOnefile(fileName, LogMessage);
    }

    void operator()(int level, const char *format, ...)
    {
        time_t t;
        time(&t);
        struct tm *p;
        p = localtime(&t);
        char leftBuffer[1024];
        snprintf(leftBuffer, sizeof(leftBuffer) - 1, "[%s][%d-%d-%d %d:%d:%d]", LevelToString(level).c_str(),
                p->tm_year + 1900, p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
        char rightBuffer[1024];
        va_list ap;
        va_start(ap, format);
        vsnprintf(rightBuffer, sizeof(rightBuffer) - 1, format, ap);
        std::string LogMessage(leftBuffer);
        LogMessage += " ";
        LogMessage += rightBuffer;
        PrintLog(LogMessage, level);
    }

private:
    std::string _path;
    int _printMethod;
};


Log lg;