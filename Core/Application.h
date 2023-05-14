#pragma once

#include <Windows.h>        // HMODULE
#include <string>
#include <stdexcept>
#include <iomanip>
#include <sstream>
// Relative include as this file will be included from other projects later
#include "../extern/dotnet/include/coreclrhost.h"    // coreclr_*

#include "ImportExport.h"

namespace Core
{
    class DLL_API TransformComponent
    {
      public: 
        float x = 0.0f;
        float y = 0.0f;
    };

    class DLL_API Application
    {
      public:
        static constexpr int ENTITY_COUNT = 5;
        static constexpr int MIN_ENTITY_ID = 0;
        static constexpr int MAX_ENTITY_ID = ENTITY_COUNT - 1;

        void Run();

        static void HelloWorld();
        static TransformComponent* GetComponent(int entityId);

      private:
        void startScriptEngine();
        void stopScriptEngine();
        std::string buildTpaList(const std::string& directory);

        // References to CoreCLR key components
        HMODULE coreClr = nullptr;
        void* hostHandle = nullptr;
        unsigned int domainId = 0;
        // Function Pointers to CoreCLR functions
        coreclr_initialize_ptr      initializeCoreClr = nullptr;
        coreclr_create_delegate_ptr createManagedDelegate = nullptr;
        coreclr_shutdown_ptr        shutdownCoreClr = nullptr;
        // Native Data
        static std::array<TransformComponent, ENTITY_COUNT> nativeData;

        // Helper Functions
        template<typename FunctType>
        FunctType getCoreClrFuncPtr(const std::string& functionName)
        {
            auto fPtr = reinterpret_cast<FunctType>(GetProcAddress(coreClr, functionName.c_str()));
            if (!fPtr)
                throw std::runtime_error("Unable to get pointer to function.");

            return fPtr;
        }
		template<typename FunctionType>
        FunctionType GetFunctionPtr(const std::string_view& assemblyName, const std::string_view& typeName, const std::string_view& functionName)
        {
			FunctionType managedDelegate = nullptr;
			int result = createManagedDelegate
			(
				hostHandle,
				domainId,
				assemblyName.data(),
				typeName.data(),
				functionName.data(),
				reinterpret_cast<void**>(&managedDelegate)
			);

			// Check if it failed
			if (result < 0)
			{

				std::ostringstream oss;
				oss << std::hex << std::setfill('0') << std::setw(8)
					<< "[DotNetRuntime] Failed to get pointer to function \""
					<< typeName << "." << functionName << "\" in assembly (" << assemblyName << "). "
					<< "Error 0x" << result << "\n";
				throw std::runtime_error(oss.str());
			}

			return managedDelegate;
        }
    };
}