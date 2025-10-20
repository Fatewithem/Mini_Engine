#pragma once

#include "cursor/cursor.h"

class MetaInfo
{
public:
    MetaInfo(const Cursor& cursor);

    // 通过 string 来访问节点信息
    std::string getProperty(const std::string& key) const;

    // 查询是否存在 key
    bool getFlag(const std::string& key) const;

private:
    typedef std::pair<std::string, std::string> Property;

    std::unordered_map<std::string, std::string> m_properties;

private:
    std::vector<Property> extractProperties(const Cursor& cursor) const;
};