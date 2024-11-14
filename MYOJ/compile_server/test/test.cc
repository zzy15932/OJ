#include <iostream>
#include <unistd.h>
using namespace std;


int main()
{
    std::string _exe("./temp/1727077555456-1.exe");
    execlp(_exe.c_str(), _exe.c_str(), nullptr);
    return 0;
}