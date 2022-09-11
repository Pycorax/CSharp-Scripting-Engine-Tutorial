#include "Application.h"

#include <Windows.h>                // GetKeyState()

namespace Core
{
    void Application::Run()
    {
        // Load
        while (true)
        {
            if (GetKeyState(VK_ESCAPE) & 0x8000)
                break;
        }
    }
}
