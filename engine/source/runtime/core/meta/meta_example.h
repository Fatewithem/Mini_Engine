#pragma once

#include "reflection/reflection.h"

namespace Momo
{
    REFLECTION_TYPE(BaseTest) // 交给代码生成器来生成 TypeBaseTestOperator 类
    CLASS(BaseTest, Fields) // Fields 作为注解告诉反射解析器谁该被反射
    {
        REFLECTION_BODY(BaseTest);  // 设置 friend class 和 Serializer

    public:
        int               m_int;
        std::vector<int*> m_int_vector;
    };

    REFLECTION_TYPE(Test1)
    CLASS(Test1 : public BaseTest, WhiteListFields)
    {
        REFLECTION_BODY(Test1);

    public:
        META(Enable)
        char m_char;
    };

    REFLECTION_TYPE(Test2)
    CLASS(Test2 : public BaseTest, , Fields)
    {
        REFLECTION_BODY(Test2);

    public:
        std::vector<Reflection::ReflectionPtr<BaseTest>> m_test_base_array;
    };
} // namespace Momo
