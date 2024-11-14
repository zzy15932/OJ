// 运行模块
#pragma once
#include <sys/resource.h>

#include "./comm/util.hpp"
#include "./comm/Log.hpp"

namespace ns_run
{
    // 提供运行接口，限制资源接口
    class Runner
    {
    public:
        Runner()
        {
        }
        ~Runner()
        {
        }

        static void setProLimit(const rlim_t cpu_limit, const rlim_t mem_limit)
        {
            struct rlimit cl;
            cl.rlim_cur = cpu_limit;
            cl.rlim_max = RLIM_INFINITY;

            // std::cout << "cpu_limit: " << cpu_limit << std::endl;

            if (setrlimit(RLIMIT_CPU, &cl) != 0)
            {
                lg(Error, "设置CPU资源限制失败, errno: %d, strerror: %s\n", errno, strerror(errno));
                exit(6);
            }

            // 内存是按照KB为单位的
            struct rlimit ml;
            ml.rlim_cur = mem_limit * 1024;
            ml.rlim_max = RLIM_INFINITY;

            if (setrlimit(RLIMIT_AS, &ml) != 0)
            {
                lg(Error, "设置MEM资源限制失败, errno: %d, strerror: %s\n", errno, strerror(errno));
                exit(7);
            }
        }

        // 需要返回运行的状态码
        static int run(const std::string &file_name, const rlim_t cpu_limit, const rlim_t mem_limit)
        {
            std::string _exe = ns_util::pathUtil::exe(file_name);
            std::string _stdin = ns_util::pathUtil::Stdin(file_name);
            std::string _stdout = ns_util::pathUtil::Stdout(file_name);
            std::string _stderr = ns_util::pathUtil::Stderr(file_name);

            umask(0);
            int _stdinfd = open(_stdin.c_str(), O_CREAT | O_RDONLY, 0664);
            int _stdoutfd = open(_stdout.c_str(), O_CREAT | O_WRONLY, 0664);
            int _stderrfd = open(_stderr.c_str(), O_CREAT | O_WRONLY, 0664);

            if (_stdinfd < 0 || _stdoutfd < 0 || _stderrfd < 0)
            {
                lg(Error, "运行时打开文件描述符失败, errno: %d, strerror: %s\n", errno, strerror(errno));
                exit(-2);
            }

            pid_t pid = fork();
            if (pid == 0)
            {
                // child
                // 重定向0,1,2到_stdinfd,_stdoutfd,_stderrfd

                // for debug
                // dup2(debug_fd, 1);

                if (dup2(_stdinfd, 0) < 0 ||
                    dup2(_stdoutfd, 1) < 0 ||
                    dup2(_stderrfd, 2) < 0)
                {
                    close(_stdinfd);
                    close(_stdoutfd);
                    close(_stderrfd);

                    lg(Error, "运行时文件描述符重定向失败, errno: %d, strerror: %s\n", errno, strerror(errno));
                    exit(-3);
                }
                setProLimit(cpu_limit, mem_limit);
                // std::cout<<"子进程执行程序替换"<< _exe << std::endl;
                execlp(_exe.c_str(), _exe.c_str(), nullptr);

                lg(Error, "运行时程序替换失败, errno: %d, strerror: %s\n", errno, strerror(errno));
                // ns_util::fileUtil::writeFile("./debugErrorLog.txt", )
                // open("./debugErrorLog.txt", O_CREAT, 0664);
                close(_stdinfd);
                close(_stdoutfd);
                close(_stderrfd);

                exit(-1);
            }
            else if (pid > 0)
            {
                // parent
                close(_stdinfd);
                close(_stdoutfd);
                close(_stderrfd);

                int status = 0;
                waitpid(pid, &status, 0);
                // 子进程运行完毕
                // 1.子进程正常退出，run_res会为0
                // 2.子进程异常终止，run_res会存储异常退出的信号
                // std::cout << status << std::endl;
                if (WIFEXITED(status))
                {
                    int exit_code = WEXITSTATUS(status);
                    lg(Info, "%s 正常退出, exit_code = %d\n", _exe.c_str(), exit_code);
                    // std::cout << "进入WIFEXITED(status)" << std::endl;
                    return exit_code;
                }
                else if (WIFSIGNALED(status))
                {
                    int term_signal = WTERMSIG(status);
                    lg(Error, "%s 被信号终止, signal = %d\n", _exe.c_str(), term_signal);
                    // std::cout << "进入WIFSIGNALED(status)，打印：" << status << std::endl;
                    return term_signal;
                }

                // int run_res = status & 0x7f;
                // lg(Info, "%s 运行完毕, run_res = %d\n", _exe.c_str(), run_res);
                // return run_res; // for debug
                
                return status & 0x7f;
            }
            else
            {
                // 创建子进程失败
                lg(Error, "运行文件时创建子进程失败, errno:%d, strerror:%s\n", errno, strerror(errno));
                exit(8);
            }
        }
    };
}
