#pragma once

#include "runtime/resource/res_type/common/world.h"

#include <filesystem>
#include <string>
#include <memory>
#include <unordered_map>

namespace Momo
{
    // 前向声明
    class Level;          // world 下包含多个 level
    class LevelDebugger;    
    class PhysicsScene;   // 物理场景

    /// Manage all game worlds, it should be support multiple worlds, including game world and editor world.
    class WorldManager
    {
    public:
        virtual ~WorldManager();

        void initialize();
        void clear();

        // 加载/保存场景
        void reloadCurrentLevel();
        void saveCurrentLevel();

        void                 tick(float delta_time);
        std::weak_ptr<Level> getCurrentActiveLevel() const { return m_current_active_level; }  // 拿到当前的level

        std::weak_ptr<PhysicsScene> getCurrentActivePhysicsScene() const;

    private:
        bool loadWorld(const std::string& world_url);
        bool loadLevel(const std::string& level_url);

        bool                      m_is_world_loaded {false};
        std::string               m_current_world_url;
        std::shared_ptr<WorldRes> m_current_world_resource;

        // all loaded levels, key: level url, vaule: level instance
        std::unordered_map<std::string, std::shared_ptr<Level>> m_loaded_levels;
        // active level, currently we just support one active level
        std::weak_ptr<Level> m_current_active_level;

        //debug level
        std::shared_ptr<LevelDebugger> m_level_debugger;
    };
} // namespace Momo