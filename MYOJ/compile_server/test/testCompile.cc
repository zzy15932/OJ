#include "compile.hpp"

int main()
{
    // 测试编译hello.cc文件
    // 1.hello.cc文件不存在
    // 2.hello.cc文件存在
    // 2.a hello.cc文件存在，且编译成功，生成.exe文件
    // 2.b hello.cc文件存在，但编译失败，没有生成.exe文件，并且有编译错误信息的文件产生

    ns_compile::Compiler cpl;
    if (cpl.compile("hello"))
    {
        lg(Debug, "编译成功\n");
    }
    else
    {
        lg(Debug, "编译失败\n");
    }

    return 0;
}