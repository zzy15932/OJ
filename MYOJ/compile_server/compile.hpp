// 编译模块
#pragma once
#include "../comm/util.hpp"


namespace ns_compile
{
    // 提供编译接口
    class Compiler
    {
    public:
        Compiler()
        {}
        ~Compiler()
        {}

        // 编译文件，并且返回编译结果
        bool compile(const std::string& file_name)
        {
            pid_t id = fork();
            if (id == -1)
            {
                // 创建失败
                lg(Error, "编译文件时创建子进程失败, errno:%d, strerror:%s\n",errno, strerror(errno));
                exit(1);
            }
            else if (id == 0)
            {
                // child
                umask(0);
                int _stderr = open(ns_util::pathUtil::compilerError(file_name).c_str(), 
                O_CREAT | O_WRONLY, 0664);
                if (_stderr < 0)
                {
                    lg(Error, "子进程创建_stderr时出错, errno:%d, strerror:%s\n",errno, strerror(errno));
                    exit(2);
                }

                int ret = dup2(_stderr, 2);
                if (ret == -1)
                {
                    lg(Error, "子进程标准错误重定向错误, errno:%d, strerror:%s\n",errno, strerror(errno));
                    exit(3);
                }

                // 进程替换
                execlp("g++", "g++", "-o", ns_util::pathUtil::exe(file_name).c_str(), 
                ns_util::pathUtil::src(file_name).c_str(), 
                "-D", "COMPILER_ONLINE", "-std=c++11", nullptr);

                lg(Error, "进程替换错误, errno:%d, strerror:%s\n",errno, strerror(errno));
                exit(4);
            }
            else if (id > 0)
            {
                // parent
                waitpid(id, nullptr, 0);

                if (ns_util::fileUtil::isFileExist(ns_util::pathUtil::exe(file_name)))
                {
                    lg(Info, "%s 文件编译成功!\n", ns_util::pathUtil::src(file_name).c_str());
                    return true;
                }
                lg(Error, "%s 文件编译失败!\n", ns_util::pathUtil::src(file_name).c_str());
                return false;
            }
            else
            {
                // 意料之外的情况
                lg(Debug, "compile创建子进程出现意料之外的情况\n");
                exit(5);
            }
        }
    };
}