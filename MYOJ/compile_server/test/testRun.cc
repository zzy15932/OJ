#include "run.hpp"

int main()
{
    // ns_Runner::Runner runner;
    // runner.Run("hello", 5, 1024);
    // 1. exe文件能顺利执行
    // 2. exe文件不能顺利执行，如：超时，除0等情况
    ns_run::Runner runner;
    runner.run("hello", 1, 1024 * 1024 * 1024);

    return 0;
}