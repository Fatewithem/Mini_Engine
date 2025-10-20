#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace Momo
{
    class Bone;
    class RawBone;
    class SkeletonData;

    // 从 vector 中找到 value 的位置
    template<typename T>
    size_t index(const std::vector<T>& vec, const T& value)
    {
        return std::distance(vec.begin(), std::find(vec.begin(), vec.end(), value));
    }

    // 把 addition 里的元素追加到 base 的尾部
    template<typename T>
    void append_vector(std::vector<T>& base, const std::vector<T>& addition)
    {
        base.insert(base.end(), addition.begin(), addition.end());
    }

    // 从 Bone 数组中查询对应 key 的骨骼
    Bone*                    find_by_index(Bone* bones, int key, int size, bool is_flat = false);
    std::shared_ptr<RawBone> find_by_index(std::vector<std::shared_ptr<RawBone>>& bones, int key, bool is_flat = false);
    // 通过名字来查询骨骼
    int                      find_index_by_name(const SkeletonData& skeleton, const std::string& name);
}