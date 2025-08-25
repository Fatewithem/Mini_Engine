#include "editor/include/editor_file_service.h"

#include "runtime/platform/file_service/file_service.h"
#include "runtime/platform/path/path.h"

#include "runtime/resource/asset_manager/asset_manager.h"
#include "runtime/resource/config_manager/config_manager.h"

#include "runtime/function/global/global_context.h"

namespace Momo
{
    /// helper function: split the input string with separator, and filter the substring
    std::vector<std::string>
    splitString(std::string input_string, const std::string& separator, const std::string& filter_string = "")
    {
        std::vector<std::string> output_string;
        int                      pos = input_string.find(separator);
        std::string              add_string;

        while (pos != std::string::npos)
        {
            add_string = input_string.substr(0, pos);
            if (!add_string.empty())
            {
                if (!filter_string.empty() && add_string == filter_string)
                {
                    // filter substring
                }
                else
                {
                    output_string.push_back(add_string);
                }
            }
            input_string.erase(0, pos + 1);
            pos = input_string.find(separator);
        }
        add_string = input_string;
        if (!add_string.empty())
        {
            output_string.push_back(add_string);
        }
        return output_string;
    }

    void EditorFileService::buildEngineFileTree()
    {
        std::string                              asset_folder = g_runtime_global_context.m_config_manager->getAssetFolder().generic_string();

        // configure root node to match actual asset folder
        std::filesystem::path asset_path_fs(asset_folder);
        m_root_node.m_file_name = asset_path_fs.filename().generic_string();
        m_root_node.m_file_type = "Folder";
        m_root_node.m_file_path = asset_path_fs.generic_string();
        m_root_node.m_node_depth = -1;

        const std::vector<std::filesystem::path> file_paths = g_runtime_global_context.m_file_system->getFiles(asset_folder);
        std::vector<std::vector<std::string>>    all_file_segments;
        for (const auto& path : file_paths)
        {
            const std::filesystem::path& relative_path = Path::getRelativePath(asset_folder, path);
            all_file_segments.emplace_back(Path::getPathSegments(relative_path));
        }

        std::vector<std::shared_ptr<EditorFileNode>> node_array;

        m_file_node_array.clear();
        auto root_node = std::make_shared<EditorFileNode>();
        *root_node     = m_root_node;
        m_file_node_array.push_back(root_node);

        int all_file_segments_count = all_file_segments.size();
        for (int file_index = 0; file_index < all_file_segments_count; file_index++)
        {
            int depth = 0;
            node_array.clear();
            node_array.push_back(root_node);
            int file_segment_count = all_file_segments[file_index].size();

            std::filesystem::path current_path = asset_folder;

            for (int file_segment_index = 0; file_segment_index < file_segment_count; file_segment_index++)
            {
                auto file_node         = std::make_shared<EditorFileNode>();
                file_node->m_file_name = all_file_segments[file_index][file_segment_index];

                // advance current virtual path
                current_path /= file_node->m_file_name;

                if (depth < file_segment_count - 1)
                {
                    file_node->m_file_type = "Folder";
                    // for folders, also record the full path so we can uniquely identify nodes
                    file_node->m_file_path = current_path.generic_string();
                }
                else
                {
                    const auto& extensions = Path::getFileExtensions(file_paths[file_index]);
                    file_node->m_file_type = std::get<0>(extensions);
                    if (file_node->m_file_type.size() == 0)
                        continue;

                    if (file_node->m_file_type.compare(".json") == 0)
                    {
                        file_node->m_file_type = std::get<1>(extensions);
                        if (file_node->m_file_type.compare(".component") == 0)
                        {
                            file_node->m_file_type = std::get<2>(extensions) + std::get<1>(extensions);
                        }
                    }
                    file_node->m_file_type = file_node->m_file_type.substr(1);
                    // files use the actual file path from the file system
                    file_node->m_file_path = file_paths[file_index].generic_string();
                }
                file_node->m_node_depth = depth;
                node_array.push_back(file_node);

                bool node_exists = checkFileArray(file_node.get());
                if (node_exists == false)
                {
                    m_file_node_array.push_back(file_node);
                }
                EditorFileNode* parent_node_ptr = getParentNodePtr(node_array[depth].get());
                if (parent_node_ptr != nullptr && node_exists == false)
                {
                    parent_node_ptr->m_child_nodes.push_back(file_node);
                }
                depth++;
            }
        }
    }

    bool EditorFileService::checkFileArray(EditorFileNode* file_node)
    {
        int editor_node_count = m_file_node_array.size();
        for (int file_node_index = 0; file_node_index < editor_node_count; file_node_index++)
        {
            if (m_file_node_array[file_node_index]->m_file_path == file_node->m_file_path)
            {
                return true;
            }
        }
        return false;
    }

    EditorFileNode* EditorFileService::getParentNodePtr(EditorFileNode* file_node)
    {
        // Root's parent is null
        if (file_node->m_node_depth < 0) return nullptr;

        // For depth 0 (first child under root), parent is the configured root node path
        std::filesystem::path child_path(file_node->m_file_path);
        std::filesystem::path parent_path = child_path.parent_path();
        if (file_node->m_node_depth == 0)
        {
            parent_path = std::filesystem::path(m_root_node.m_file_path);
        }

        int editor_node_count = m_file_node_array.size();
        for (int file_node_index = 0; file_node_index < editor_node_count; file_node_index++)
        {
            if (m_file_node_array[file_node_index]->m_file_path == parent_path.generic_string())
            {
                return m_file_node_array[file_node_index].get();
            }
        }
        return nullptr;
    }
}
