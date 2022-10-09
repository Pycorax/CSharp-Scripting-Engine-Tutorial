#pragma once

#include "Script.hxx"

namespace ScriptAPI
{
    public ref class EngineInterface
    {
      public:
        static void Init();
        static bool AddScriptViaName(int entityId, System::String^ scriptName);
        static void ExecuteUpdate();

      private:
        using ScriptList = System::Collections::Generic::List<Script^>;

        static System::Collections::Generic::List<ScriptList^>^ scripts;		  
        static System::Collections::Generic::IEnumerable<System::Type^>^ scriptTypeList;

        static void updateScriptTypeList();
    };
}
