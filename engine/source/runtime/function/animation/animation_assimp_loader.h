// #pragma once

// #include <assimp/Importer.hpp>
// #include <assimp/scene.h>
// #include <assimp/postprocess.h>

// #include "runtime/resource/res_type/data/animation_clip.h"
// #include "runtime/resource/res_type/data/animation_skeleton_node_map.h"
// #include "runtime/resource/res_type/data/skeleton_data.h"
// #include "runtime/resource/res_type/data/skeleton_mask.h"

// #include <memory>
// #include <string>

// namespace Momo
// {
//     class AnimationAssimpLoader
//     {
//     public:
//         AnimationAssimpLoader(const std::string& fbx_url);
//         ~AnimationAssimpLoader();

//     private:
//         Assimp::Importer m_importer;          // 注意要保存 Importer
//         const aiScene* m_scene {nullptr};     // 保留 const

//         std::shared_ptr<AnimationClip> m_AnimationClip;
//         std::shared_ptr<SkeletonData>  m_SkeletonData;
//         std::shared_ptr<AnimSkelMap>   m_AnimSkelMap;
//         std::shared_ptr<BoneBlendMask> m_BoneBlendMask;
//     };
// } // namespace Momo