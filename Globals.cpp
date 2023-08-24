// *****************************************************************************
    // include console logic headers
    #include "ConsoleLogic/V32Console.hpp"
    
    // include emulator headers
    #include "VideoOutput.hpp"
    #include "Globals.hpp"
    #include "Logging.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      PROGRAM OBJECTS
// =============================================================================


// instance of the Vircon virtual machine
V32::V32Console Console;

// wrappers for console I/O operation
VideoOutput Video;
V32::SPUOutputBuffer AudioBuffer;
string LoadedCartridgePath;

// libretro data structures
struct retro_hw_render_callback hw_render;

// libretro callback functions
retro_log_printf_t log_cb = nullptr;
retro_video_refresh_t video_cb = nullptr;
retro_audio_sample_t audio_cb = nullptr;
retro_audio_sample_batch_t audio_batch_cb = nullptr;
retro_environment_t environ_cb = nullptr;
retro_input_poll_t input_poll_cb = nullptr;
retro_input_state_t input_state_cb = nullptr;


// =============================================================================
//      CALLBACK FUNCTIONS FOR CONSOLE LOGIC
// =============================================================================


namespace CallbackFunctions
{
    void ClearScreen( V32::GPUColor ClearColor )
    {
        Video.ClearScreen( ClearColor );
    }
    
    // -----------------------------------------------------------------------------
    
    void DrawQuad( V32::GPUQuad& DrawnQuad )
    {
        Video.DrawTexturedQuad( DrawnQuad );
    }
    
    // -----------------------------------------------------------------------------
    
    void SetMultiplyColor( V32::GPUColor MultiplyColor )
    {
        Video.SetMultiplyColor( MultiplyColor );
    }
    
    // -----------------------------------------------------------------------------
    
    void SetBlendingMode( int NewBlendingMode )
    {
        Video.SetBlendingMode( (V32::IOPortValues)NewBlendingMode );
    }
    
    // -----------------------------------------------------------------------------
    
    void SelectTexture( int GPUTextureID )
    {
        Video.SelectTexture( GPUTextureID );
    }
    
    // -----------------------------------------------------------------------------
    
    void LoadTexture( int GPUTextureID, void* Pixels )
    {
        Video.LoadTexture( GPUTextureID, Pixels );
    }
    
    // -----------------------------------------------------------------------------
    
    void UnloadCartridgeTextures()
    {
        for( int i = 0; i < V32::Constants::GPUMaximumCartridgeTextures; i++ )
          Video.UnloadTexture( i );
    }
    
    // -----------------------------------------------------------------------------
    
    void UnloadBiosTexture()
    {
        Video.UnloadTexture( -1 );
    }
    
    // -----------------------------------------------------------------------------
    
    void LogLine( const string& Message )
    {
        LOG( Message );
    }
    
    // -----------------------------------------------------------------------------
    
    void ThrowException( const string& Message )
    {
        THROW( Message );
    }
}
