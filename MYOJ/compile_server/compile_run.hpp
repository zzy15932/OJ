#pragma once

#include <sys/resource.h>
#include <jsoncpp/json/json.h>

#include "./comm/util.hpp"
#include "compile.hpp"
#include "Runner.hpp"

namespace ns_compile_run
{
    // 协调程序的编译和运行

    enum STATUS
    {
        EmptyCodeError = -2,
        CompileFailError
    };

    class Compile_Run
    {
    public:
        Compile_Run()
        {
        }
        ~Compile_Run()
        {
        }

        // in_json中包含:
        // 1、code：用户提交的代码
        // 2、input：用户给自己提交的代码所做的输入，不做处理
        // 3、cpu_limit：CPU运行时间限制
        // 4、mem_limit：内存限制，即空间限制

        static void start(const std::string &in_json, std::string *out_json)
        {
            // 程序状态
            int status = 0;

            // 1.把输入的in_json反序列化
            Json::Value in_value;
            Json::Reader reader;

            reader.parse(in_json, in_value);

            std::string code = in_value["code"].asString();

            // 代码为空的情况，直接处理结果
            if (code.size() == 0)
            {
                status = EmptyCodeError;
                getOutJson(status, out_json, "");
                return;
            }

            std::string input = in_value["input"].asString();
            rlim_t cpu_limit = in_value["cpu_limit"].asInt64();
            rlim_t mem_limit = in_value["mem_limit"].asInt64();

            // 2.生成一个随机且唯一的文件名
            std::string file_name = ns_util::nameUtil::getUniqueName();

            // 3.把code导入文件中
            if (!ns_util::fileUtil::writeFile(ns_util::pathUtil::src(file_name), code))
            {
                lg(Error, "写入源文件失败, errno: %d, strerror: %s\n", errno, strerror(errno));
                exit(2);
            }

            // 4.编译代码
            ns_compile::Compiler compiler;

            // 代码编译失败
            if (!compiler.compile(file_name))
            {
                status = CompileFailError;
                getOutJson(status, out_json, file_name);
                return;
            }

            // 5.运行代码
            // ns_run::Runner runner;

            status = ns_run::Runner::run(file_name, cpu_limit, mem_limit);
            getOutJson(status, out_json, file_name);
        }

        //
        // out_json中包含：
        // 1、status：状态码
        // 2、reason：请求结果
        // （选填）：
        // 3、stdout：用户程序运行完毕后的结果
        // 4、stderr：用户程序如果运行出错的错误结果
        //
        static void getOutJson(int code, std::string *out_json, const std::string &file_name)
        {
            Json::Value out_value;
            out_value["status"] = code;
            out_value["reason"] = codeToDesc(code, file_name);

            if (code == 0)
            {
                // 程序执行完成

                std::string stdout_file_name = ns_util::pathUtil::Stdout(file_name);
                std::string stderr_file_name = ns_util::pathUtil::Stderr(file_name);

                std::string stdout_file;
                std::string stderr_file;

                // 将来要给用户看的，所以要换行符
                ns_util::fileUtil::readFile(stdout_file_name, &stdout_file, true);
                ns_util::fileUtil::readFile(stderr_file_name, &stderr_file, true);

                out_value["stdout"] = stdout_file;
                out_value["stderr"] = stderr_file;
            }

            // 调试用StyledWriter，正式项目用FastWriter
            Json::StyledWriter writer;
            *out_json = writer.write(out_value);

            // removeTempFile(file_name);
        }

        static std::string codeToDesc(int code, const std::string &file_name)
        {
            switch (code)
            {
            case 0:
                return "Successful!";
            case EmptyCodeError:
                return "Empty code!";
            case CompileFailError:
            {
                // 编译出错需要知道出错的报错，而报错存再.compile_error文件中
                std::string compile_error_file_name = ns_util::pathUtil::compilerError(file_name);
                std::string compile_error_file;

                // 报错是要给用户看的，所以也需要加换行符
                ns_util::fileUtil::readFile(compile_error_file_name, &compile_error_file, true);
                return compile_error_file;
            }
            case SIGABRT: // 6
                return "Memory out of range!";
            case SIGFPE: // 8
                return "Floating overflow!";
            case SIGXCPU: // 24
                return "CPU timeout!";
            case SIGSEGV: // 11
                return "Segmentation Fault!";
            default:
            {
                return "Unknown error, status code: " + std::to_string(code);
            }
            }
        }

        static void removeTempFile(const std::string &file_name)
        {
            // 要清除的有.cpp .exe .compile_error .stdin .stdout .stderr
            // 可以使用unlink 或 remove系统调用来删除文件

            std::string src = ns_util::pathUtil::src(file_name);
            if (ns_util::fileUtil::isFileExist(src))
            {
                unlink(src.c_str());
            }

            std::string exe = ns_util::pathUtil::exe(file_name);
            if (ns_util::fileUtil::isFileExist(exe))
            {
                unlink(exe.c_str());
            }

            std::string compile_error = ns_util::pathUtil::compilerError(file_name);
            if (ns_util::fileUtil::isFileExist(compile_error))
            {
                unlink(compile_error.c_str());
            }

            std::string Stdin = ns_util::pathUtil::Stdin(file_name);
            if (ns_util::fileUtil::isFileExist(Stdin))
            {
                unlink(Stdin.c_str());
            }

            std::string Stdout = ns_util::pathUtil::Stdout(file_name);
            if (ns_util::fileUtil::isFileExist(Stdout))
            {
                unlink(Stdout.c_str());
            }

            std::string Stderr = ns_util::pathUtil::Stderr(file_name);
            if (ns_util::fileUtil::isFileExist(Stderr))
            {
                unlink(Stderr.c_str());
            }
        }
    };
}
