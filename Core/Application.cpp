#include "Application.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <Windows.h>                // GetKeyState()
#include <shlwapi.h>                // GetModuleFileNameA(), PathRemoveFileSpecA()
#include <array>
#include <filesystem>               // std::filesystem::current_path()

#pragma comment(lib, "shlwapi.lib") // Needed for <shlwapi.h>

namespace Core
{
    std::array<TransformComponent, Application::ENTITY_COUNT> Application::nativeData;

    void Application::Run()
    {
        startScriptEngine();
        compileScriptAssembly();

        // Get Functions
        auto init = GetFunctionPtr<void(*)(void)>
        (
            "ScriptAPI",
            "ScriptAPI.EngineInterface",
            "Init"
        );
        auto addScript = GetFunctionPtr<bool(*)(int, const char*)>
        (
            "ScriptAPI",
            "ScriptAPI.EngineInterface",
            "AddScriptViaName"
        );
        auto executeUpdate = GetFunctionPtr<void(*)(void)>
        (
            "ScriptAPI",
            "ScriptAPI.EngineInterface",
            "ExecuteUpdate"
        );
        auto reloadScripts = GetFunctionPtr<void(*)(void)>
        (
            "ScriptAPI",
            "ScriptAPI.EngineInterface",
            "Reload"
        );

        // Initialize
        init();

        addScript(0, "TestScript");

        // Load
        while (true)
        {
            if (GetKeyState(VK_ESCAPE) & 0x8000)
                break;

            if (GetKeyState(VK_SPACE) & 0x8000)
            {
                compileScriptAssembly();
                reloadScripts();
                addScript(0, "TestScript");
            }

            executeUpdate();
        }

        stopScriptEngine();
    }

    void Application::HelloWorld()
    {
        std::cout << "Hello Native World!" << std::endl;
    }

    TransformComponent* Application::GetComponent(int entityId)
    {
        if (entityId < MIN_ENTITY_ID || entityId > MAX_ENTITY_ID)
            return nullptr;

        return &nativeData[entityId];
    }

    void Application::startScriptEngine()
    {
        // Get the .NET Runtime's path first
        const auto DOT_NET_PATH = getDotNetRuntimePath();
        if (DOT_NET_PATH.empty())
            throw std::runtime_error("Failed to find .NET Runtime.");

        // Get the current executable directory so that we can find the coreclr.dll to load
        std::string runtimePath(MAX_PATH, '\0');
        GetModuleFileNameA(nullptr, runtimePath.data(), MAX_PATH);
        PathRemoveFileSpecA(runtimePath.data());
        // Since PathRemoveFileSpecA() removes from data(), the size is not updated, so we must manually update it
        runtimePath.resize(std::strlen(runtimePath.data()));

        // Also, while we're at it, set the current working directory to the current executable directory
        std::filesystem::current_path(runtimePath);

        // Construct the CoreCLR path
        const std::string CORE_CLR_PATH = DOT_NET_PATH + "\\coreclr.dll";

        // Load the CoreCLR DLL
        coreClr = LoadLibraryExA(CORE_CLR_PATH.c_str(), nullptr, 0);
        if (!coreClr)
            throw std::runtime_error("Failed to load CoreCLR.");

        // Step 2: Get CoreCLR hosting functions
        initializeCoreClr     = getCoreClrFuncPtr<coreclr_initialize_ptr>("coreclr_initialize");
        createManagedDelegate = getCoreClrFuncPtr<coreclr_create_delegate_ptr>("coreclr_create_delegate");
        shutdownCoreClr       = getCoreClrFuncPtr<coreclr_shutdown_ptr>("coreclr_shutdown");

        // Step 3: Construct AppDomain properties used when starting the runtime
        std::string tpaList = buildTpaList(runtimePath) + buildTpaList(DOT_NET_PATH);

        // Define CoreCLR properties
        std::array propertyKeys =
        {
            "TRUSTED_PLATFORM_ASSEMBLIES",      // Trusted assemblies (like the GAC)
            "APP_PATHS",                        // Directories to probe for application assemblies
        };
        std::array propertyValues =
        {
            tpaList.c_str(),
            runtimePath.c_str()
        };

        // Step 4: Start the CoreCLR runtime
        int result = initializeCoreClr
        (
            runtimePath.c_str(),     // AppDomain base path
            "SampleHost",            // AppDomain friendly name, this can be anything you want really
            propertyKeys.size(),     // Property count
            propertyKeys.data(),     // Property names
            propertyValues.data(),   // Property values
            &hostHandle,             // Host handle
            &domainId                // AppDomain ID
        );

        // Check if intiialization of CoreCLR failed
        if (result < 0)
        {
            std::ostringstream oss;
            oss << std::hex << std::setfill('0') << std::setw(8)
                << "Failed to initialize CoreCLR. Error 0x" << result << "\n";
            throw std::runtime_error(oss.str());
        }
    }
    void Application::stopScriptEngine()
    {
        // Shutdown CoreCLR
        const int RESULT = shutdownCoreClr(hostHandle, domainId);
        if (RESULT < 0)
        {
            std::stringstream oss;
            oss << std::hex << std::setfill('0') << std::setw(8)
                << "[DotNetRuntime] Failed to shut down CoreCLR. Error 0x" << RESULT << "\n";
            throw std::runtime_error(oss.str());
        }
    }
    std::string Application::buildTpaList(const std::string& directory)
    {
        // Constants
        const std::string SEARCH_PATH = directory + "\\*.dll";
        static constexpr char PATH_DELIMITER = ';';

        // Create a osstream object to compile the string
        std::ostringstream tpaList;

        // Search the current directory for the TPAs (.DLLs)
        WIN32_FIND_DATAA findData;
        HANDLE fileHandle = FindFirstFileA(SEARCH_PATH.c_str(), &findData);
        if (fileHandle != INVALID_HANDLE_VALUE)
        {
            do
            {
                // Append the assembly to the list
                tpaList << directory << '\\' << findData.cFileName << PATH_DELIMITER;
            } 
            while (FindNextFileA(fileHandle, &findData));
            FindClose(fileHandle);
        }

        return tpaList.str();
    }
    void Application::compileScriptAssembly()
    {  
        const char* PROJ_PATH =
            "..\\..\\ManagedScripts\\ManagedScripts.csproj";
        std::wstring buildCmd = L" build \"" +
            std::filesystem::absolute(PROJ_PATH).wstring() +
            L"\" -c Debug --no-self-contained " +
            L"-o \"./tmp_build/\" -r \"win-x64\"";

        // Define the struct to config the compiler process call
        STARTUPINFOW startInfo;
        PROCESS_INFORMATION pi;
        ZeroMemory(&startInfo, sizeof(startInfo));
        ZeroMemory(&pi, sizeof(pi));
        startInfo.cb = sizeof(startInfo);

        // Start compiler process
        const auto SUCCESS = CreateProcess
        (
            L"C:\\Program Files\\dotnet\\dotnet.exe", buildCmd.data(),
            nullptr, nullptr, true, NULL, nullptr, nullptr,
            &startInfo, &pi
        );

        // Check that we launched the process
        if (!SUCCESS)
        {
            auto err = GetLastError();
            std::ostringstream oss;
            oss << "Failed to launch compiler. Error code: " << std::hex << err;
            throw std::runtime_error(oss.str());
        }

        // Wait for process to end
        DWORD exitCode{};
        while (true)
        {
            const auto EXEC_SUCCESS = GetExitCodeProcess(pi.hProcess, &exitCode);
            if (!EXEC_SUCCESS)
            {
                auto err = GetLastError();
                std::ostringstream oss;
                oss << "Failed to query process. Error code: " << std::hex << err;
                throw std::runtime_error(oss.str());
            }

            if (exitCode != STILL_ACTIVE)
                break;
        }

        // Successful build
        if (exitCode == 0)
        {
            // Copy out files
            std::filesystem::copy_file
            (
                "./tmp_build/ManagedScripts.dll",
                "ManagedScripts.dll",
                std::filesystem::copy_options::overwrite_existing
            );
            // Copy out the pdb file too
            try
            {
                std::filesystem::copy_file
                (
                    "./tmp_build/ManagedScripts.pdb",
                    "ManagedScripts.pdb",
                    std::filesystem::copy_options::overwrite_existing
                );
            }
            catch (const std::filesystem::filesystem_error& e)
            {
                throw std::runtime_error("Failed to overwrite .PDB file. Is a debugger attached?");
            }
        }
        // Failed build
        else
        {
            throw std::runtime_error("Failed to build managed scripts!");
        }
    }
    std::string Application::getDotNetRuntimePath() const
    {
        // Check if any .NET Runtime is even installed
        const std::filesystem::path PATH = 
            std::filesystem::path("C:/Program Files/dotnet/shared/Microsoft.NETCore.App");
        if (!std::filesystem::exists(PATH))
            return "";

        // Check all folders in the directory to find versions
        std::pair<int, std::filesystem::path> latestVer = { -1, {} };
        for (const auto& DIR_ENTRY : std::filesystem::directory_iterator(PATH))
        {
            // Is a file, not a folder
            if (!DIR_ENTRY.is_directory())
                continue;

            // Get the directory's name
            const auto& DIR = DIR_ENTRY.path();
            const auto& DIR_NAME = (--(DIR.end()))->string();
            if (DIR_NAME.empty())
                continue;

            // Get the version number
            const int VER_NUM = DIR_NAME[0] - '0';

            // We will only naively check major version here and ignore the rest of 
            // semantic versioning to keep things simple for this sample.
            if (VER_NUM > latestVer.first)
            {
                latestVer = { VER_NUM, DIR };
            }
        }

        // Check if we found any valid versions
        if (latestVer.first >= 0)
        {
            // Replace all forward slashes with backslashes 
            // (.NET can't handle forward slashes)
            auto dotnetPath = latestVer.second.string();
            std::replace_if
            (
                dotnetPath.begin(), dotnetPath.end(), 
                [](char c){ return c == '/'; }, 
                '\\'
            );
            return dotnetPath;
        }

        return "";
    }
}
