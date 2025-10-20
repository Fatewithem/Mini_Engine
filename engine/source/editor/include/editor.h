#pragma once

#include "runtime/core/math/vector2.h"

#include <memory>

namespace Momo 
{
    class EditorUI;

    class MomoEngine;

    class MomoEditor {
        friend class EditorUI;

    public:
        MomoEditor();
        virtual ~MomoEditor();

        void initialize(MomoEngine* engine_runtime);
        void clear();

        void run();
    
    protected:
        std::shared_ptr<EditorUI> m_editor_ui;
        MomoEngine* m_engine_runtime{ nullptr };
    };
}

