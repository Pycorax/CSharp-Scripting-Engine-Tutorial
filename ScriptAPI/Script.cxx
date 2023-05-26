#include "Script.hxx"
#include "../Core/Application.h"

namespace ScriptAPI
{
    TransformComponent Script::GetTransformComponent()
    {
        return TransformComponent(entityId);
    }

    void Script::CreateNativeException()
    {
        throw std::runtime_error("Intentional Error!");
    }

    void Script::CreateSEHException()
    {
        int* i = nullptr;
        *i = 42;
    }

    void Script::SetEntityId(int id)
    {
        entityId = id;
    }
}