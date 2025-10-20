#pragma once

#include <memory>
#include <string>
#include <vector>

namespace Momo
{
    class EditorFileNode;
    using EditorFileNodeArray = std::vector<std::shared_ptr<EditorFileNode>>;

    struct EditorFileNode
    {
        std::string         m_file_name;    // 节点名
        std::string         m_file_type;    // 类型
        std::string         m_file_path;    // 路径
        int                 m_node_depth;   // 深度
        EditorFileNodeArray m_child_nodes;  // 子节点列表
        EditorFileNode() = default;
        EditorFileNode(const std::string& name, const std::string& type, const std::string& path, int depth) :
            m_file_name(name), m_file_type(type), m_file_path(path), m_node_depth(depth)
        {}
    };

    class EditorFileService
    {
        EditorFileNodeArray m_file_node_array;  // 数组存储节点
        // Will be set dynamically from ConfigManager::getAssetFolder() during buildEngineFileTree()
        EditorFileNode      m_root_node{};

    private:
        EditorFileNode* getParentNodePtr(EditorFileNode* file_node);
        bool            checkFileArray(EditorFileNode* file_node);

    public:
        EditorFileNode* getEditorRootNode() { return m_file_node_array.empty() ? nullptr : m_file_node_array[0].get(); }

        void buildEngineFileTree();
    };
} // namespace Momo
