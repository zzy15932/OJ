#pragma once

#include <atomic>
#include <mutex>

#include "../comm/util.hpp"

namespace ns_control
{
    class Machine
    {
    public:
        Machine(const std::string &IP, const uint16_t port)
            : _IP(IP), _port(port), a_loads(0)
        {
        }
        ~Machine()
        {
        }

        void inLoad()
        {
            a_loads++;
        }

        void deLoad()
        {
            a_loads--;
        }

        void resetLoad()
        {
            a_loads = 0;
        }

        uint32_t getLoad()
        {
            return a_loads;
        }

    private:
        std::string _IP;               // 主机IP地址
        uint16_t _port;                // 主机端口号
        std::atomic<uint32_t> a_loads; // 负载量，原子类型，保证线程安全
    };

    const std::string conf_path = "./service_machine.conf";

    class LoadBalance
    {
    public:
        LoadBalance()
        {
            // 导入配置文件
            if (!loadConf())
            {
                // 导入失败,差错问题以在loadConf中说明,此处直接退出
                exit(1);
            }
            lg(Info, "加载配置文件%s成功!\n", conf_path.c_str());
        }

        ~LoadBalance()
        {
        }

        bool loadConf()
        {
            // 1.打开文件流
            std::ifstream in(conf_path);
            if (!in.is_open())
            {
                // 打开失败的情况
                lg(Error, "加载配置文件%s失败, 请检查! errno: %d, strerror: %s\n", conf_path.c_str(), errno, strerror(errno));
                return false;
            }

            std::string line;                   // 从文件流中读取缓冲区
            std::vector<std::string> tokens;    // 分割字符串后的结果缓冲区
            
            while (getline(in, line))
            {
                ns_util::splitUtil::splitString(line, ":", &tokens);
                if (tokens.size() != 2)
                {
                    lg(Error, "分割字符串失败, errno: %d, strerror: %s\n", errno, strerror(errno));
                    return false;
                }

                Machine machine(tokens[0], stoi(tokens[1]));

                // 先将配置文件中的所有主机按照上线处理
                _onlines.push_back(_machines.size());
                _machines.push_back(std::move(machine));
            }

            // 从文件流中读取完毕，回收资源
            in.close();
            return true;
        }

        // 两个输出型参数，分别是主机的ID号，和描述主机的二级指针，以及返回是否选择成功
        bool smartChoice(int* num, Machine** ppmachine)
        {
            // 1. 判断是否还有主机在线
            _mutex.lock();
            if (_onlines.size() == 0)
            {
                // 所有主机均已下线
                lg(Fatal, "所有主机均已下线, 请运维人员检查主机状况!\n");
                return false;
            }

            // 2.遍历所有在线的主机表,找到负载最小的主机号
            int min_num = _onlines[0];                          // 用来存储负载最小的主机的编号
            uint32_t min_load = _machines[min_num].getLoad();   // 用来存储负载最小的主机的负载量

            for (int i = 1; i < _onlines.size(); i++)
            {
                int tmp_num = _onlines[i];
                uint32_t tmp_load = _machines[tmp_num].getLoad();
                if (min_load > tmp_load)
                {
                    min_load = tmp_load;
                    min_num = tmp_num;
                }
            }


            _mutex.unlock();

            *num = min_num;
            *ppmachine = &_machines[min_num];
            return true;
        }

        // 上线所有主机
        void onlineMachine()
        {
            _mutex.lock();
            _onlines.insert(_onlines.end(), _offlines.begin(), _offlines.end());
            _offlines.clear();
            _mutex.unlock();
        }
        
    private:
        std::vector<Machine> _machines; // 存储所有的主机，每个主机的下标就是天然的主机编号
        std::vector<int> _onlines;      // 记录所有上线状态的主机编号
        std::vector<int> _offlines;      // 记录所有下线状态的主机编号
        std::mutex _mutex;              // 锁，由于STL不保证线程安全，需要加锁保护
    };

}