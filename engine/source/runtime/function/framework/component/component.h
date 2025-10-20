#pragma once
#include "runtime/core/meta/reflection/reflection.h"

namespace Momo
{
    class GObject;

    // Component
    REFLECTION_TYPE(Component)
    CLASS(Component, WhiteListFields)
    {
        REFLECTION_BODY(Component)
    protected:
        std::weak_ptr<GObject> m_parent_object;           // 避免组件与宿主对象形成强引用循环（GObject 一般拥有 shared_ptr<Component>，组件再持有 shared_ptr<GObject> 会导致双方都不释放）
        bool                   m_is_dirty {false};        // 统一脏位
        bool                   m_is_scale_dirty {false};  // 标记是否进行了缩放

    public:
        Component() = default;
        virtual ~Component() {}

        // 资源加载后进行绑定，把“我属于哪个对象”的上下文传给组件（设置 m_parent_object）
        virtual void postLoadResource(std::weak_ptr<GObject> parent_object) { m_parent_object = parent_object; }

        virtual void tick(float delta_time) {};

        bool isDirty() const { return m_is_dirty; }

        void setDirtyFlag(bool is_dirty) { m_is_dirty = is_dirty; }

        bool m_tick_in_editor_mode {false};
    };

} // namespace Momo