#pragma once

#include "TransformComponent.hxx"

namespace ScriptAPI
{
    public ref class Script abstract
    {
      public:
        void virtual Update() {};

        TransformComponent GetTransformComponent();

        void CreateNativeException();
        void CreateSEHException();

      internal:
        void SetEntityId(int id);

      private:
        int entityId;
    };
}
