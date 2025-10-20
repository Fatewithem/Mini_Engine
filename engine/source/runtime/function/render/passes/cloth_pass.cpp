#include "runtime/function/render/passes/cloth_pass.h"
#include "runtime/function/render/render_helper.h"
#include "runtime/core/math/matrix4.h"

#include <stdexcept>
#include <cstring>

namespace Momo
{
    void ClothPass::initialize(const RenderPassInitInfo* init_info)
    {
        const auto* cloth_init_info = static_cast<const ClothPassInitInfo*>(init_info);
        m_cloth_manager = cloth_init_info->m_cloth_manager;
    }

    

    void ClothPass::preparePassData(std::shared_ptr<RenderResourceBase> render_resource)
    {
        auto& vertices = m_cloth_manager->clothVertices;
        auto& indices  = m_cloth_manager->clothIndices;
        
        VkDeviceSize vertexBufferSize = sizeof(vertices[0] * vertices.size());
        VkDeviceSize indexBufferSize = sizeof(indices[0] * indices.size());

        // 顶点缓冲
        m_rhi->createBuffer(vertexBufferSize,
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     m_vertex_buffer, m_vertex_memory);

        // 索引缓冲
        m_rhi->createBuffer(indexBufferSize,
                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     m_index_buffer, m_index_memory);

        // 上传数据
        void* data = nullptr;
        
        // 顶点数据上传
        if(m_rhi->mapMemory(m_vertex_memory, 0, vertexBufferSize, 0, &data))
        {
            memcpy(data, vertices.data(), (size_t)vertexBufferSize);
            m_rhi->flushMappedMemoryRanges(nullptr, m_vertex_memory, 0, vertexBufferSize);
            m_rhi->unmapMemory(m_vertex_memory);
        }


        // 索引数据上传
        if (m_rhi->mapMemory(m_index_memory, 0, indexBufferSize, 0, &data))
        {
            memcpy(data, indices.data(), (size_t)indexBufferSize);
            m_rhi->flushMappedMemoryRanges(nullptr, m_index_memory, 0, indexBufferSize);
            m_rhi->unmapMemory(m_index_memory);
        }

        updateUniformBuffer();
    }

    // 每帧物理模拟(CPU)
    void ClothPass::simulate(float dt)
    {
        m_cloth_manager->updateCloth(
            m_cloth_manager->clothVertices,
            dt,
            m_cloth_manager->width,
            m_cloth_manager->height
        );
    }

    // MVP 矩阵
    void ClothPass::updateUniformBuffer() 
    {
        Matrix4x4 model = Matrix4x4::IDENTITY;
        Matrix4x4 view  = Matrix4x4::IDENTITY;
        Matrix4x4 proj  = Matrix4x4::IDENTITY;

        m_ubo.model = model;
        m_ubo.view  = view;
        m_ubo.proj  = proj;

        if(!m_uniform_buffer)
        {
            m_rhi->createBuffer(sizeof(m_ubo), 
                                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                m_uniform_buffer, m_uniform_memory);
        }

        void* data = nullptr;
        if(m_rhi->mapMemory(m_uniform_memory, 0, sizeof(m_ubo), 0, &data)) {
            memcpy(data, &m_ubo, sizeof(m_ubo));
            m_rhi->flushMappedMemoryRanges(nullptr, m_uniform_memory, 0, sizeof(m_ubo));
            m_rhi->unmapMemory(m_uniform_memory);
        }
    }

    void ClothPass::draw()
    {
        VkCommandBuffer cmd = m_render_command_buffer->m_vk_command_buffer;

        VkBuffer vertexBuffers[] = {m_vertex_buffer->m_vk_buffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(cmd, m_index_buffer->m_vk_buffer, 0, VK_INDEX_TYPE_UINT16);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_render_pipeline->m_vk_pipeline);

        vkCmdDrawIndexed(cmd,
                         static_cast<uint32_t>(m_cloth_manager->clothIndices.size()),
                         1, 0, 0, 0);
    }

    void ClothPass::setupClothPipeline()
    {
        // Vertex & Fragment shaders
        RHIPipelineShaderStageCreateInfo shader_stages[2];
        shader_stages[0] = loadShader("shader/cloth.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
        shader_stages[1] = loadShader("shader/cloth.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

        // 顶点输入布局
        auto binding_descriptions = ClothMeshVertex::getBindingDescriptions();
        auto attribute_descriptions = ClothMeshVertex::getAttributeDescriptions();

        RHIPipelineVertexInputStateCreateInfo vertex_input_info{};
        vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size());
        vertex_input_info.pVertexBindingDescriptions    = binding_descriptions.data();
        vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
        vertex_input_info.pVertexAttributeDescriptions    = attribute_descriptions.data();

        // 输入装配
        RHIPipelineInputAssemblyStateCreateInfo input_assembly{};
        input_assembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly.primitiveRestartEnable = VK_FALSE;

        // 光栅化设置
        RHIPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.cullMode = VK_CULL_MODE_NONE;  // 关闭背面剔除
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.lineWidth = 1.0f;
    }

}
