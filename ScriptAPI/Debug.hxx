#pragma once
#include <stdexcept>
#include <iostream>

#define SAFE_NATIVE_CALL_BEGIN try {
#define SAFE_NATIVE_CALL_END                                       \
}                                                                  \
catch (const std::exception& e)                                    \
{                                                                  \
    std::cout << "Native Exception: " << e.what() << std::endl;    \
}                                                                  \
catch (System::Exception^ e)                                       \
{                                                                  \
    System::Console::WriteLine("Managed Exception: " + e->Message);\
}                                                                  \
catch (...)                                                        \
{                                                                  \
    System::Console::WriteLine("Unknown exception caught.");       \
}