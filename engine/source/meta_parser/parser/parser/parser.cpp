#include "common/precompiled.h"

#include "language_types/class.h"

#include "generator/reflection_generator.h"
#include "generator/serializer_generator.h"

#include "parser.h"

// 递归进入 namespace节点 中，继续分析里面的类
#define RECURSE_NAMESPACES(kind, cursor, method, namespaces) \
    { \
        if (kind == CXCursor_Namespace) \
        { \
            auto display_name = cursor.getDisplayName(); \
            if (!display_name.empty()) \
            { \
                namespaces.emplace_back(display_name); \
                method(cursor, namespaces); \
                namespaces.pop_back(); \
            } \
        } \
    }

// 如果某个类需要编译，则把它加入当前的 schema 模块，同时记录到全局注册表
#define TRY_ADD_LANGUAGE_TYPE(handle, container) \
    { \
        if (handle->shouldCompile()) \
        { \
            auto file = handle->getSourceFile(); \
            m_schema_modules[file].container.emplace_back(handle); \
            m_type_table[handle->m_display_name] = file; \
        } \
    }

void MetaParser::prepare(void) {}

// 查找 include 的文件
std::string MetaParser::getIncludeFile(std::string name)
{
    auto iter = m_type_table.find(name);
    return iter == m_type_table.end() ? std::string() : iter->second;
}

MetaParser::MetaParser(const std::string project_input_file,
                       const std::string include_file_path,
                       const std::string include_path,
                       const std::string sys_include,
                       const std::string module_name,
                       bool              is_show_errors) :
    m_project_input_file(project_input_file),
    m_source_include_file_name(include_file_path), m_index(nullptr), m_translation_unit(nullptr),
    m_sys_include(sys_include), m_module_name(module_name), m_is_show_errors(is_show_errors)
{
    m_work_paths = Utils::split(include_path, ";");  // 拿到所有 include 路径

    // 进行 序列化和反射
    m_generators.emplace_back(new Generator::SerializerGenerator(
        m_work_paths[0], std::bind(&MetaParser::getIncludeFile, this, std::placeholders::_1)));
    m_generators.emplace_back(new Generator::ReflectionGenerator(
        m_work_paths[0], std::bind(&MetaParser::getIncludeFile, this, std::placeholders::_1)));
}

MetaParser::~MetaParser(void)
{
    for (auto item : m_generators)
    {
        delete item;
    }
    m_generators.clear();

    if (m_translation_unit)
        clang_disposeTranslationUnit(m_translation_unit);

    if (m_index)
        clang_disposeIndex(m_index);
}

void MetaParser::finish(void)
{
    for (auto generator_iter : m_generators)
    {
        generator_iter->finish();
    }
}

bool MetaParser::parseProject()
{
    bool result = true;
    std::cout << "Parsing project file: " << m_project_input_file << std::endl;

    std::fstream include_txt_file(m_project_input_file, std::ios::in);

    if (include_txt_file.fail())
    {
        std::cout << "Could not load file: " << m_project_input_file << std::endl;
        return false;
    }

    // 把整个文件内容读成一个字符串 context
    std::stringstream buffer;
    buffer << include_txt_file.rdbuf();

    std::string context = buffer.str();

    auto         inlcude_files = Utils::split(context, ";");
    std::fstream include_file;

    include_file.open(m_source_include_file_name, std::ios::out);
    if (!include_file.is_open())
    {
        std::cout << "Could not open the Source Include file: " << m_source_include_file_name << std::endl;
        return false;
    }

    std::cout << "Generating the Source Include file: " << m_source_include_file_name << std::endl;

    std::string output_filename = Utils::getFileName(m_source_include_file_name);

    if (output_filename.empty())
    {
        output_filename = "META_INPUT_HEADER_H";
    }
    else
    {
        Utils::replace(output_filename, ".", "_");
        Utils::replace(output_filename, " ", "_");
        Utils::toUpper(output_filename);
    }
    include_file << "#ifndef __" << output_filename << "__" << std::endl;
    include_file << "#define __" << output_filename << "__" << std::endl;

    for (auto include_item : inlcude_files)
    {
        std::string temp_string(include_item);
        Utils::replace(temp_string, '\\', '/');
        include_file << "#include  \"" << temp_string << "\"" << std::endl;
    }

    include_file << "#endif" << std::endl;
    include_file.close();
    return result;
}

int MetaParser::parse(void)
{   
    // 生成临时的聚合 include 头文件
    bool parse_include_ = parseProject();
    if (!parse_include_)
    {
        std::cerr << "Parsing project file error! " << std::endl;
        return -1;
    }

    std::cerr << "Parsing the whole project..." << std::endl;
    int is_show_errors      = m_is_show_errors ? 1 : 0;
    m_index                 = clang_createIndex(true, is_show_errors);  // 创建一个 Clang Index
    std::string pre_include = "-I";
    std::string sys_include_temp;
    if (!(m_sys_include == "*"))
    {
        sys_include_temp = pre_include + m_sys_include;  // 加上系统的 include 路径
        arguments.emplace_back(sys_include_temp.c_str());
    }

    auto paths = m_work_paths;
    for (int index = 0; index < paths.size(); ++index)
    {
        paths[index] = pre_include + paths[index];

        arguments.emplace_back(paths[index].c_str());
    }

    fs::path input_path(m_source_include_file_name);
    if (!fs::exists(input_path))
    {
        std::cerr << input_path << " is not exist" << std::endl;
        return -2;
    }

    // 用之前准备好的 include 参数，创建一个编译单元
    m_translation_unit = clang_createTranslationUnitFromSourceFile(
        m_index, m_source_include_file_name.c_str(), static_cast<int>(arguments.size()), arguments.data(), 0, nullptr);
    auto cursor = clang_getTranslationUnitCursor(m_translation_unit);  // cursor：TU 根节点

    Namespace temp_namespace;

    // 递归遍历 AST，找到类/结构体的定义，构造 Class 对象，并存储到 m_schema_modules
    buildClassAST(cursor, temp_namespace);

    temp_namespace.clear();

    return 0;
}

void MetaParser::generateFiles(void)
{
    std::cerr << "Start generate runtime schemas(" << m_schema_modules.size() << ")..." << std::endl;
    for (auto& schema : m_schema_modules)
    {
        for (auto& generator_iter : m_generators)
        {
            generator_iter->generate(schema.first, schema.second);
        }
    }

    finish();
}

void MetaParser::buildClassAST(const Cursor& cursor, Namespace& current_namespace)
{
    for (auto& child : cursor.getChildren())
    {
        auto kind = child.getKind();

        // 如果是类或者结构体的定义
        if (child.isDefinition() && (kind == CXCursor_ClassDecl || kind == CXCursor_StructDecl))
        {
            // 创建一个 Class 对象，封装 Clang 的 AST 节点 + 命名空间信息
            auto class_ptr = std::make_shared<Class>(child, current_namespace);

            // 如果这个类符合条件（带反射宏 / 注解），加入 schema
            TRY_ADD_LANGUAGE_TYPE(class_ptr, classes);
        }
        else
        {
            RECURSE_NAMESPACES(kind, child, buildClassAST, current_namespace);
        }
    }
}
