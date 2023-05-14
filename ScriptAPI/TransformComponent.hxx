#pragma once

namespace ScriptAPI
{
    public value struct TransformComponent
    {
      public:
        property float X
        {
            float get();
            void set(float value);
        }
        property float Y
        {
            float get();
            void set(float value);
        }

      internal:
        TransformComponent(int id);

      private:
        int entityId;
    };
}
