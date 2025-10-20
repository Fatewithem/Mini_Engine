#pragma once

#include "runtime/function/animation/skeleton.h"
#include "runtime/function/framework/component/component.h"
#include "runtime/resource/res_type/components/animation_assimp.h"

namespace Momo
{
    class AnimationAssimpComponent : public Component
    {
    public:
        AnimationAssimpComponent() = default;

        void postLoadResource(std::weak_ptr<GObject> parent_object) override;

        void tick(float delta_time) override;

        const AnimationAssimpResult& getResult() const;

        const Skeleton& getSkeleton() const;

    protected:
        META(Enable)
        AnimationAssimpComponentRes m_animation_res;

        Skeleton m_skeleton;
    };
} // namespace Momo
