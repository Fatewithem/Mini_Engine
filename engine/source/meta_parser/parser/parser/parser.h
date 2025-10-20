#pragma once

#include "common/precompiled.h"

#include "common/namespace.h"
#include "common/schema_module.h"

#include "cursor/cursor.h"

#include "generator/generator.h"
#include "template_manager/template_manager.h"

class Class;

class MetaParser
{
public:
    static void prepare(void);

    MetaParser(const std::string project_input_file,
               const std::string include_file_path,
               const std::string include_path,
               const std::string include_sys,
               const std::string module_name,
               bool              is_show_errors);
    ~MetaParser(void);
    void finish(void);
    int  parse(void);
    void generateFiles(void);

private:
    std::string m_project_input_file;  // 项目输入文件

    std::vector<std::string> m_work_paths;                // include 搜索路径列表
    std::string              m_module_name;               // 模块名
    std::string              m_sys_include;               // 系统 inlcude 路径
    std::string              m_source_include_file_name;  // “聚合 include 头文件”的路径

    CXIndex           m_index;
    CXTranslationUnit m_translation_unit;

    std::unordered_map<std::string, std::string>  m_type_table;      // 类型名
    std::unordered_map<std::string, SchemaMoudle> m_schema_modules;  // schema

    std::vector<const char*>                    arguments = {{"-x",
                                           "c++",
                                           "-std=c++11",
                                           "-D__REFLECTION_PARSER__",
                                           "-DNDEBUG",
                                           "-D__clang__",
                                           "-w",
                                           "-MG",
                                           "-M",
                                           "-ferror-limit=0",
                                           "-o clangLog.txt"}};
    std::vector<Generator::GeneratorInterface*> m_generators;        // 序列化/反射

    bool m_is_show_errors;

private:
    bool        parseProject(void);
    void        buildClassAST(const Cursor& cursor, Namespace& current_namespace);
    std::string getIncludeFile(std::string name);
};
