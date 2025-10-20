#pragma once

#include "runtime/function/render/interface/rhi.h"

namespace Momo
{
    class RHI;
    class RenderResourceBase;
    class WindowUI;

    struct RenderPassInitInfo
    {};

    struct RenderPassCommonInfo
    {
        std::shared_ptr<RHI>                rhi;              // 渲染硬件接口
        std::shared_ptr<RenderResourceBase> render_resource;  // 渲染资源统一管理类
    };

    class RenderPassBase
    {
    public:
        virtual void initialize(const RenderPassInitInfo* init_info) = 0;                   // 初始化
        virtual void postInitialize();                                                      // 初始化完成后的额外操作
        virtual void setCommonInfo(RenderPassCommonInfo common_info);                       // 绑定全局共享的 RHI 和资源管理器
        virtual void preparePassData(std::shared_ptr<RenderResourceBase> render_resource);  // 准备更新数据（更新 uniform buffer、绑定资源）
        virtual void initializeUIRenderBackend(WindowUI* window_ui);                        // 用于和 UI 系统对接

    protected:
        std::shared_ptr<RHI>                m_rhi;
        std::shared_ptr<RenderResourceBase> m_render_resource;
    };
}
