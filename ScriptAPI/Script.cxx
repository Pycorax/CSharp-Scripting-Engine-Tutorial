#include "Script.hxx"
#include "../Core/Application.h"

namespace ScriptAPI
{
    TransformComponent Script::GetTransformComponent()
    {
        return TransformComponent(entityId);
    }
    void Script::SetEntityId(int id)
    {
        entityId = id;
    }
}