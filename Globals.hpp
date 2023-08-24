// *****************************************************************************
    // start include guard
    #ifndef GLOBALS_HPP
    #define GLOBALS_HPP
    
    // include console logic headers
    #include "ConsoleLogic/ExternalInterfaces.hpp"
    
    // include C/C++ headers
    #include <map>          // [ C++ STL ] Maps
    #include <list>         // [ C++ STL ] Lists
    #include <string>       // [ C++ STL ] Strings
    
    // include libretro headers
    #include "libretro.h"
    
    // forward declarations for all needed classes
    // (to avoid needing to include all headers here)
    namespace V32{ class V32Console; }
    class VideoOutput;
// *****************************************************************************


// =============================================================================
//      PROGRAM OBJECTS
// =============================================================================


// instance of the Vircon32 virtual machine
extern V32::V32Console Console;

// wrappers for console I/O operation
extern VideoOutput Video;
extern V32::SPUOutputBuffer AudioBuffer;
extern std::string LoadedCartridgePath;

// libretro data structures
extern struct retro_hw_render_callback hw_render;

// libretro callback functions
extern retro_log_printf_t log_cb;
extern retro_video_refresh_t video_cb;
extern retro_audio_sample_t audio_cb;
extern retro_audio_sample_batch_t audio_batch_cb;
extern retro_environment_t environ_cb;
extern retro_input_poll_t input_poll_cb;
extern retro_input_state_t input_state_cb;


// =============================================================================
//      CALLBACK FUNCTIONS FOR CONSOLE LOGIC
// =============================================================================


namespace CallbackFunctions
{
    // video functions callable by the console
    void ClearScreen( V32::GPUColor ClearColor );
    void DrawQuad( V32::GPUQuad& DrawnQuad );
    void SetMultiplyColor( V32::GPUColor MultiplyColor );
    void SetBlendingMode( int NewBlendingMode );
    void SelectTexture( int GPUTextureID );
    void LoadTexture( int GPUTextureID, void* Pixels );
    void UnloadCartridgeTextures();
    void UnloadBiosTexture();
    
    // log functions callable by the console
    void LogLine( const std::string& Message );
    void ThrowException( const std::string& Message );
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
