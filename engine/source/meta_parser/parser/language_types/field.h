#pragma once

#include "type_info.h"

class Class;

class Field : public TypeInfo
{

public:
    Field(const Cursor& cursor, const Namespace& current_namespace, Class* parent = nullptr);

    virtual ~Field(void) {}

    bool shouldCompile(void) const;

public:
    bool m_is_const;

    Class* m_parent;             // 所属的类

    std::string m_name;          // 原始名字
    std::string m_display_name;  // 显示名字
    std::string m_type;          // 字段类型

    std::string m_default;

    bool isAccessible(void) const;
};