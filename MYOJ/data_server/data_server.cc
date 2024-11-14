#include <jsoncpp/json/json.h>
#include <ctemplate/template.h>

#include "../comm/httplib.h"
#include "../comm/Comm_model_MySQL.hpp"
#include "../comm/Log.hpp"

const std::string conf_file_path = "/home/zzy1/2024/my-online-judge/MYOJ/data_server/config.conf";
const std::string passwd_call_word = "password=";

// 浏览器不会自动识别普通的换行符
// 需要手动进行转换
// 要在 HTML 页面上正确显示预设代码（header）和测试用例（tail）的换行符
// 需要将换行符（\n）转换为 HTML 中的 <br> 标签
void replaceNewlinesWithBR(std::string &str)
{
    size_t pos = 0;
    // 查找换行符的位置，并用 <br> 替换
    while ((pos = str.find('\n', pos)) != std::string::npos)
    {
        str.replace(pos, 1, "<br>");
        pos += 4; // 移动到替换后的下一个位置
    }
}

bool read_password_from_conf(std::string *password)
{
    if (password == nullptr)
    {
        return false;
    }

    password->clear();
    std::ifstream file(conf_file_path);
    if (!file.is_open())
    {
        // std::cerr << "无法打开配置文件: " << file_path << std::endl;
        lg(Fatal, "无法打开 %s 配置文件, 请检查!\n", conf_file_path.c_str());
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.find(passwd_call_word) == 0)
        {
            *password = line.substr(passwd_call_word.size()); // 截取 "admin_password=" 之后的部分
            break;
        }
    }

    file.close();
    return true;
}

int main(int argc, char *argv[])
{
    if (argc == 2)
    {
        std::string option(argv[1]);
        if (option == "-d")
        {
            // 1、守护进程化
            if (daemon(1, 0) == -1)
            {
                perror("data_server 守护进程化失败！\n");
                exit(1);
            }
        }
    }

    std::string stored_password;
    if (!read_password_from_conf(&stored_password))
    {
        exit(2);
    }

    lg(Info, "加载配置文件 %s 成功\n", conf_file_path.c_str());

    httplib::Server svr;
    ns_model_MySQL::Model model;

    svr.set_base_dir("/home/zzy1/2024/my-online-judge/MYOJ/data_server/wwwdata");

    // 处理密码验证请求
    svr.Post("/verify-password",
             [&stored_password](const httplib::Request &req, httplib::Response &res)
             {
                Json::Value requestData;
                Json::Reader reader;
                if (reader.parse(req.body, requestData)) 
                {
                    std::string password = requestData["password"].asString();

                    Json::Value responseData;
                    if (password == stored_password) 
                    {
                        responseData["success"] = true;
                    } 
                    else 
                    {
                        responseData["success"] = false;
                    }
                res.set_content(responseData.toStyledString(), "application/json");
                } 
                else 
                {
                    res.status = 400;
                    res.set_content("Invalid JSON", "text/plain");
                } });

    // 管理界面路由
    svr.Get("/manage",
            [](const httplib::Request &, httplib::Response &res)
            {
                // 这里可以返回题库管理的界面（前端HTML页面）
                std::string html;
                ns_util::fileUtil::readFile("/home/zzy1/2024/my-online-judge/MYOJ/data_server/wwwdata/manage.html", &html, true);
                res.set_content(html, "text/html");
            });

    // 处理查询指定题目的请求
    svr.Get(R"(/manage/questions/(\d+))",
            [&](const httplib::Request &req, httplib::Response &res)
            {
                std::string question_num = req.matches[1]; // 获取题目编号
                ns_model_MySQL::Question question;

                if (model.getOneQuestion(question_num, &question))
                {
                    replaceNewlinesWithBR(question.desc);
                    replaceNewlinesWithBR(question.header);
                    replaceNewlinesWithBR(question.tail);

                    Json::Value json_question;
                    json_question["number"] = question.number;
                    json_question["title"] = question.title;
                    json_question["star"] = question.star;
                    json_question["cpu_limit"] = question.cpu_limit;
                    json_question["mem_limit"] = question.mem_limit;
                    json_question["desc"] = question.desc;
                    json_question["header"] = question.header;
                    json_question["tail"] = question.tail;

                    Json::StreamWriterBuilder writer;
                    std::string output = Json::writeString(writer, json_question);
                    res.set_content(output, "application/json");
                    lg(Info, "查找单个题目操作成功\n");
                }
                else
                {
                    res.status = 404;
                    res.set_content("Question not found", "text/plain");
                    lg(Warning, "查找单个题目操作失败\n");
                }
            });

    // 添加题目
    svr.Post("/manage/questions",
             [&](const httplib::Request &req, httplib::Response &res)
             {
                 Json::CharReaderBuilder reader;
                 Json::Value json_question;
                 std::string errs;

                 std::unique_ptr<Json::CharReader> const jsonReader(reader.newCharReader());
                 //  std::cout << "收到的请求body: " << req.body << std::endl; debug

                 auto json = jsonReader->parse(req.body.c_str(),
                                               req.body.c_str() + req.body.length(),
                                               &json_question, &errs);

                 if (!jsonReader->parse(req.body.c_str(), req.body.c_str() + req.body.length(), &json_question, &errs))
                 {

                     res.status = 400;
                     res.set_content("Invalid JSON format", "text/plain");
                     //  lg(Warning, "parse添加题目失败\n");
                     // std::cout << "1111" << std::endl;
                     return;
                 }

                 // 获取前端传入的字段
                 std::string title = json_question["title"].asString();
                 std::string star = json_question["star"].asString();
                 int cpu_limit = json_question["cpu_limit"].asInt();
                 int mem_limit = json_question["mem_limit"].asInt();
                 std::string desc = json_question["desc"].asString();
                 std::string header = json_question["header"].asString();
                 std::string tail = json_question["tail"].asString();

                 std::string sql;

                 if (!json_question.isMember("number"))
                 {
                     sql = "INSERT INTO " + ns_model_MySQL::table_name + " (title, star, cpu_limit, mem_limit, `desc`, header, tail) VALUES ('";
                     sql += title + "', '";
                     sql += star + "', ";
                     sql += std::to_string(cpu_limit) + ", ";
                     sql += std::to_string(mem_limit) + ", '";
                     sql += desc + "', '";
                     sql += header + "', '";
                     sql += tail + "');";
                 }
                 else
                 {
                     int number = json_question["number"].asInt();
                     // 自定义题号插入SQL
                     sql = "INSERT INTO " + ns_model_MySQL::table_name + " (number, title, star, cpu_limit, mem_limit, `desc`, header, tail) VALUES (";
                     sql += std::to_string(number) + ", '";
                     sql += title + "', '";
                     sql += star + "', ";
                     sql += std::to_string(cpu_limit) + ", ";
                     sql += std::to_string(mem_limit) + ", '";
                     sql += desc + "', '";
                     sql += header + "', '";
                     sql += tail + "');";
                 }

                 //  lg(Debug, "sql finish\n");

                 if (model.queryMySQL(sql, nullptr))
                 {
                     //  lg(Debug, "sql query finish\n");
                     res.status = 200;
                     res.set_content("Question added successfully", "text/plain");
                     lg(Info, "添加题目成功\n");
                 }
                 else
                 {
                     //  lg(Debug, "sql query finish\n");
                     res.status = 500;
                     res.set_content("Failed to add question", "text/plain");
                     lg(Warning, "添加题目失败\n");
                 }
             });

    // 删除题目
    svr.Delete(R"(/manage/questions/(\d+))",
               [&](const httplib::Request &req, httplib::Response &res)
               {
                   std::string question_num = req.matches[1]; // 获取题目编号

                   // 删除 SQL 语句
                   std::string sql = "DELETE FROM " + ns_model_MySQL::table_name + " WHERE number = " + question_num + ";";

                   if (model.queryMySQL(sql, nullptr))
                   {
                       res.status = 200;
                       res.set_content("Question deleted successfully", "text/plain");
                       lg(Info, "题目删除操作成功\n");
                   }
                   else
                   {
                       res.status = 500;
                       res.set_content("Failed to delete question", "text/plain");
                       lg(Warning, "题目删除操作失败\n");
                   }
               });

    lg(Info, "题库管理服务启动成功\n");
    svr.listen("0.0.0.0", 9999);
    return 0;
}
