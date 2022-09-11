#include "EngineInterface.hxx"

#include "../Core/Application.h"

namespace ScriptAPI
{
    void EngineInterface::HelloWorld()
    {
        System::Console::WriteLine("Hello Managed World!");
        Core::Application::HelloWorld();
    }
}
