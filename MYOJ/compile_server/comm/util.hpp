#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <atomic>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <cstdlib>
#include <string>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <sstream>

#include "../comm/Log.hpp"

namespace ns_util
{
    // 公共编译路径
    const std::string defaultTempPath = "/home/zzy1/2024/my-online-judge/MYOJ/compile_server/temp/";

    // 给文件名加路径和后缀
    class pathUtil
    {
    public:
        pathUtil()
        {
        }
        ~pathUtil()
        {
        }

        // 给要文件添加后缀，路径
        static std::string addSuffix(const std::string &file_name, const std::string &suffix)
        {
            return defaultTempPath + file_name + suffix;
        }

        // 程序编译时的三个临时文件
        // 生成源文件
        static std::string src(const std::string &file_name)
        {
            return addSuffix(file_name, ".cpp");
        }

        // 生成可执行文件
        static std::string exe(const std::string &file_name)
        {
            return addSuffix(file_name, ".exe");
        }

        // 生成编译错误的完整文件名
        static std::string compilerError(const std::string &file_name)
        {
            return addSuffix(file_name, ".compile_error");
        }

        // 程序运行时的三个临时文件
        // 生成标准输入的完整文件名（标准输入所在路径+后缀）(这个地方函数名必须大写，否则命名冲突)
        static std::string Stdin(const std::string &file_name)
        {
            return addSuffix(file_name, ".stdin");
        }
        // 生成标准输出的完整文件名（标准输出所在路径+后缀）
        static std::string Stdout(const std::string &file_name)
        {
            return addSuffix(file_name, ".stdout");
        }
        // 生成标准错误的完整文件名（标准错误所在路径+后缀）
        static std::string Stderr(const std::string &file_name)
        {
            return addSuffix(file_name, ".stderr");
        }
    };

    // 判断文件是否存在
    class fileUtil
    {
    public:
        fileUtil()
        {
        }
        ~fileUtil()
        {
        }
        static bool isFileExist(const std::string &file_name)
        {
            struct stat buffer;
            return (stat(file_name.c_str(), &buffer) == 0);
        }

        // keep表示是否需要\n换行符
        static bool readFile(const std::string &file_name, std::string *input, bool keep = false)
        {
            input->clear();

            std::ifstream in(file_name);
            if (!in.is_open())
            {
                return false;
            }

            std::string line;

            while (getline(in, line))
            {
                *input += line;
                *input += (keep ? "\n" : "");
            }

            in.close();
            return true;
        }

        static bool writeFile(const std::string &file_name, const std::string &output)
        {
            std::ofstream out(file_name);
            if (!out.is_open())
            {
                return false;
            }

            out.write(output.c_str(), output.size());
            out.close();
            return true;
        }
    };

    class TimeUtil
    {
    public:
        // 获取时间戳（秒级）
        static std::string GetTimeStamp()
        {
            struct timeval _time;
            gettimeofday(&_time, nullptr);
            return std::to_string(_time.tv_sec); // 累积到现在的秒数
        }
        // 获取时间戳（毫秒级）
        static std::string GetTimeMs()
        {
            struct timeval _time;
            gettimeofday(&_time, nullptr);
            return std::to_string(_time.tv_sec * 1000 + _time.tv_usec / 1000);
        }
    };

    // 生成唯一的文件名
    class nameUtil
    {
    public:
        nameUtil()
        {
        }
        ~nameUtil()
        {
        }
        static std::string getUniqueName()
        {
            static std::atomic_uint id(0); // atomic是C++提供的原子性计数器
            id++;
            std::string ms = TimeUtil::GetTimeMs();
            std::string uniq_id = to_string(id);
            return ms + "-" + uniq_id;
            // // 获取当前时间戳
            // auto now = std::chrono::system_clock::now();
            // auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

            // // 使用随机数生成器
            // std::random_device rd;  // 非确定性随机数生成器
            // std::mt19937 gen(rd()); // 使用Mersenne Twister算法的伪随机数生成器
            // std::uniform_int_distribution<> dis(1000, 9999);

            // // 生成唯一的文件名
            // std::stringstream ss;
            // ss << millis;
            // ss << "_";
            // ss << dis(gen); // 添加一个随机数来进一步确保唯一性

            // return ss.str();
        }
    };

    // 切分字符串工具
    class splitUtil
    {
    public:
        splitUtil()
        {
        }
        ~splitUtil()
        {
        }
        static void splitString(const std::string& src, const std::string& sep, std::vector<std::string>* tokens)
        {
            tokens->clear();
            boost::split(*tokens, src, boost::is_any_of(sep), boost::algorithm::token_compress_on);
        }
    };
}