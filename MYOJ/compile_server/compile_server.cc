#include "compile_run.hpp"
#include "./comm/httplib.h"

const std::string http_pattern = "/compile_and_run";

void Usage(const std::string &proc)
{
    std::cout << std::endl;
    std::cout << "Usage: " << proc << " [port]" << std::endl;
    std::cout << std::endl;
}

// void segfault_handler(int signum)
// {
//     // printf("捕获到信号 %d: 段错误\n", signum);
//     // exit(1);
// }

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        Usage(argv[0]);
        exit(1);
    }

    // 注册SIGSEGV的信号处理函数,debug
    // signal(SIGSEGV, segfault_handler);

    uint32_t port = atoi(argv[1]);

    // ns_compile_run::Compile_Run compile_run;

    httplib::Server svr;

    svr.Post(http_pattern.c_str(), [](const httplib::Request &req, httplib::Response &res)
        {
            std::string in_json = req.body;
            std::string out_json;

            if (!in_json.empty())
            {
                ns_compile_run::Compile_Run::start(in_json, &out_json);
                
                // application/json;charset=utf-8设置的是响应的正文段的类型
                // 这决定了浏览器该怎样读取并且展示响应的正文段的内容
                res.set_content(out_json, "application/json;charset=utf-8");
            } 
        });

    lg(Info, "编译服务启动成功\n");
    svr.listen("0.0.0.0", port);
    return 0;
}
