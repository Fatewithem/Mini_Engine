#pragma once

#include <atomic>
#include <limits>

namespace Momo
{
    // 无符号整数
    using GObjectID = std::size_t;

    constexpr GObjectID k_invalid_gobject_id = std::numeric_limits<std::size_t>::max();

    // 为每个 GO 分配 ID
    class ObjectIDAllocator
    {
    public:
        static GObjectID alloc();

    private:
        static std::atomic<GObjectID> m_next_id;  // 保证变量具有原子性
    };
} // namespace Momo
