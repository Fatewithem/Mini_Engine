#pragma once

#include "type_info.h"

#include "field.h"
#include "method.h"

struct BaseClass
{
    BaseClass(const Cursor& cursor);

    std::string name;
};

class Class : public TypeInfo
{
    // to access m_qualifiedName
    friend class Field;
    friend class Method;
    friend class MetaParser;

public:
    Class(const Cursor& cursor, const Namespace& current_namespace);

    virtual bool shouldCompile(void) const;

    // 判断成员变量是否需要反射
    bool shouldCompileFields(void) const;
    bool shouldCompileMethods(void) const;

    template<typename T>
    using SharedPtrVector = std::vector<std::shared_ptr<T>>;

    std::string getClassName(void);

    SharedPtrVector<BaseClass> m_base_classes;

public:
    std::string m_name;                 // 类名

    std::string m_qualified_name;       // 带命名空间完整类名

    SharedPtrVector<Field> m_fields;    // 保存类成员变量
    SharedPtrVector<Method> m_methods;  // 保存类成员方法

    std::string m_display_name;         // 最终展示名字（去掉首个m）

    bool isAccessible(void) const;
};
