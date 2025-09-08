#include "runtime/function/character/character.h"

#include "runtime/engine.h"
#include "runtime/function/framework/component/motor/motor_component.h"
#include "runtime/function/framework/component/transform/transform_component.h"
#include "runtime/function/global/global_context.h"
#include "runtime/function/input/input_system.h"

namespace Momo
{
    Character::Character(std::shared_ptr<GObject> character_object) { setObject(character_object); }

    GObjectID Character::getObjectID() const
    {
        if (m_character_object)
        {
            return m_character_object->getID();
        }

        return k_invalid_gobject_id;
    }

    void Character::setObject(std::shared_ptr<GObject> gobject)
    {
        m_character_object = gobject;
        if (m_character_object)
        {
            // 拿到 Transform 组件
            const TransformComponent* transform_component =
                m_character_object->tryGetComponentConst(TransformComponent);
            const Transform& transform = transform_component->getTransformConst();
            m_position                 = transform.m_position;
            m_rotation                 = transform.m_rotation;
        }
        else
        {
            m_position = Vector3::ZERO;
            m_rotation = Quaternion::IDENTITY;
        }
    }

    void Character::tick(float delta_time)
    {
        if (m_character_object == nullptr)
            return;

        // 从全局拿到输入
        unsigned int command = g_runtime_global_context.m_input_system->getGameCommand();
        if (command < (unsigned int)GameCommand::invalid)
        {
            if ((((unsigned int)GameCommand::free_carema & command) > 0) != m_is_free_camera)
            {
                toggleFreeCamera();
            }
        }

        TransformComponent* transform_component = m_character_object->tryGetComponent(TransformComponent);

        if (m_rotation_dirty)
        {
            transform_component->setRotation(m_rotation_buffer);
            m_rotation_dirty = false;
        }

        const MotorComponent* motor_component = m_character_object->tryGetComponentConst(MotorComponent);
        if (motor_component == nullptr)
        {
            return;
        }

        if (motor_component->getIsMoving())
        {
            m_rotation_buffer = m_rotation;           // 把当前逻辑旋转拷贝进缓存
            transform_component->setRotation(m_rotation_buffer); // 立即写进 Transform
            m_rotation_dirty = true;                  // 再次标记“还有待确认”
        }

        const Vector3& new_position = motor_component->getTargetPosition();

        m_position = new_position;
    }

    void Character::toggleFreeCamera()
    {
        CameraComponent* camera_component = m_character_object->tryGetComponent(CameraComponent);
        if (camera_component == nullptr) return;

        m_is_free_camera = !m_is_free_camera;

        if (m_is_free_camera)
        {
            m_original_camera_mode = camera_component->getCameraMode();
            camera_component->setCameraMode(CameraMode::free);
        }
        else
        {
            camera_component->setCameraMode(m_original_camera_mode);
        }
    }
} // namespace Momo