#include "compile_run.hpp"

int main()
{
    ns_compile_run::Compile_Run cr;

    Json::Value value_in;
    Json::StyledWriter writer;

    // in_json中包含:
    // 1、code：用户提交的代码
    // 2、input：用户给自己提交的代码所做的输入，不做处理
    // 3、cpu_limit：CPU运行时间限制
    // 4、mem_limit：内存限制，即空间限制

    // 测试：
    // 1.正常程序
    // value_in["code"] = "#include <iostream>\nint main(){    std::cout << \"hello world\" << std::endl;    return 0;}";
    // value_in["cpu_limit"] = 1;
    // value_in["mem_limit"] = 1024 * 1024 * 1024;

    // 2.空程序
    // value_in["code"] = "";
    // value_in["cpu_limit"] = 1;
    // value_in["mem_limit"] = 1024 * 1024 * 1024;

    // 3.编译错误
    // value_in["code"] = "dsadwadawdaw";
    // value_in["cpu_limit"] = 1;
    // value_in["mem_limit"] = 1024 * 1024 * 1024;

    // 4.CPU超时等异常错误
    // value_in["code"] = "#include <iostream>\n int main(){    while (true)    {        std::cout << \"hello zzy\" << std::endl;    }    return 0;}";
    // value_in["cpu_limit"] = 1;
    // value_in["mem_limit"] = 1024 * 1024 * 1024;

    value_in["code"] = "int main()\n {return 0;}";
    value_in["cpu_limit"] = 1;
    value_in["mem_limit"] = 1024 * 1024 * 1024;

    std::string in_json;
    in_json = writer.write(value_in);

    std::string out_json;
    cr.start(in_json, &out_json);

    std::cout << out_json << std::endl;

    return 0;
}