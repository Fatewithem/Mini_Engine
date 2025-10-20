#pragma once
#include "common/namespace.h"

#include "cursor/cursor.h"

#include "meta/meta_info.h"
#include "parser/parser.h"

class TypeInfo
{
public:
    TypeInfo(const Cursor& cursor, const Namespace& current_namespace);
    virtual ~TypeInfo(void) {}

    const MetaInfo& getMetaData(void) const;

    std::string getSourceFile(void) const;

    Namespace getCurrentNamespace() const;

    Cursor& getCurosr();

protected:
    MetaInfo m_meta_data;    // 当前类型的元信息 

    bool m_enabled;

    std::string m_alias_cn;

    Namespace m_namespace;   // 命名空间

private:
    // AST 游标，表示这个类型在源代码里的定义位置
    Cursor m_root_cursor;
};