#include <jsoncpp/json/json.h>
#include "comm/httplib.h"

int main()
{
    httplib::Client cl("127.0.0.1", 8082);

    std::string compile_json;

    Json::Value value_in;
    Json::StyledWriter writer;

    value_in["code"] = R"(#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;

class Solution
{
public:
    bool isPalindrome(int x)
    {
        if (x < 0)
        {
            return false; // 负数不可能是回文数
        }

        string str1 = to_string(x);
        string str2(str1);
        reverse(str2.begin(), str2.end());

        for (int i = 0; i < str1.size() / 2; i++)
        {
            if (str2[i] != str1[i])
            {
                return false;
            }
        }

        return true;
    }
};

void Test1()
{
    // 通过定义临时对象，来完成方法的调用
    bool ret = Solution().isPalindrome(121);
    if (ret)
    {
        std::cout << "通过用例1, 测试121通过 ... OK!" << std::endl;
    }
    else
    {
        std::cout << "没有通过用例1, 测试的值是: 121" << std::endl;
    }
}

void Test2()
{
    // 通过定义临时对象，来完成方法的调用
    bool ret = Solution().isPalindrome(-10);
    if (!ret)
    {
        std::cout << "通过用例2, 测试-10通过 ... OK!" << std::endl;
    }
    else
    {
        std::cout << "没有通过用例2, 测试的值是: -10" << std::endl;
    }
}

int main()
{
    Test1();
    Test2();

    return 0;
})";
    value_in["cpu_limit"] = 1;
    value_in["mem_limit"] = 1024 * 1024 * 1024;

    compile_json = writer.write(value_in);

    httplib::Result res = cl.Post("/compile_and_run", compile_json, "application/json;charset=utf-8");

    if (res && res->status == 200)
    {
        std::cout << "编译运行服务完成...\n" << std::endl;
        std::cout << res->body << std::endl;
    }
    else
    {
        // 失败，需要重新请求，并且将此次选择的主机放到下线表中
        std::cout << "编译运行服务失败...\n" << std::endl;
    }

    return 0;
}
