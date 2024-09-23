#include <iostream>
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
        // 请将你的代码写在下面
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
// #ifndef COMPILER_ONLINE
// // #include "header.cpp"
// #endif

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
}