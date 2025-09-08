#pragma once
#include "runtime/core/meta/reflection/reflection.h"

#include <string>
#include <vector>

namespace Momo
{
    class Component;

    // 单个组件的“类型 + 配置”
    REFLECTION_TYPE(ComponentDefinitionRes)
    CLASS(ComponentDefinitionRes, Fields)
    {
        REFLECTION_BODY(ComponentDefinitionRes);

    public:
        std::string m_type_name;
        std::string m_component;
    };

    // 对象蓝图 = 多个组件定义的集合（多态反射指针保存）
    REFLECTION_TYPE(ObjectDefinitionRes)
    CLASS(ObjectDefinitionRes, Fields)
    {
        REFLECTION_BODY(ObjectDefinitionRes);

    public:
        // 由若干的组件构成
        std::vector<Reflection::ReflectionPtr<Component>> m_components;
    };

    // 对象实例 = 指向蓝图 + 实例级覆盖组件/数据
    REFLECTION_TYPE(ObjectInstanceRes)
    CLASS(ObjectInstanceRes, Fields)
    {
        REFLECTION_BODY(ObjectInstanceRes);

    public:
        std::string              m_name;
        std::string              m_definition;

        std::vector<Reflection::ReflectionPtr<Component>> m_instanced_components;
    };
} // namespace Momo
