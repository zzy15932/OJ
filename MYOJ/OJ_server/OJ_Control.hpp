#pragma once

#include <atomic>
#include <mutex>
#include <assert.h>
#include <jsoncpp/json/json.h>

#include "OJ_view.hpp"
#include "../comm/Comm_model_MySQL.hpp"
#include "../comm/util.hpp"
#include "../comm/httplib.h"

namespace ns_control
{
    const std::string http_pattern = "/compile_and_run";

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
            a_loads.fetch_add(1, std::memory_order_relaxed);
        }

        void deLoad()
        {
            a_loads.fetch_sub(1, std::memory_order_relaxed);
        }

        void resetLoad()
        {
            a_loads.store(0, std::memory_order_relaxed);
        }

        uint32_t getLoad() const
        {
            return a_loads.load(std::memory_order_relaxed);
        }

        std::string getIP() const
        {
            return _IP;
        }

        uint16_t getPort() const
        {
            return _port;
        }

        Machine(const Machine &) = delete;
        Machine &operator=(const Machine &) = delete;

        Machine(Machine &&other) noexcept
            : _IP(std::move(other._IP)), _port(other._port), a_loads(other.a_loads.load())
        {
        }

        // 移动赋值运算符
        Machine &operator=(Machine &&other) noexcept
        {
            if (this != &other)
            {
                _IP = std::move(other._IP);
                _port = other._port;
                a_loads.store(other.a_loads.load());
                other.a_loads.store(0);
            }
            return *this;
        }

    private:
        std::string _IP;          // 主机IP地址
        uint16_t _port;           // 主机端口号
        std::atomic_uint a_loads; // 负载量，原子类型，保证线程安全
    };

    const std::string conf_path = "/home/zzy1/2024/my-online-judge/MYOJ/OJ_server/service_machine.conf";

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

            std::string line;                // 从文件流中读取缓冲区
            std::vector<std::string> tokens; // 分割字符串后的结果缓冲区

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
        bool smartChoice(int *num, Machine **ppmachine)
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
            int min_num = _onlines[0];                        // 用来存储负载最小的主机的编号
            uint32_t min_load = _machines[min_num].getLoad(); // 用来存储负载最小的主机的负载量

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

        // 展示所有主机，分别将离线主机，在线主机打印出来，主要debug使用
        void showMachine()
        {
            std::cout << "在线主机: ";
            for (int x : _onlines)
            {
                std::cout << x << " ";
            }
            std::cout << std::endl;

            std::cout << "离线主机: ";
            for (int x : _offlines)
            {
                std::cout << x << " ";
            }
            std::cout << std::endl;
        }

        // 下线指定主机
        void offlineMachine(int host_id)
        {
            _mutex.lock();
            if (_machines.size() <= host_id)
            {
                lg(Warning, "要下线的主机: %d 不存在!\n", host_id);
                return;
            }

            for (auto it = _onlines.begin(); it != _onlines.end(); it++)
            {
                if (*it == host_id)
                {
                    // 由于直接break，所以不需要考虑迭代器失效的问题
                    _onlines.erase(it);
                    _offlines.push_back(host_id);
                    break;
                }
            }

            _mutex.unlock();
        }

    private:
        std::vector<Machine> _machines; // 存储所有的主机，每个主机的下标就是天然的主机编号
        std::vector<int> _onlines;      // 记录所有上线状态的主机编号
        std::vector<int> _offlines;     // 记录所有下线状态的主机编号
        std::mutex _mutex;              // 锁，由于STL不保证线程安全，需要加锁保护
    };

    // 核心控制器
    class Control
    {
    public:
        Control()
        {
        }
        ~Control()
        {
        }

        // 将所有主机恢复为在线状态
        void recoveryMachine()
        {
            _lb.onlineMachine();
        }

        // 获取所有问题
        bool allQuestions(std::string *html)
        {
            std::vector<ns_model_MySQL::Question> questions;

            if (!_model.getAllQuestions(&questions))
            {
                lg(Error, "获取所有题目失败, 无法构成网页!\n");
                *html = "获取所有题目失败, 无法构成网页!";
                return false;
            }

            std::sort(questions.begin(), questions.end(),
                      [](const ns_model_MySQL::Question &q1, const ns_model_MySQL::Question &q2)
                      {
                          int num1 = std::stoi(q1.number);
                          int num2 = std::stoi(q2.number);
                          assert(!(num1 == num2));

                          return (num1 < num2);
                      });

            _view.AllExpandHtml(questions, html);

            return true;
        }

        bool oneQuestion(const std::string &num, std::string *html)
        {
            ns_model_MySQL::Question q;

            if (!_model.getOneQuestion(num, &q))
            {
                lg(Error, "获取题号%s的题目信息失败, 无法构建网页!\n", num.c_str());
                *html = "获取单个题目失败,无法构成网页!";
                return false;
            }

            _view.OneExpandHtml(q, html);
            return true;
        }

        void Judge(const std::string &num, const std::string &in_json, std::string *out_json)
        {
            // 1.根据题号,拿到相应的题目信息
            ns_model_MySQL::Question q;
            _model.getOneQuestion(num, &q);

            // 2.对题目信息反序列化,保存到json串中,其中in_json中的主要信息有code和input
            Json::Value in_value;
            Json::Reader reader;
            reader.parse(in_json, in_value);

            // 3.构建compile_server所需要的json串
            Json::Value compile_value;
            std::string code = in_value["code"].asString();
            code += "\n";
            code += q.tail;

            compile_value["code"] = code;
            compile_value["input"] = in_value["input"].asString();
            compile_value["cpu_limit"] = q.cpu_limit;
            compile_value["mem_limit"] = q.mem_limit;

            Json::FastWriter writer;
            std::string compile_string = writer.write(compile_value);

            // 4.选择负载均衡最低的主机
            // 注意：并不是一寻找就能找到的，可能找到的主机已经挂掉了
            // 这个时候就要将挂掉的主机放入offlines中
            // 然后再去寻找新的主机
            // 这样结果要么就直接找到，要么就全部挂掉
            int host_id = 0;
            while (true)
            {
                Machine *pmachine = nullptr;
                if (!_lb.smartChoice(&host_id, &pmachine))
                {
                    // 主机全挂了
                    break;
                }

                // 选到了一个主机
                // 5. 向主机发送http请求,得到结果,通过返回的状态码判断主机是否还在线
                httplib::Client cli(pmachine->getIP(), pmachine->getPort());

                // 发送post请求，并增加负载
                pmachine->inLoad();

                lg(Info, "选择主机id: %d, ip: %s, port: %d, load: %d 尝试进行编译运行服务...\n",
                   host_id, pmachine->getIP().c_str(), pmachine->getPort(), pmachine->getLoad());
                // std::cout << "发起判题请求" << http_pattern.c_str() << std::endl;
                httplib::Result res = cli.Post(http_pattern.c_str(), compile_string, "application/json;charset=utf-8");
                // httplib::Result 类型使httplib库中的定义的类，其中重载了->操作符
                // 可以用于获取response中的各种信息

                // 判断结果有效，并且状态码为200才说明是成功的
                if (res && res->status == 200)
                {
                    *out_json = res->body;
                    // 请求完毕，减少负载
                    pmachine->deLoad();
                    lg(Info, "编译运行服务完成...\n");
                    break;
                }
                else
                {
                    // 失败，需要重新请求，并且将此次选择的主机放到下线表中
                    lg(Warning, "本次请求的主机id: %d, 请求失败, ip: %s, port: %d\n",
                       host_id, pmachine->getIP().c_str(), pmachine->getPort());
                    pmachine->resetLoad();
                    _lb.offlineMachine(host_id);
                    _lb.showMachine(); // for debug
                }
            }
        }

    private:
        ns_view::View _view;
        ns_model_MySQL::Model _model;
        LoadBalance _lb;
    };

}