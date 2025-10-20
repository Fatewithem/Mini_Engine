#pragma once

#include "runtime/function/render/render_cloth.h"
#include "runtime/function/physics/cloth/cloth_manager.h"

#include "runtime/function/render/render_pass.h"
#include "runtime/function/render/render_resource.h"

namespace Momo
{
    struct ClothPassInitInfo : RenderPassInitInfo
    {
        std::shared_ptr<ClothManager> m_cloth_manager;
    };

    class ClothPass : public RenderPass
    {
    public:
        void initialize(const RenderPassInitInfo* init_info) override final;
        void preparePassData(std::shared_ptr<RenderResourceBase> render_resource) override final;
        void draw() override final;

        void simulate(float dt);

        void setupClothPipeline();
        void updateUniformBuffer();
        void setupAttachments();
        void setupDescriptorSetLayout();
        void allocateDescriptorSet();
        void updateDescriptorSet();
        void prepareUniformBuffer();

        void updateAfterFramebufferRecreate();

    private:
        std::shared_ptr<ClothManager> m_cloth_manager;

        RHIPipeline*       m_render_pipeline = nullptr;
        RHIDescriptorSet*  m_descriptor_set = nullptr;
        RHIDescriptorSetLayout* m_descriptor_set_layout = nullptr;

        RHIBuffer*         m_vertex_buffer = nullptr;
        RHIBuffer*         m_index_buffer = nullptr;
        RHIBuffer*         m_uniform_buffer = nullptr;

        RHIDeviceMemory*   m_vertex_memory = nullptr;
        RHIDeviceMemory*   m_index_memory = nullptr;
        RHIDeviceMemory*   m_uniform_memory = nullptr;

        RHICommandBuffer*  m_render_command_buffer = nullptr;
        RHIRenderPass*     m_render_pass = nullptr;
        RHIFence*          m_render_fence = nullptr;

        RHIViewport        m_viewport;

        struct ClothUniformBufferObject
        {
            Matrix4x4 model;
            Matrix4x4 view;
            Matrix4x4 proj;
        };
        ClothUniformBufferObject m_ubo;

        ClothUniformBufferObject* m_cloth_uniform_buffer_mapped = nullptr;
    };
}