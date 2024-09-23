// 该模块负责和数据交互，对题库进行增删改查，对外提供访问数据接口
// MySQL版本题库
#pragma once

#include <mysql/mysql.h>
#include <vector>

#include "../comm/util.hpp"

namespace ns_model_MySQL
{
    struct Question
    {
        std::string number; // 题目的唯一编号
        std::string title;  // 题目的标题
        std::string star;   // 题目的难度——简单/中等/困难
        int cpu_limit;      // 题目的时间限制（s)
        int mem_limit;      // 题目的空间限制（KB）
        std::string desc;   // 题目的题干描述
        std::string header; // 题目预设给用户在线编辑器的代码
        std::string tail;   // 题目的测试用例，需要和header拼接，形成完整代码
    };

    // MySQL中的表名
    const std::string table_name = "oj_questions";

    // IP地址
    const std::string host = "127.0.0.1";

    // MySQL用户
    const std::string user = "oj_client";

    // 用户密码
    const std::string passwd = "ZZYcxrmyh777";

    // 数据库名称
    const std::string db = "oj";

    // MySQL服务端口号
    const uint16_t port = 8080;

    class Model
    {
    public:
        Model()
        {
        }
        ~Model()
        {
        }

        // 查询数据库的接口
        // const std::string& query 为要执行的sql语句
        // std::vector<Question>* out 为输出型参数，存放查询的结果
        bool queryMySQL(const std::string &query, std::vector<Question> *out)
        {
            // 1.创建MySQL句柄
            MYSQL *my = mysql_init(nullptr);

            // 2.连接数据库
            if (nullptr ==
                mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(), db.c_str(), port,
                                   nullptr, 0))
            {
                lg(Fatal, "数据库连接失败, errno: %d, strerror: %s\n", errno, strerror(errno));
                return false;
            }

            // 设置字符集，否则传输中文会乱码
            mysql_set_character_set(my, "utf8");

            lg(Info, "数据库连接成功!\n");

            // 3.下发SQL命令
            int ret = mysql_query(my, query.c_str());
            if (0 != ret)
            {
                lg(Error, "%s, \"sql\"语句执行失败, ret: %d, mysql_error: %s\n",
                   query.c_str(), ret, mysql_error(my));
                return false;
            }

            // 4.提取结果
            MYSQL_RES *res = mysql_store_result(my);

            // 5.分析结果
            int rows = mysql_num_rows(res);   // 获得行数量
            int cols = mysql_num_fields(res); // 获得列数量

            Question q;

            for (int i = 0; i < rows; i++)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                q.number = row[0];
                q.title = row[1];
                q.star = row[2];
                q.desc = row[3];
                q.header = row[4];
                q.tail = row[5];
                q.cpu_limit = atoi(row[6]);
                q.mem_limit = atoi(row[7]);

                out->push_back(q);
            }

            // 6.释放资源
            mysql_free_result(res);

            // 关闭MySQL连接
            mysql_close(my);
            return true;
        }

        bool getAllQuestions(std::vector<Question> *out)
        {
            std::string sql = "select * from ";
            sql += table_name;
            sql += ";";

            return queryMySQL(sql, out);
        }

        bool getOneQuestion(const std::string &question_num, Question *out)
        {
            bool res = false;
            std::string sql = "select * from ";
            sql += table_name;
            sql += " where number=";
            sql += question_num;
            sql += ";";
            std::vector<Question> result;

            if (queryMySQL(sql, &result))
            {
                if (result.size() == 1)
                {
                    *out = result[0];
                    res = true;
                }
            }

            return res;
        }
    };

}
