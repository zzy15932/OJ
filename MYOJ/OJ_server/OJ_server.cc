#include <fstream>
#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "OJ_Control.hpp"

static ns_control::Control *ctrl_ptr = nullptr;

void recovery(int signo)
{
    ctrl_ptr->recoveryMachine();
}

int main(int argc, char *argv[])
{
    // 0、修改指定信号的默认行为，把ctrl + \产生的SIGQUIT信号重定向为一键上线所有主机（交由运维使用）
    signal(SIGQUIT, recovery);

    // 1、守护进程化
    if (argc == 2)
    {
        std::string option(argv[1]);
        if (option == "-d")
        {
            // 1、守护进程化
            if (daemon(1, 0) == -1)
            {
                perror("OJ_server 守护进程化失败！\n");
                exit(1);
            }
        }
    }

    // 2、用户请求的服务路由功能（用户不同的请求前往不同的区域）
    httplib::Server svr;      // httplib中的Server类
    ns_control::Control ctrl; // 核心业务逻辑的控制器（初始化的同时，加载题库）
    ctrl_ptr = &ctrl;

    // 3、搭建首页，告诉httplib服务器使用指定的目录作为静态文件服务的根目录，在找不到任何对应路由请求的情况下，使用这个目录下的文件作为响应（即首页文件）
    svr.set_base_dir("/home/zzy1/2024/my-online-judge/MYOJ/OJ_server/wwwroot");

    // 3.1 获取所有题目的列表
    svr.Get("/all_questions", [&ctrl](const httplib::Request &req, httplib::Response &resp)
            { 
                std::string html;
                ctrl.allQuestions(&html);
                resp.set_content(html, "text/html; charset=utf-8"); });

    // 3.2 用户提供题目编号来获取题目内容
    svr.Get(R"(/question/(\d+))", [&ctrl](const httplib::Request &req, httplib::Response &resp)
            {
                std::string number = req.matches[1];     //matches[1]中储存着正则匹配到的字符
                std::string html;
                ctrl.oneQuestion(number, &html);
                resp.set_content(html, "text/html; charset=utf-8"); });

    // 3.3 用户提交代码，使用我们的判题功能
    // 尽管POST请求的参数是在正文当中的，但是调用函数的时候还是要根据URL路径来调用（因为HTTP 服务器的核心功能之一是根据客户端请求的 URL 路径来决定如何处理请求。路由机制确保了不同类型的请求可以被正确地路由到相应的处理函数。）
    svr.Post(R"(/judge/(\d+))", [&ctrl](const httplib::Request &req, httplib::Response &resp)
             {   
                std::string number = req.matches[1];
                std::string result_json;
                ctrl.Judge(number, req.body /* POST请求的参数，存放在正文当中 */, &result_json);
                resp.set_content(result_json, "application/json;charset=utf-8"); });

    svr.listen("0.0.0.0", 8081);
    return 0;
}