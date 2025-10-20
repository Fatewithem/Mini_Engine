#include "common/precompiled.h"

#include "class.h"
#include "method.h"

Method::Method(const Cursor& cursor, const Namespace& current_namespace, Class* parent) :
    TypeInfo(cursor, current_namespace), m_parent(parent), m_name(cursor.getSpelling())
{}

bool Method::shouldCompile(void) const { return isAccessible(); }

bool Method::isAccessible(void) const
{
    return (
              // 条件 1：父类允许方法反射
              (m_parent->m_meta_data.getFlag(NativeProperty::Methods) ||
               m_parent->m_meta_data.getFlag(NativeProperty::All)) &&
              // 并且当前方法没有被显式禁用
              !m_meta_data.getFlag(NativeProperty::Disable)
           )
           ||
           (
              // 条件 2：父类开启了白名单模式
              m_parent->m_meta_data.getFlag(NativeProperty::WhiteListMethods) &&
              // 并且当前方法被显式标记为启用
              m_meta_data.getFlag(NativeProperty::Enable)
           );
}
