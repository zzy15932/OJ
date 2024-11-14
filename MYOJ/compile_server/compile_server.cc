#include "compile_run.hpp"
#include "./comm/httplib.h"

const std::string http_pattern = "/compile_and_run";

void Usage(const std::string &proc)
{
    std::cout << std::endl;
    std::cout << "Usage: " << proc << " [port]" << std::endl;
    std::cout << std::endl;
}


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        Usage(argv[0]);
        exit(1);
    }

    if (argc == 3)
    {
        std::string option(argv[2]);
        if (option == "-d")
        {
            // 1、守护进程化
            if (daemon(1, 0) == -1)
            {
                perror("compile_server 守护进程化失败！\n");
                exit(2);
            }
        }
    }

    uint32_t port = atoi(argv[1]);

    // ns_compile_run::Compile_Run compile_run;

    httplib::Server svr;

    svr.Post(http_pattern.c_str(), [](const httplib::Request &req, httplib::Response &res)
             {
            // std::cout << "收到post请求" << std::endl;
            lg(Info, "收到请求...\n");
            std::string in_json = req.body;
            std::string out_json;

            if (!in_json.empty())
            {
                // std::cout << "调用start" << std::endl;
                ns_compile_run::Compile_Run::start(in_json, &out_json);
                
                // application/json;charset=utf-8设置的是响应的正文段的类型
                // 这决定了浏览器该怎样读取并且展示响应的正文段的内容
                res.set_content(out_json, "application/json;charset=utf-8");
            } });

    lg(Info, "编译服务启动成功\n");
    svr.listen("0.0.0.0", port);
    return 0;
}
