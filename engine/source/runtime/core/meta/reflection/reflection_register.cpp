#include <assert.h>

#include "runtime/core/meta/json.h"
#include "runtime/core/meta/reflection/reflection.h"
#include "runtime/core/meta/reflection/reflection_register.h"
#include "runtime/core/meta/serializer/serializer.h"

#define private public
#define protected public
#include "_generated/reflection/all_reflection.h"
#include "_generated/serializer/all_serializer.ipp"
#undef private
#undef protected

namespace Momo
{
    namespace Reflection
    {
        void TypeMetaRegister::metaUnregister() { TypeMetaRegisterinterface::unregisterAll(); }
    } // namespace Reflection
} // namespace Momo
