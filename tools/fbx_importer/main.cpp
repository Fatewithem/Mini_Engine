#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>

#include <sys/stat.h>
#include <sys/types.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
// Helper: Write a 1x1 TGA file with 4 channels (BGRA order)
bool Write1x1TGA(const std::string& filename, unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) {
    unsigned char pixel[4] = { b, g, r, a }; // BGRA order for TGA
    return stbi_write_tga(filename.c_str(), 1, 1, 4, pixel) != 0;
}

void ExportPBRMaterials(const aiScene* scene, const std::string& outputDir, const std::string& jsonFilename) {
    // Create output directory if it doesn't exist
#if defined(_WIN32)
    _mkdir(outputDir.c_str());
#else
    mkdir(outputDir.c_str(), 0755);
#endif

    std::ofstream ofs(jsonFilename);
    if (!ofs) {
        std::cerr << "Failed to open " << jsonFilename << " for writing." << std::endl;
        return;
    }
    ofs << "{\n";
    ofs << "  \"materials\": [\n";
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
        const aiMaterial* material = scene->mMaterials[i];
        aiString name;
        material->Get(AI_MATKEY_NAME, name);

        // Get baseColor
        aiColor3D baseColor(1.f, 1.f, 1.f);
        material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor);
        // Get metallic/roughness (Assimp does not have explicit keys, use 0/1)
        float metallic = 0.0f, roughness = 1.0f;
        material->Get("pbrMetallicRoughness.metallicFactor", 0, 0, metallic);
        material->Get("pbrMetallicRoughness.roughnessFactor", 0, 0, roughness);
        // Get emissive
        aiColor3D emissive(0.f, 0.f, 0.f);
        material->Get(AI_MATKEY_COLOR_EMISSIVE, emissive);

        // File names
        std::ostringstream baseColorFile, metallicRoughnessFile, normalFile, occlusionFile, emissiveFile;
        baseColorFile << outputDir << "/material_" << i << "_BaseColor.tga";
        metallicRoughnessFile << outputDir << "/material_" << i << "_MetallicRoughness.tga";
        normalFile << outputDir << "/material_" << i << "_Normal.tga";
        occlusionFile << ""; // leave blank
        emissiveFile << outputDir << "/material_" << i << "_Emissive.tga";

        // Write 1x1 TGA for BaseColor
        Write1x1TGA(baseColorFile.str(), (unsigned char)(baseColor.r * 255), (unsigned char)(baseColor.g * 255), (unsigned char)(baseColor.b * 255), 255);
        // Write 1x1 TGA for MetallicRoughness (B: metallic, G: roughness, R: 0, A: 255)
        Write1x1TGA(metallicRoughnessFile.str(), 0, (unsigned char)(roughness * 255), (unsigned char)(metallic * 255), 255);
        // Write 1x1 TGA for Normal (default: (128,128,255,255))
        Write1x1TGA(normalFile.str(), 128, 128, 255, 255);
        // Write 1x1 TGA for Emissive (use emissive color)
        Write1x1TGA(emissiveFile.str(), (unsigned char)(emissive.r * 255), (unsigned char)(emissive.g * 255), (unsigned char)(emissive.b * 255), 255);

        ofs << "    {\n";
        ofs << "      \"base_colour_texture_file\": \"" << "material_" << i << "_BaseColor.tga" << "\",\n";
        ofs << "      \"metallic_roughness_texture_file\": \"" << "material_" << i << "_MetallicRoughness.tga" << "\",\n";
        ofs << "      \"normal_texture_file\": \"" << "material_" << i << "_Normal.tga" << "\",\n";
        ofs << "      \"occlusion_texture_file\": \"\",\n";
        ofs << "      \"emissive_texture_file\": \"" << "material_" << i << "_Emissive.tga" << "\"\n";
        ofs << "    }";
        if (i + 1 < scene->mNumMaterials) ofs << ",";
        ofs << "\n";
    }
    ofs << "  ]\n";
    ofs << "}\n";
    ofs.close();
}

// Helper: Recursively collect all unique bones in the node hierarchy with parent indices
#include <unordered_map>
void CollectBones(const aiNode* node, int parentIdx,
                  std::vector<const aiNode*>& boneNodes,
                  std::unordered_map<std::string, int>& boneNameToIndex,
                  int& boneCounter)
{
    // If node name is not already in boneNameToIndex, add it as a bone
    if (boneNameToIndex.find(node->mName.C_Str()) == boneNameToIndex.end()) {
        boneNameToIndex[node->mName.C_Str()] = boneCounter++;
        boneNodes.push_back(node);
    }
    int thisIdx = boneNameToIndex[node->mName.C_Str()];
    // For each child, continue
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        CollectBones(node->mChildren[i], thisIdx, boneNodes, boneNameToIndex, boneCounter);
    }
}

// Helper: Map from bone name to aiBone* for offset matrices
void MapBoneNameToAiBone(const aiScene* scene, std::unordered_map<std::string, const aiBone*>& boneNameToAiBone) {
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[i];
        for (unsigned int j = 0; j < mesh->mNumBones; ++j) {
            const aiBone* bone = mesh->mBones[j];
            boneNameToAiBone[bone->mName.C_Str()] = bone;
        }
    }
}

// Helper: Find parent index for a node in the bone list
int FindParentIndex(const aiNode* node, const std::unordered_map<std::string, int>& boneNameToIndex) {
    if (!node->mParent) return -1;
    auto it = boneNameToIndex.find(node->mParent->mName.C_Str());
    if (it != boneNameToIndex.end()) return it->second;
    return -1;
}

void ExportSkeleton(const aiScene* scene, const std::string& filename) {
    std::ofstream ofs(filename);
    if (!ofs) {
        std::cerr << "Failed to open " << filename << " for writing." << std::endl;
        return;
    }

    // Collect all unique bones from all meshes
    std::unordered_map<std::string, const aiBone*> boneNameToAiBone;
    MapBoneNameToAiBone(scene, boneNameToAiBone);

    // Collect all bone nodes in the node hierarchy that match a mesh bone name
    std::vector<const aiNode*> boneNodesOrdered;
    std::unordered_map<std::string, int> boneNameToIndex;
    int boneCounter = 0;
    // Traverse the node hierarchy, only add nodes whose names are in mesh bones
    std::function<void(const aiNode*, int)> collect =
        [&](const aiNode* node, int parentIdx) {
            if (boneNameToAiBone.count(node->mName.C_Str())) {
                boneNameToIndex[node->mName.C_Str()] = boneCounter++;
                boneNodesOrdered.push_back(node);
            }
            for (unsigned int i = 0; i < node->mNumChildren; ++i) {
                collect(node->mChildren[i], boneNameToIndex.count(node->mName.C_Str()) ? boneNameToIndex[node->mName.C_Str()] : parentIdx);
            }
        };
    collect(scene->mRootNode, -1);

    ofs << "{\n";
    ofs << "  \"bones_map\": [\n";
    for (size_t i = 0; i < boneNodesOrdered.size(); ++i) {
        const aiNode* node = boneNodesOrdered[i];
        std::string boneName = node->mName.C_Str();
        int idx = static_cast<int>(i);
        int parentIdx = FindParentIndex(node, boneNameToIndex);
        ofs << "    {\n";
        ofs << "      \"name\": \"" << boneName << "\",\n";
        ofs << "      \"index\": " << idx;
        // Only write parent_index if parentIdx >= 0
        if (parentIdx >= 0) {
            ofs << ",\n";
            ofs << "      \"parent_index\": " << parentIdx << ",\n";
        } else {
            ofs << ",\n";
        }
        // Write binding_pose: always output all values
        ofs << "      \"binding_pose\": {\n";
        ofs << "        \"scale\": [1.0, 1.0, 1.0],\n";
        ofs << "        \"rotation\": [0.0, 0.0, 0.0, 1.0],\n";
        ofs << "        \"position\": [0.0, 0.0, 0.0]\n";
        ofs << "      },\n";
        // Write tpose_matrix as transposed (row-major)
        ofs << "      \"tpose_matrix\": {\n";
        // Use aiBone's offset matrix if available, otherwise identity
        const aiBone* bone = nullptr;
        auto it = boneNameToAiBone.find(boneName);
        aiMatrix4x4 mat;
        if (it != boneNameToAiBone.end()) {
            mat = it->second->mOffsetMatrix;
        } else {
            mat = aiMatrix4x4(); // identity
        }
        // Output as transposed: v0..v15 = mat[0][0], mat[1][0], mat[2][0], mat[3][0], ... (column-major to row-major)
        for (int idxm = 0; idxm < 16; ++idxm) {
            int row = idxm / 4;
            int col = idxm % 4;
            ofs << "        \"v" << idxm << "\": " << mat[col][row];
            if (idxm != 15) ofs << ",";
            ofs << "\n";
        }
        ofs << "      }\n";
        ofs << "    }";
        if (i + 1 < boneNodesOrdered.size()) ofs << ",";
        ofs << "\n";
    }
    ofs << "  ]\n";
    ofs << "}\n";
    ofs.close();
}

void ExportAnimation(const aiScene* scene, const std::string& filename) {
    std::ofstream ofs(filename);
    if (!ofs) {
        std::cerr << "Failed to open " << filename << " for writing." << std::endl;
        return;
    }

    if (scene->mNumAnimations == 0) {
        ofs << "{ \"node_map\": { \"convert\": [] }, \"clip_data\": { \"total_frame\": 0, \"node_count\": 0, \"node_channels\": [] } }";
        ofs.close();
        return;
    }

    const aiAnimation* anim = scene->mAnimations[0];

    ofs << "{\n";
    ofs << "  \"node_map\": {\n";
    ofs << "    \"convert\": [\n";
    for (unsigned int c = 0; c < anim->mNumChannels; ++c) {
        const aiNodeAnim* channel = anim->mChannels[c];
        ofs << "      \"" << channel->mNodeName.C_Str() << "\"";
        if (c + 1 < anim->mNumChannels) ofs << ",";
        ofs << "\n";
    }
    ofs << "    ]\n";
    ofs << "  },\n";

    // Calculate total_frame as the maximum number of keyframes among all channels
    unsigned int total_frame = 0;
    for (unsigned int c = 0; c < anim->mNumChannels; ++c) {
        const aiNodeAnim* channel = anim->mChannels[c];
        total_frame = std::max(total_frame, channel->mNumPositionKeys);
        total_frame = std::max(total_frame, channel->mNumRotationKeys);
        total_frame = std::max(total_frame, channel->mNumScalingKeys);
    }
    if (total_frame == 0) {
        total_frame = static_cast<unsigned int>(anim->mDuration);
    }

    ofs << "  \"clip_data\": {\n";
    ofs << "    \"total_frame\": " << total_frame << ",\n";
    ofs << "    \"node_count\": " << anim->mNumChannels << ",\n";
    ofs << "    \"node_channels\": [\n";

    for (unsigned int c = 0; c < anim->mNumChannels; ++c) {
        const aiNodeAnim* channel = anim->mChannels[c];
        ofs << "      {\n";
        ofs << "        \"name\": \"" << channel->mNodeName.C_Str() << "\",\n";

        // Position keys
        ofs << "        \"position_keys\": [";
        for (unsigned int k = 0; k < channel->mNumPositionKeys; ++k) {
            const aiVectorKey& key = channel->mPositionKeys[k];
            ofs << "{ \"time\": " << key.mTime
                << ", \"x\": " << key.mValue.x
                << ", \"y\": " << key.mValue.y
                << ", \"z\": " << key.mValue.z << " }";
            if (k + 1 < channel->mNumPositionKeys) ofs << ", ";
        }
        ofs << "],\n";

        // Rotation keys
        ofs << "        \"rotation_keys\": [";
        for (unsigned int k = 0; k < channel->mNumRotationKeys; ++k) {
            const aiQuatKey& key = channel->mRotationKeys[k];
            ofs << "{ \"time\": " << key.mTime
                << ", \"x\": " << key.mValue.x
                << ", \"y\": " << key.mValue.y
                << ", \"z\": " << key.mValue.z
                << ", \"w\": " << key.mValue.w << " }";
            if (k + 1 < channel->mNumRotationKeys) ofs << ", ";
        }
        ofs << "],\n";

        // Scaling keys
        ofs << "        \"scaling_keys\": [";
        for (unsigned int k = 0; k < channel->mNumScalingKeys; ++k) {
            const aiVectorKey& key = channel->mScalingKeys[k];
            ofs << "{ \"time\": " << key.mTime
                << ", \"x\": " << key.mValue.x
                << ", \"y\": " << key.mValue.y
                << ", \"z\": " << key.mValue.z << " }";
            if (k + 1 < channel->mNumScalingKeys) ofs << ", ";
        }
        ofs << "]\n";

        ofs << "      }";
        if (c + 1 < anim->mNumChannels) ofs << ",";
        ofs << "\n";
    }
    ofs << "    ]\n";
    ofs << "  }\n";
    ofs << "}\n";

    ofs.close();
}

void ExportMeshBind(const aiScene* scene, const std::string& filename) {
    std::ofstream ofs(filename);
    if (!ofs) {
        std::cerr << "Failed to open " << filename << " for writing." << std::endl;
        return;
    }

    // We only support exporting the first mesh for this format
    if (scene->mNumMeshes == 0) {
        ofs << "{}\n";
        ofs.close();
        return;
    }
    const aiMesh* mesh = scene->mMeshes[0];

    // Build a map from bone name to index
    std::map<std::string, int> boneNameToIndex;
    for (unsigned int b = 0; b < mesh->mNumBones; ++b) {
        boneNameToIndex[mesh->mBones[b]->mName.C_Str()] = b;
    }

    // Build per-vertex bone weights: for each vertex, collect all (boneIndex, weight)
    std::vector<std::vector<std::pair<int, float>>> vertexBoneWeights(mesh->mNumVertices);
    for (unsigned int b = 0; b < mesh->mNumBones; ++b) {
        const aiBone* bone = mesh->mBones[b];
        for (unsigned int w = 0; w < bone->mNumWeights; ++w) {
            const aiVertexWeight& vw = bone->mWeights[w];
            if (vw.mVertexId < mesh->mNumVertices) {
                vertexBoneWeights[vw.mVertexId].push_back(std::make_pair(b, vw.mWeight));
            }
        }
    }

    ofs << "{\n";

    // vertex_buffer
    ofs << "  \"vertex_buffer\": [\n";
    for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
        ofs << "    {";
        // position
        ofs << "\"px\": " << mesh->mVertices[v].x << ", ";
        ofs << "\"py\": " << mesh->mVertices[v].y << ", ";
        ofs << "\"pz\": " << mesh->mVertices[v].z << ", ";
        // normal
        if (mesh->HasNormals()) {
            ofs << "\"nx\": " << mesh->mNormals[v].x << ", ";
            ofs << "\"ny\": " << mesh->mNormals[v].y << ", ";
            ofs << "\"nz\": " << mesh->mNormals[v].z << ", ";
        } else {
            ofs << "\"nx\": 0, \"ny\": 0, \"nz\": 0, ";
        }
        // tangent
        if (mesh->HasTangentsAndBitangents()) {
            ofs << "\"tx\": " << mesh->mTangents[v].x << ", ";
            ofs << "\"ty\": " << mesh->mTangents[v].y << ", ";
            ofs << "\"tz\": " << mesh->mTangents[v].z << ", ";
        } else {
            ofs << "\"tx\": 0, \"ty\": 0, \"tz\": 0, ";
        }
        // uv (first channel)
        if (mesh->HasTextureCoords(0)) {
            ofs << "\"u\": " << mesh->mTextureCoords[0][v].x << ", ";
            ofs << "\"v\": " << mesh->mTextureCoords[0][v].y;
        } else {
            ofs << "\"u\": 0, \"v\": 0";
        }
        ofs << "}";
        if (v + 1 < mesh->mNumVertices) ofs << ",";
        ofs << "\n";
    }
    ofs << "  ],\n";

    // index_buffer
    ofs << "  \"index_buffer\": [";
    bool firstIndex = true;
    for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
        const aiFace& face = mesh->mFaces[f];
        // Only export triangles
        for (unsigned int idx = 0; idx < face.mNumIndices; ++idx) {
            if (!firstIndex) ofs << ", ";
            ofs << face.mIndices[idx];
            firstIndex = false;
        }
    }
    ofs << "],\n";

    // bind: per-vertex bone weights
    ofs << "  \"bind\": [\n";
    for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
        const aiMesh* mesh = scene->mMeshes[m];
        for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
            ofs << "    {";

            int written = 0;
            for (unsigned int w = 0; w < mesh->mNumBones; ++w) {
                const aiBone* bone = mesh->mBones[w];
                for (unsigned int i = 0; i < bone->mNumWeights; ++i) {
                    if (bone->mWeights[i].mVertexId == v) {
                        int boneIndex = w;
                        float weight = bone->mWeights[i].mWeight;

                        // 跳过 index = -1 的情况
                        if (boneIndex < 0) continue;

                        if (written > 0) ofs << ", ";
                        ofs << "\"index" << written << "\": " << boneIndex
                            << ", \"weight" << written << "\": " << weight;
                        written++;
                    }
                }
            }

            ofs << " }";
            if (m != scene->mNumMeshes - 1 || v != mesh->mNumVertices - 1)
                ofs << ",";
            ofs << "\n";
        }
    }
    ofs << "  ]\n";

    ofs << "}\n";
    ofs.close();
}

void ExportSkeletonMap(const aiScene* scene, const std::string& filename) {
    std::ofstream ofs(filename);
    if (!ofs) {
        std::cerr << "Failed to open " << filename << " for writing." << std::endl;
        return;
    }

    // Collect all bones across all meshes in order
    // Here we output the indices in order, starting from 0 to total bones - 1
    unsigned int totalBones = 0;
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        totalBones += scene->mMeshes[i]->mNumBones;
    }

    ofs << "{\n";
    ofs << "  \"convert\": [";
    for (unsigned int i = 0; i < totalBones; ++i) {
        ofs << i;
        if (i + 1 < totalBones) ofs << ", ";
    }
    ofs << "]\n";
    ofs << "}\n";

    ofs.close();
}

void ExportSkeletonMask(const aiScene* scene, const std::string& filename) {
    std::ofstream ofs(filename);
    if (!ofs) {
        std::cerr << "Failed to open " << filename << " for writing." << std::endl;
        return;
    }

    // Count total bones
    unsigned int totalBones = 0;
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        totalBones += scene->mMeshes[i]->mNumBones;
    }

    ofs << "{\n";
    ofs << "  \"enabled\": [";
    for (unsigned int i = 0; i < totalBones; ++i) {
        ofs << 1;
        if (i + 1 < totalBones) ofs << ", ";
    }
    ofs << "]\n";
    ofs << "}\n";

    ofs.close();
}

void ExportMaterial(const aiScene* scene, const std::string& filename) {
    std::ofstream ofs(filename);
    if (!ofs) {
        std::cerr << "Failed to open " << filename << " for writing." << std::endl;
        return;
    }

    ofs << "{\n";
    ofs << "  \"materials\": [\n";
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
        const aiMaterial* material = scene->mMaterials[i];

        aiString name;
        material->Get(AI_MATKEY_NAME, name);

        ofs << "    {\n";
        ofs << "      \"name\": \"" << name.C_Str() << "\"";

        // Diffuse color
        aiColor3D diffuse(0.f, 0.f, 0.f);
        if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse)) {
            ofs << ",\n      \"diffuse\": [" << diffuse.r << ", " << diffuse.g << ", " << diffuse.b << "]";
        }

        // Specular color
        aiColor3D specular(0.f, 0.f, 0.f);
        if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_SPECULAR, specular)) {
            ofs << ",\n      \"specular\": [" << specular.r << ", " << specular.g << ", " << specular.b << "]";
        }

        // Ambient color
        aiColor3D ambient(0.f, 0.f, 0.f);
        if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_AMBIENT, ambient)) {
            ofs << ",\n      \"ambient\": [" << ambient.r << ", " << ambient.g << ", " << ambient.b << "]";
        }

        // Emissive color
        aiColor3D emissive(0.f, 0.f, 0.f);
        if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_EMISSIVE, emissive)) {
            ofs << ",\n      \"emissive\": [" << emissive.r << ", " << emissive.g << ", " << emissive.b << "]";
        }

        // Shininess
        float shininess = 0.f;
        if (AI_SUCCESS == material->Get(AI_MATKEY_SHININESS, shininess)) {
            ofs << ",\n      \"shininess\": " << shininess;
        }

        // Opacity
        float opacity = 1.f;
        if (AI_SUCCESS == material->Get(AI_MATKEY_OPACITY, opacity)) {
            ofs << ",\n      \"opacity\": " << opacity;
        }

        // Textures
        aiString texturePath;
        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
            ofs << ",\n      \"diffuse_texture\": \"" << texturePath.C_Str() << "\"";
        }
        if (material->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS) {
            ofs << ",\n      \"normal_texture\": \"" << texturePath.C_Str() << "\"";
        }
        if (material->GetTexture(aiTextureType_SPECULAR, 0, &texturePath) == AI_SUCCESS) {
            ofs << ",\n      \"specular_texture\": \"" << texturePath.C_Str() << "\"";
        }
        if (material->GetTexture(aiTextureType_EMISSIVE, 0, &texturePath) == AI_SUCCESS) {
            ofs << ",\n      \"emissive_texture\": \"" << texturePath.C_Str() << "\"";
        }

        ofs << "\n    }";
        if (i + 1 < scene->mNumMaterials) ofs << ",";
        ofs << "\n";
    }
    ofs << "  ]\n";
    ofs << "}\n";

    ofs.close();
}

#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>

void ExportTexturesAsTGA(const aiScene* scene, const std::string& outputDir) {
    // Create output directory if it doesn't exist
#if defined(_WIN32)
    _mkdir(outputDir.c_str());
#else
    mkdir(outputDir.c_str(), 0755);
#endif

    for (unsigned int i = 0; i < scene->mNumTextures; ++i) {
        const aiTexture* texture = scene->mTextures[i];
        std::string filename = outputDir + "/texture_" + std::to_string(i) + ".tga";

        if (texture->mHeight == 0) {
            // Compressed texture (e.g. PNG, JPEG embedded)
            // We cannot directly write compressed data as TGA, skip or decode if needed
            std::cerr << "Skipping compressed embedded texture index " << i << " (cannot export as TGA directly)." << std::endl;
            continue;
        } else {
            // Uncompressed texture (raw data)
            int width = texture->mWidth;
            int height = texture->mHeight;
            const unsigned char* data = reinterpret_cast<const unsigned char*>(texture->pcData);

            // Assimp stores textures in BGRA format with 4 bytes per pixel
            // TGA expects BGRA or RGBA, we will write as BGRA

            // Write TGA with 4 channels
            int stride_in_bytes = width * 4;

            if (!stbi_write_tga(filename.c_str(), width, height, 4, data)) {
                std::cerr << "Failed to write texture to " << filename << std::endl;
            } else {
                std::cout << "Exported texture to " << filename << std::endl;
            }
        }
    }

    // Also export external textures referenced by materials
    for (unsigned int m = 0; m < scene->mNumMaterials; ++m) {
        const aiMaterial* material = scene->mMaterials[m];
        aiTextureType textureTypes[] = {
            aiTextureType_DIFFUSE,
            aiTextureType_NORMALS,
            aiTextureType_SPECULAR,
            aiTextureType_EMISSIVE
        };
        for (aiTextureType tt : textureTypes) {
            unsigned int texCount = material->GetTextureCount(tt);
            for (unsigned int t = 0; t < texCount; ++t) {
                aiString path;
                if (material->GetTexture(tt, t, &path) == AI_SUCCESS) {
                    std::string texPath = path.C_Str();
                    // If texture is embedded, it's already handled above
                    if (!texPath.empty() && texPath[0] != '*') {
                        // Load external texture file and convert to TGA
                        // We can use stb_image to load and stb_image_write to save
                        int width, height, channels;
                        unsigned char* imgData = stbi_load(texPath.c_str(), &width, &height, &channels, 4);
                        if (imgData) {
                            std::string filename = outputDir + "/material_" + std::to_string(m) + "_tex_" + std::to_string(t) + ".tga";
                            if (!stbi_write_tga(filename.c_str(), width, height, 4, imgData)) {
                                std::cerr << "Failed to write external texture to " << filename << std::endl;
                            } else {
                                std::cout << "Exported external texture to " << filename << std::endl;
                            }
                            stbi_image_free(imgData);
                        } else {
                            std::cerr << "Failed to load external texture: " << texPath << std::endl;
                        }
                    }
                }
            }
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: fbx_importer <file.fbx>" << std::endl;
        return 1;
    }

    const char* filename = argv[1];

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

    if (!scene) {
        std::cerr << "Error loading file: " << importer.GetErrorString() << std::endl;
        return 1;
    }

    std::cout << "Number of meshes: " << scene->mNumMeshes << std::endl;
    std::cout << "Number of animations: " << scene->mNumAnimations << std::endl;

    // Print animation details
    for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {
        const aiAnimation* anim = scene->mAnimations[i];
        double duration = anim->mTicksPerSecond > 0 ? anim->mDuration / anim->mTicksPerSecond : anim->mDuration;

        std::cout << "clip_index: " << i << std::endl;
        std::cout << "  clip_count: " << scene->mNumAnimations << std::endl;
        std::cout << "  blend_clip_file_length: " << duration << " seconds" << std::endl;
    }

    // std::cout << "Bone node names:" << std::endl;
    // for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
    //     const aiMesh* mesh = scene->mMeshes[i];
    //     for (unsigned int j = 0; j < mesh->mNumBones; ++j) {
    //         std::cout << "  " << mesh->mBones[j]->mName.C_Str() << std::endl;
    //     }
    // }

    ExportSkeleton(scene, "../test_loader/skeleton.json");
    ExportAnimation(scene, "../test_loader/animation_clip.json");
    ExportMeshBind(scene, "../test_loader/mesh_bind.json");
    ExportSkeletonMap(scene, "../test_loader/anim.skeleton_map.json");
    ExportSkeletonMask(scene, "../test_loader/anim.skeleton_mask.json");
    // ExportMaterial(scene, "../test_loader/robot.material.json");
    // ExportTexturesAsTGA(scene, "../test_loader/_textures");
    // ExportPBRMaterials(scene, "../test_loader/_pbr_textures", "../test_loader/robot.pbr.material.json");

    return 0;
}
