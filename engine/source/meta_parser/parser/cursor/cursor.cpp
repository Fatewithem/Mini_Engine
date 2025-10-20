#include "common/precompiled.h"
#include "meta/meta_utils.h"

#include "cursor.h"

// 保存传入的 CXCursor
Cursor::Cursor(const CXCursor& handle) : m_handle(handle) {}

// 获取 Cursor 的种类
CXCursorKind Cursor::getKind(void) const { return m_handle.kind; }

// 获取标识符的名字
std::string Cursor::getSpelling(void) const
{
    std::string spelling;

    Utils::toString(clang_getCursorSpelling(m_handle), spelling);

    return spelling;
}

// 获取显示名
std::string Cursor::getDisplayName(void) const
{
    std::string display_name;

    Utils::toString(clang_getCursorDisplayName(m_handle), display_name);

    return display_name;
}

// 获取该节点对应的源码文件路径
std::string Cursor::getSourceFile(void) const
{
    auto range = clang_Cursor_getSpellingNameRange(m_handle, 0, 0);

    auto start = clang_getRangeStart(range);

    CXFile   file;
    unsigned line, column, offset;

    clang_getFileLocation(start, &file, &line, &column, &offset);

    std::string filename;

    Utils::toString(clang_getFileName(file), filename);

    return filename;
}

// 判断是否是定义
bool Cursor::isDefinition(void) const { return clang_isCursorDefinition(m_handle); }

// 获取类型
CursorType Cursor::getType(void) const { return clang_getCursorType(m_handle); }

// 获取 子Cursor
Cursor::List Cursor::getChildren(void) const
{
    List children;

    auto visitor = [](CXCursor cursor, CXCursor parent, CXClientData data) {
        auto container = static_cast<List*>(data);

        container->emplace_back(cursor);

        if (cursor.kind == CXCursor_LastPreprocessing)
            return CXChildVisit_Break;

        return CXChildVisit_Continue;
    };

    clang_visitChildren(m_handle, visitor, &children);

    return children;
}

void Cursor::visitChildren(Visitor visitor, void* data) { clang_visitChildren(m_handle, visitor, data); }
