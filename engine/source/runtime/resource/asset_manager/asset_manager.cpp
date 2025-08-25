#include "runtime/resource/asset_manager/asset_manager.h"

#include "runtime/resource/config_manager/config_manager.h"

#include "runtime/function/global/global_context.h"

#include <filesystem>

namespace Momo
{
    std::filesystem::path AssetManager::getFullPath(const std::string& relative_path) const
    {
        // 拼接为绝对路径
        return std::filesystem::absolute(g_runtime_global_context.m_config_manager->getRootFolder() / relative_path);
    }
} // namespace Momo