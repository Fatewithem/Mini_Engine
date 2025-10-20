// #include "animation_assimp_loader.h"

// #include "runtime/resource/res_type/data/animation_clip.h"
// #include "runtime/resource/res_type/data/skeleton_mask.h"

// #include "runtime/function/animation/utilities.h"
// #include "runtime/function/global/global_context.h"

// namespace Momo 
// {
//     AnimationAssimpLoader::AnimationAssimpLoader(const std::string& fbx_url) 
//     {
//         m_scene = m_importer.ReadFile(
//             fbx_url,
//             aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices
//         );

//         if (!m_scene || m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_scene->mRootNode) {
//             throw std::runtime_error("Failed to load FBX: " + fbx_url);
//         }

        
//     }


// }