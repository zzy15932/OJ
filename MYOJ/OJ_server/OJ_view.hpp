// 通常是拿到数据之后，进行构建网页，渲染网络内容，展示给用户的（浏览器功能）
#pragma once

#include <iostream>
#include <string>
#include <ctemplate/template.h>

#include "OJ_model_MySQL.hpp"

namespace ns_view
{
    using namespace std;
    using namespace ns_model_MySQL;

    const string template_path = "./template_html/";

    class View
    {
    public:
        View()
        {
        }
        ~View()
        {
        }

    public:
        void AllExpandHtml(const vector<Question> &questions, string *html)
        {
            // 题目的编号 题目的标题 题目的难度
            // 推荐使用表格显示
            // 1. 形成路径
            string src_html = template_path + "all_questions.html";
            // 2. 形成数据字典
            ctemplate::TemplateDictionary root("all_questions");
            for (const auto &q : questions)
            {
                ctemplate::TemplateDictionary *sub = root.AddSectionDictionary("question_list");
                sub->SetValue("number", q.number);
                sub->SetValue("title", q.title);
                sub->SetValue("star", q.star);
            }

            // 3. 获取被渲染的html
            ctemplate::Template *tpl = ctemplate::Template::GetTemplate(src_html, ctemplate::DO_NOT_STRIP);

            // 4. 开始完成渲染功能
            tpl->Expand(html, &root);
        }

        void OneExpandHtml(const Question &q, string *html)
        {
            // 1. 形成路径
            string src_html = template_path + "one_question.html";

            // 2. 形成数据字典（不需要循环了，只有一个题目）
            ctemplate::TemplateDictionary root("one_question");
            root.SetValue("number", q.number);
            root.SetValue("title", q.title);
            root.SetValue("star", q.star);
            root.SetValue("desc", q.desc);
            root.SetValue("pre_code", q.header);

            // 3. 获取被渲染的html
            ctemplate::Template *tpl = ctemplate::Template::GetTemplate(src_html, ctemplate::DO_NOT_STRIP);

            // 4. 开始完成渲染功能
            tpl->Expand(html, &root);
        }
    };
}