#include "TransformComponent.hxx"
#include "../Core/Application.h"

namespace ScriptAPI
{
    float TransformComponent::X::get()
    {
        return Core::Application::GetComponent(entityId)->x;
    }
    void TransformComponent::X::set(float value)
    {
        Core::Application::GetComponent(entityId)->x = value;
    }
    float TransformComponent::Y::get()
    {
        return Core::Application::GetComponent(entityId)->y;
    }
    void TransformComponent::Y::set(float value)
    {
        Core::Application::GetComponent(entityId)->y = value;
    }

    TransformComponent::TransformComponent(int id)
    : entityId { id }
    {}
}