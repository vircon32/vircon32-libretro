# source files to compile library V32ConsoleLogic
add_library(V32ConsoleLogic STATIC
    AuxiliaryFunctions.cpp
    ExternalInterfaces.cpp
    V32Buses.cpp
    V32CartridgeController.cpp
    V32Console.cpp
    V32CPU.cpp
    V32CPUProcessors.cpp
    V32GamepadController.cpp
    V32GPU.cpp
    V32GPUWriters.cpp
    V32Memory.cpp
    V32MemoryCardController.cpp
    V32NullController.cpp
    V32RNG.cpp
    V32SPU.cpp
    V32SPUWriters.cpp
    V32Timer.cpp)

# our C++ sources need C++11 to compile
set_property(TARGET V32ConsoleLogic PROPERTY CXX_STANDARD 11)

# under Linux this may be needed for linkage later
set_property(TARGET V32ConsoleLogic PROPERTY POSITION_INDEPENDENT_CODE ON)
