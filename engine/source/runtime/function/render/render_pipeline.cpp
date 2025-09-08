#include "runtime/function/render/render_pipeline.h"
#include "runtime/function/render/interface/vulkan/vulkan_rhi.h"

#include "runtime/function/render/passes/color_grading_pass.h"
#include "runtime/function/render/passes/combine_ui_pass.h"
#include "runtime/function/render/passes/directional_light_pass.h"
#include "runtime/function/render/passes/main_camera_pass.h"
#include "runtime/function/render/passes/pick_pass.h"
#include "runtime/function/render/passes/point_light_pass.h"
#include "runtime/function/render/passes/tone_mapping_pass.h"
#include "runtime/function/render/passes/ui_pass.h"
#include "runtime/function/render/passes/particle_pass.h"

#include "runtime/function/render/debugdraw/debug_draw_manager.h"

#include "runtime/core/base/macro.h"

namespace Momo
{
    void RenderPipeline::initialize(RenderPipelineInitInfo init_info)
    {
        // 创建所有 pass 的实例
        m_point_light_shadow_pass = std::make_shared<PointLightShadowPass>();
        m_directional_light_pass  = std::make_shared<DirectionalLightShadowPass>();
        m_main_camera_pass        = std::make_shared<MainCameraPass>();
        m_tone_mapping_pass       = std::make_shared<ToneMappingPass>();
        m_color_grading_pass      = std::make_shared<ColorGradingPass>();
        m_ui_pass                 = std::make_shared<UIPass>();
        m_combine_ui_pass         = std::make_shared<CombineUIPass>();
        m_pick_pass               = std::make_shared<PickPass>();
        m_fxaa_pass               = std::make_shared<FXAAPass>();
        m_particle_pass           = std::make_shared<ParticlePass>();

        RenderPassCommonInfo pass_common_info;
        pass_common_info.rhi             = m_rhi;  // 抽象 VulkanRHI
        pass_common_info.render_resource = init_info.render_resource;

        m_point_light_shadow_pass->setCommonInfo(pass_common_info);
        m_directional_light_pass->setCommonInfo(pass_common_info);
        m_main_camera_pass->setCommonInfo(pass_common_info);
        m_tone_mapping_pass->setCommonInfo(pass_common_info);
        m_color_grading_pass->setCommonInfo(pass_common_info);
        m_ui_pass->setCommonInfo(pass_common_info);
        m_combine_ui_pass->setCommonInfo(pass_common_info);
        m_pick_pass->setCommonInfo(pass_common_info);
        m_fxaa_pass->setCommonInfo(pass_common_info);
        m_particle_pass->setCommonInfo(pass_common_info);

        m_point_light_shadow_pass->initialize(nullptr);
        m_directional_light_pass->initialize(nullptr);

        std::shared_ptr<MainCameraPass> main_camera_pass = std::static_pointer_cast<MainCameraPass>(m_main_camera_pass);
        std::shared_ptr<RenderPass>     _main_camera_pass = std::static_pointer_cast<RenderPass>(m_main_camera_pass);
        std::shared_ptr<ParticlePass> particle_pass = std::static_pointer_cast<ParticlePass>(m_particle_pass);

        // 粒子系统
        ParticlePassInitInfo particle_init_info{};
        particle_init_info.m_particle_manager = g_runtime_global_context.m_particle_manager;
        m_particle_pass->initialize(&particle_init_info);

        // 两种阴影贴图的 framebuffer image view 传给主摄像机 Pass
        main_camera_pass->m_point_light_shadow_color_image_view =
            std::static_pointer_cast<RenderPass>(m_point_light_shadow_pass)->getFramebufferImageViews()[0];
        main_camera_pass->m_directional_light_shadow_color_image_view =
            std::static_pointer_cast<RenderPass>(m_directional_light_pass)->m_framebuffer.attachments[0].view;

        MainCameraPassInitInfo main_camera_init_info;
        main_camera_init_info.enble_fxaa = init_info.enable_fxaa;
        main_camera_pass->setParticlePass(particle_pass);
        m_main_camera_pass->initialize(&main_camera_init_info);

        std::static_pointer_cast<ParticlePass>(m_particle_pass)->setupParticlePass();

        // 取主摄像机 Pass 的 descriptor_set_layouts，把 每网格布局 交给两种阴影 Pass，用于网格级资源绑定一致化
        std::vector<RHIDescriptorSetLayout*> descriptor_layouts = _main_camera_pass->getDescriptorSetLayouts();
        std::static_pointer_cast<PointLightShadowPass>(m_point_light_shadow_pass)
            ->setPerMeshLayout(descriptor_layouts[MainCameraPass::LayoutType::_per_mesh]);
        std::static_pointer_cast<DirectionalLightShadowPass>(m_directional_light_pass)
            ->setPerMeshLayout(descriptor_layouts[MainCameraPass::LayoutType::_per_mesh]);

        // 阴影 Pass 在拿到 per-mesh layout 后做二次初始化
        m_point_light_shadow_pass->postInitialize();
        m_directional_light_pass->postInitialize();

        // odd 备份缓冲
        ToneMappingPassInitInfo tone_mapping_init_info;
        tone_mapping_init_info.render_pass = _main_camera_pass->getRenderPass();
        tone_mapping_init_info.input_attachment =
            _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd];
        m_tone_mapping_pass->initialize(&tone_mapping_init_info);

        // even 备份缓冲
        ColorGradingPassInitInfo color_grading_init_info;
        color_grading_init_info.render_pass = _main_camera_pass->getRenderPass();
        color_grading_init_info.input_attachment =
            _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_backup_buffer_even];
        m_color_grading_pass->initialize(&color_grading_init_info);

        // 复用主摄像机的render_pass
        UIPassInitInfo ui_init_info;
        ui_init_info.render_pass = _main_camera_pass->getRenderPass();
        m_ui_pass->initialize(&ui_init_info);

        // 场景输入取 odd，UI 输入取 even，用于把 UI 叠加回场景颜色
        CombineUIPassInitInfo combine_ui_init_info;
        combine_ui_init_info.render_pass = _main_camera_pass->getRenderPass();
        combine_ui_init_info.scene_input_attachment =
            _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd];
        combine_ui_init_info.ui_input_attachment =
            _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_backup_buffer_even];
        m_combine_ui_pass->initialize(&combine_ui_init_info);

        PickPassInitInfo pick_init_info;
        pick_init_info.per_mesh_layout = descriptor_layouts[MainCameraPass::LayoutType::_per_mesh];
        m_pick_pass->initialize(&pick_init_info);

        // 主摄像机的“post-process odd 缓冲”
        FXAAPassInitInfo fxaa_init_info;
        fxaa_init_info.render_pass = _main_camera_pass->getRenderPass();
        fxaa_init_info.input_attachment =
            _main_camera_pass->getFramebufferImageViews()[_main_camera_pass_post_process_buffer_odd];
        m_fxaa_pass->initialize(&fxaa_init_info);

    }

    void RenderPipeline::forwardRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource)
    {
        VulkanRHI*      vulkan_rhi      = static_cast<VulkanRHI*>(rhi.get());
        RenderResource* vulkan_resource = static_cast<RenderResource*>(render_resource.get());

        // 把帧循环用的环形缓冲复位到当前帧起点
        vulkan_resource->resetRingBufferOffset(vulkan_rhi->m_current_frame_index);

        // 等待上一帧 GPU 完成
        vulkan_rhi->waitForFences();

        // 重置命令池，回收命令缓冲
        vulkan_rhi->resetCommandPool();

        // 处理交换链尺寸变化
        bool recreate_swapchain =
            vulkan_rhi->prepareBeforePass(std::bind(&RenderPipeline::passUpdateAfterRecreateSwapchain, this));
        if (recreate_swapchain)
        {
            return;
        }

        // 离屏绘制阴影
        static_cast<DirectionalLightShadowPass*>(m_directional_light_pass.get())->draw();

        static_cast<PointLightShadowPass*>(m_point_light_shadow_pass.get())->draw();

        // 取出后处理/UI/粒子引用：为了把它们传给主摄像机的绘制入口（避免层层 get）
        ColorGradingPass& color_grading_pass = *(static_cast<ColorGradingPass*>(m_color_grading_pass.get()));
        FXAAPass&         fxaa_pass          = *(static_cast<FXAAPass*>(m_fxaa_pass.get()));
        ToneMappingPass&  tone_mapping_pass  = *(static_cast<ToneMappingPass*>(m_tone_mapping_pass.get()));
        UIPass&           ui_pass            = *(static_cast<UIPass*>(m_ui_pass.get()));
        CombineUIPass&    combine_ui_pass    = *(static_cast<CombineUIPass*>(m_combine_ui_pass.get()));
        ParticlePass&     particle_pass      = *(static_cast<ParticlePass*>(m_particle_pass.get()));

        static_cast<ParticlePass*>(m_particle_pass.get())
            ->setRenderCommandBufferHandle(
                static_cast<MainCameraPass*>(m_main_camera_pass.get())->getRenderCommandBuffer());
        
        // 主摄像机绘制（前向）
        static_cast<MainCameraPass*>(m_main_camera_pass.get())
            ->drawForward(color_grading_pass,
                          fxaa_pass,
                          tone_mapping_pass,
                          ui_pass,
                          combine_ui_pass,
                          particle_pass,
                          vulkan_rhi->m_current_swapchain_image_index);

        
        g_runtime_global_context.m_debugdraw_manager->draw(vulkan_rhi->m_current_swapchain_image_index);

        vulkan_rhi->submitRendering(std::bind(&RenderPipeline::passUpdateAfterRecreateSwapchain, this));
        static_cast<ParticlePass*>(m_particle_pass.get())->copyNormalAndDepthImage();
        static_cast<ParticlePass*>(m_particle_pass.get())->simulate();
    }

    void RenderPipeline::deferredRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource)
    {
        VulkanRHI*      vulkan_rhi      = static_cast<VulkanRHI*>(rhi.get());
        RenderResource* vulkan_resource = static_cast<RenderResource*>(render_resource.get());

        vulkan_resource->resetRingBufferOffset(vulkan_rhi->m_current_frame_index);

        vulkan_rhi->waitForFences();

        vulkan_rhi->resetCommandPool();

        bool recreate_swapchain =
            vulkan_rhi->prepareBeforePass(std::bind(&RenderPipeline::passUpdateAfterRecreateSwapchain, this));
        if (recreate_swapchain)
        {
            return;
        }

        static_cast<DirectionalLightShadowPass*>(m_directional_light_pass.get())->draw();

        static_cast<PointLightShadowPass*>(m_point_light_shadow_pass.get())->draw();

        ColorGradingPass& color_grading_pass = *(static_cast<ColorGradingPass*>(m_color_grading_pass.get()));
        FXAAPass&         fxaa_pass          = *(static_cast<FXAAPass*>(m_fxaa_pass.get()));
        ToneMappingPass&  tone_mapping_pass  = *(static_cast<ToneMappingPass*>(m_tone_mapping_pass.get()));
        UIPass&           ui_pass            = *(static_cast<UIPass*>(m_ui_pass.get()));
        CombineUIPass&    combine_ui_pass    = *(static_cast<CombineUIPass*>(m_combine_ui_pass.get()));
        ParticlePass&     particle_pass      = *(static_cast<ParticlePass*>(m_particle_pass.get()));

        static_cast<ParticlePass*>(m_particle_pass.get())
            ->setRenderCommandBufferHandle(
                static_cast<MainCameraPass*>(m_main_camera_pass.get())->getRenderCommandBuffer());

        static_cast<MainCameraPass*>(m_main_camera_pass.get())
            ->draw(color_grading_pass,
                   fxaa_pass,
                   tone_mapping_pass,
                   ui_pass,
                   combine_ui_pass,
                   particle_pass,
                   vulkan_rhi->m_current_swapchain_image_index);
                   
        g_runtime_global_context.m_debugdraw_manager->draw(vulkan_rhi->m_current_swapchain_image_index);

        vulkan_rhi->submitRendering(std::bind(&RenderPipeline::passUpdateAfterRecreateSwapchain, this));
        static_cast<ParticlePass*>(m_particle_pass.get())->copyNormalAndDepthImage();
        static_cast<ParticlePass*>(m_particle_pass.get())->simulate();
    }

    // 当窗口尺寸变化导致交换链/主摄像机 FBO 重建时，按新 framebuffer image view 索引逐个通知各 Pass
    void RenderPipeline::passUpdateAfterRecreateSwapchain()
    {
        MainCameraPass&   main_camera_pass   = *(static_cast<MainCameraPass*>(m_main_camera_pass.get()));
        ColorGradingPass& color_grading_pass = *(static_cast<ColorGradingPass*>(m_color_grading_pass.get()));
        FXAAPass&         fxaa_pass          = *(static_cast<FXAAPass*>(m_fxaa_pass.get()));
        ToneMappingPass&  tone_mapping_pass  = *(static_cast<ToneMappingPass*>(m_tone_mapping_pass.get()));
        CombineUIPass&    combine_ui_pass    = *(static_cast<CombineUIPass*>(m_combine_ui_pass.get()));
        PickPass&         pick_pass          = *(static_cast<PickPass*>(m_pick_pass.get()));
        ParticlePass&     particle_pass      = *(static_cast<ParticlePass*>(m_particle_pass.get()));

        main_camera_pass.updateAfterFramebufferRecreate();
        tone_mapping_pass.updateAfterFramebufferRecreate(
            main_camera_pass.getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd]);
        color_grading_pass.updateAfterFramebufferRecreate(
            main_camera_pass.getFramebufferImageViews()[_main_camera_pass_backup_buffer_even]);
        fxaa_pass.updateAfterFramebufferRecreate(
            main_camera_pass.getFramebufferImageViews()[_main_camera_pass_post_process_buffer_odd]);
        combine_ui_pass.updateAfterFramebufferRecreate(
            main_camera_pass.getFramebufferImageViews()[_main_camera_pass_backup_buffer_odd],
            main_camera_pass.getFramebufferImageViews()[_main_camera_pass_backup_buffer_even]);
        pick_pass.recreateFramebuffer();
        particle_pass.updateAfterFramebufferRecreate();
        g_runtime_global_context.m_debugdraw_manager->updateAfterRecreateSwapchain();
    }

    // 返回像素处的网格/对象 GUID
    uint32_t RenderPipeline::getGuidOfPickedMesh(const Vector2& picked_uv)
    {
        PickPass& pick_pass = *(static_cast<PickPass*>(m_pick_pass.get()));
        return pick_pass.pick(picked_uv);
    }

    // 修改主摄像机 Pass 的绘制状态（是否显示坐标轴、当前选中的轴）
    void RenderPipeline::setAxisVisibleState(bool state)
    {
        MainCameraPass& main_camera_pass = *(static_cast<MainCameraPass*>(m_main_camera_pass.get()));
        main_camera_pass.m_is_show_axis  = state;
    }

    void RenderPipeline::setSelectedAxis(size_t selected_axis)
    {
        MainCameraPass& main_camera_pass = *(static_cast<MainCameraPass*>(m_main_camera_pass.get()));
        main_camera_pass.m_selected_axis = selected_axis;
    }
} // namespace Momo
