// *****************************************************************************
    // include libretro headers
    #include "glsym/glsym.h"
    #include "libretro.h"
    
    // include Vircon32 headers
    #include "ConsoleLogic/V32Console.hpp"
    #include "VirconDefinitions/Constants.hpp"
    #include "VirconDefinitions/Enumerations.hpp"
    
    // include emulator headers
    #include "VideoOutput.hpp"
    #include "Globals.hpp"
    #include "Logging.hpp"
    
    // include C/C++ headers
    #include <stdio.h>
    #include <stdint.h>
    #include <stdlib.h>
    #include <string.h>
    #include <math.h>
    #include <time.h>
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      DATA STRUCTURES DEFINING GAMEPADS AND PORTS
// =============================================================================


// the Vircon32 gamepad is defined taking as a base the basic SNES-like
// libretro pad and selects from it only the d-pad and the needed 7 buttons
struct retro_input_descriptor controller_descriptor[] =
{
    // gamepad when connected to port 1
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "Left" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "Up" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "Down" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START, "Start" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "A" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "B" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X, "X" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y, "Y" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L, "L" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R, "R" },
    
    // gamepad when connected to port 2
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "Left" },
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "Up" },
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "Down" },
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START, "Start" },
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "A" },
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "B" },
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X, "X" },
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y, "Y" },
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L, "L" },
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R, "R" },
    
    // gamepad when connected to port 3
    { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "Left" },
    { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "Up" },
    { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "Down" },
    { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },
    { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START, "Start" },
    { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "A" },
    { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "B" },
    { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X, "X" },
    { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y, "Y" },
    { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L, "L" },
    { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R, "R" },
    
    // gamepad when connected to port 4
    { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "Left" },
    { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "Up" },
    { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "Down" },
    { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },
    { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START, "Start" },
    { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "A" },
    { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "B" },
    { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X, "X" },
    { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y, "Y" },
    { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L, "L" },
    { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R, "R" },
    
    // add a null termination to signal end of array
    { 0, 0, 0, 0, 0 }
};

// -----------------------------------------------------------------------------

// define the possible types of Vircon32 controllers
// (just one: Vircon32 only supports the standard gamepad)
const struct retro_controller_description controller_types[] =
{
    { "Vircon32 Gamepad", RETRO_DEVICE_JOYPAD }
};

// -----------------------------------------------------------------------------

// define all 4 Vircon32 controller ports (all are identical)
const struct retro_controller_info controller_ports[] =
{
    { controller_types, 1 },  // second parameter is number of types (i.e. array size)
    { controller_types, 1 },  // repeat for each of the 4 ports
    { controller_types, 1 },
    { controller_types, 1 },
    { nullptr, 0 }            // add a null termination to signal end of array
};


// =============================================================================
//      SETTING UP THE CORE ENVIRONMENT
// =============================================================================


void retro_set_controller_port_device( unsigned port, unsigned device )
{
    Console.SetGamepadConnection( port, (device == RETRO_DEVICE_JOYPAD) );
}

// -----------------------------------------------------------------------------

void retro_get_system_info( struct retro_system_info *info )
{
    memset( info, 0, sizeof( *info ) );
    info->library_name     = "Vircon32";
    info->library_version  = "2023.08.24";
    info->need_fullpath    = true;          // games can be too large to hold in memory
    info->valid_extensions = "v32|V32";     // target system may be case sensitive
}

// -----------------------------------------------------------------------------

void retro_get_system_av_info( struct retro_system_av_info *info )
{
    // video and audio frequencies
    info->timing.fps = 60;
    info->timing.sample_rate = 44100.0;
    
    // screen resolution is fixed 
    info->geometry.base_width  = V32::Constants::ScreenWidth;
    info->geometry.base_height = V32::Constants::ScreenHeight;
    info->geometry.max_width   = V32::Constants::ScreenWidth;
    info->geometry.max_height  = V32::Constants::ScreenHeight;

    // 0 means ratio = width/height
    info->geometry.aspect_ratio = 0;
}

// -----------------------------------------------------------------------------

void retro_set_environment( retro_environment_t cb )
{
    // capture the provided function
    environ_cb = cb;
    
    // we need to state that this is a core that casn be started
    // without a game! Otherwise the core will immediately unload
    bool no_rom = true;
    environ_cb( RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_rom );
    
    // define our controllers and controller ports
    environ_cb( RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, (void*)controller_descriptor );
    environ_cb( RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)controller_ports );
    
    // fill callback function for logging
    struct retro_log_callback logging;
    
    if( cb( RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging ) )
      log_cb = logging.log;
}


// =============================================================================
//      MAIN EXECUTION FUNCTION
// =============================================================================


void retro_run()
{
    input_poll_cb();
    
    // read input for all connected gamepads
    for( int Port = 0; Port < V32::Constants::GamepadPorts; Port++ )
    {
        if( !Console.HasGamepad( Port ) )
          continue;
        
        // read all controls
        Console.SetGamepadControl( Port, V32::GamepadControls::Left,        input_state_cb( Port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT  ) );
        Console.SetGamepadControl( Port, V32::GamepadControls::Right,       input_state_cb( Port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT ) );
        Console.SetGamepadControl( Port, V32::GamepadControls::Up,          input_state_cb( Port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP    ) );
        Console.SetGamepadControl( Port, V32::GamepadControls::Down,        input_state_cb( Port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN  ) );
        Console.SetGamepadControl( Port, V32::GamepadControls::ButtonStart, input_state_cb( Port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START ) );
        Console.SetGamepadControl( Port, V32::GamepadControls::ButtonA,     input_state_cb( Port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A     ) );
        Console.SetGamepadControl( Port, V32::GamepadControls::ButtonB,     input_state_cb( Port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B     ) );
        Console.SetGamepadControl( Port, V32::GamepadControls::ButtonX,     input_state_cb( Port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X     ) );
        Console.SetGamepadControl( Port, V32::GamepadControls::ButtonY,     input_state_cb( Port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y     ) );
        Console.SetGamepadControl( Port, V32::GamepadControls::ButtonL,     input_state_cb( Port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L     ) );
        Console.SetGamepadControl( Port, V32::GamepadControls::ButtonR,     input_state_cb( Port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R     ) );
    }
    
    // run the console
    if( !Console.IsPowerOn() )
      Console.SetPower( true );
    
    Video.BeginFrame();
    Console.RunNextFrame();
    
    // send this frame's video signal to libretro
    video_cb( RETRO_HW_FRAME_BUFFER_VALID, V32::Constants::ScreenWidth, V32::Constants::ScreenHeight, 0 );
    
    // send this frame's audio signal to libretro
    Console.GetFrameSoundOutput( AudioBuffer );
    audio_batch_cb( (const int16_t*)AudioBuffer.Samples, V32::Constants::SPUSamplesPerFrame );
}


// =============================================================================
//      HANDLING CONTEXT FOR CORE AND OPENGL
// =============================================================================


void context_reset()
{
    LOG( "Received signal: Reset context" );
    rglgen_resolve_symbols( hw_render.get_proc_address );
    
    // initialize video output
    Video.CompileShaderProgram();
    Video.CreateWhiteTexture();
    Video.InitRendering();
    
    // set console's video callbacks
    V32::Callbacks::ClearScreen = CallbackFunctions::ClearScreen;
    V32::Callbacks::DrawQuad = CallbackFunctions::DrawQuad;
    V32::Callbacks::SetMultiplyColor = CallbackFunctions::SetMultiplyColor;
    V32::Callbacks::SetBlendingMode = CallbackFunctions::SetBlendingMode;
    V32::Callbacks::SelectTexture = CallbackFunctions::SelectTexture;
    V32::Callbacks::LoadTexture = CallbackFunctions::LoadTexture;
    V32::Callbacks::UnloadCartridgeTextures = CallbackFunctions::UnloadCartridgeTextures;
    V32::Callbacks::UnloadBiosTexture = CallbackFunctions::UnloadBiosTexture;
    
    // set console's log callbacks
    V32::Callbacks::LogLine = CallbackFunctions::LogLine;
    V32::Callbacks::ThrowException = CallbackFunctions::ThrowException;
    
    // obtain current time
    time_t CreationTime;
    time( &CreationTime );
    struct tm* CreationTimeInfo = localtime( &CreationTime );
    
    // set console date and time
    // (Careful! C gives year counting from 1900)
    Console.SetCurrentDate( CreationTimeInfo->tm_year + 1900, CreationTimeInfo->tm_yday );
    Console.SetCurrentTime( CreationTimeInfo->tm_hour, CreationTimeInfo->tm_min, CreationTimeInfo->tm_sec );
    
    // operations that load files may throw on failure
    try
    {
        // Look for BIOS in system directory
        const char *SystemDirPath = 0;
        environ_cb( RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &SystemDirPath );
        Console.LoadBios( SystemDirPath + string("/StandardBios.v32") );
        
        // load a cartridge if it was specified
        if( !LoadedCartridgePath.empty() )
          Console.LoadCartridge( LoadedCartridgePath );
    }
    catch( const std::exception& e )
    {
        LOG( "ERROR: " + string( e.what() ) );
    }
}

// -----------------------------------------------------------------------------

void context_destroy()
{
    LOG( "Received signal: Destroy context" );
    Console.UnloadCartridge();
    Console.UnloadBios();
    Video.Destroy();
}

// -----------------------------------------------------------------------------

bool retro_init_hw_context()
{
    LOG( "Received signal: Init HW context" );
    
    // HW context info dependent on the device
    #if HAVE_OPENGLES2
      hw_render.context_type = RETRO_HW_CONTEXT_OPENGLES2;
    #else
      hw_render.context_type = RETRO_HW_CONTEXT_OPENGL_CORE;
      hw_render.version_major = 3;
      hw_render.version_minor = 1;
    #endif
    
    // fixed HW context details
    hw_render.context_reset = context_reset;
    hw_render.context_destroy = context_destroy;
    hw_render.depth = false;
    hw_render.stencil = false;
    hw_render.bottom_left_origin = true;
    
    if( !environ_cb( RETRO_ENVIRONMENT_SET_HW_RENDER, &hw_render ) )
       return false;
    
    return true;
}


// =============================================================================
//      BASIC QUERIES FROM THE FRONT-END
// =============================================================================


unsigned retro_api_version()
{
   return RETRO_API_VERSION;
}

// -----------------------------------------------------------------------------

unsigned retro_get_region()
{
    // we are not tied to any physical region,
    // but return NTSC since we use 60 fps
    return RETRO_REGION_NTSC;
}


// =============================================================================
//      OBTAINING CALLBACK FUNCTIONS FROM THE FRONT-END
// =============================================================================


void retro_set_audio_sample(retro_audio_sample_t cb)
{
    audio_cb = cb;
}

// -----------------------------------------------------------------------------

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
    audio_batch_cb = cb;
}

// -----------------------------------------------------------------------------

void retro_set_input_poll(retro_input_poll_t cb)
{
    input_poll_cb = cb;
}

// -----------------------------------------------------------------------------

void retro_set_input_state(retro_input_state_t cb)
{
   input_state_cb = cb;
}

// -----------------------------------------------------------------------------

void retro_set_video_refresh(retro_video_refresh_t cb)
{
    video_cb = cb;
}


// =============================================================================
//      GENERAL CORE OPERATION
// =============================================================================


void retro_init()
{
    LOG( "Received signal: Init" );
    // (nothing to do for this core)
}

// -----------------------------------------------------------------------------

void retro_deinit()
{
    LOG( "Received signal: Deinit" );
    // (nothing to do for this core)
}

// -----------------------------------------------------------------------------

void retro_reset()
{
    LOG( "Received signal: Reset" );
    Console.Reset();
}

// -----------------------------------------------------------------------------

bool retro_load_game( const struct retro_game_info *info )
{
    // for OpenGL we will normally want 32bpp color format
    enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
    
    if( !environ_cb( RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt ) )
    {
        LOG( "ERROR: XRGB8888 is not supported." );
        return false;
    }
    
    // initialize our context
    if( !retro_init_hw_context() )
    {
        LOG( "ERROR: HW Context could not be initialized" );
        return false;
    }
    
    // case 1: core loaded with a game
    if( info && info->path )
    {
        LOG( std::string("Core loaded with game: ") + info->path );
        LoadedCartridgePath = info->path;
    }
    
    // case 2: core loaded with no game
    // (the console will run the BIOS)
    else
    {
        LOG( "Core laded with no game" );
        LoadedCartridgePath = "";
    }
    
    return true;
}

// -----------------------------------------------------------------------------

void retro_unload_game()
{
    Console.UnloadCartridge();
}

// -----------------------------------------------------------------------------

bool retro_load_game_special( unsigned type, const struct retro_game_info *info, size_t num )
{
    // not implemented: we only use regular load game
    (void)type;
    (void)info;
    (void)num;
    return false;
}


// =============================================================================
//      HANDLING SAVED GAMES (PENDING)
// =============================================================================


void *retro_get_memory_data( unsigned id )
{
    //uint8_t *data;
    //switch(id)
    //case RETRO_MEMORY_SAVE_RAM:
    //data = Memory.SRAM;
    //break;
    
    // not implemented: state serialization is not supported
    (void)id;
    return NULL;
}

// -----------------------------------------------------------------------------

size_t retro_get_memory_size( unsigned id )
{
    /*
    switch (type)
    {
        case RETRO_MEMORY_SAVE_RAM:
          return V32::Constants::MemoryCardRAMSize;
        default:
          return 0;
    }*/
    (void)id;
    return 0;
}


// =============================================================================
//      HANDLING SAVE-STATES (NOT SUPPORTED)
// =============================================================================


size_t retro_serialize_size()
{
    // not implemented
    return 0;
}

// -----------------------------------------------------------------------------

bool retro_serialize(void *data, size_t size )
{
    // not implemented
    (void)data;
    (void)size;
    return false;
}

// -----------------------------------------------------------------------------

bool retro_unserialize( const void *data, size_t size )
{
    // not implemented
    (void)data;
    (void)size;
    return false;
}


// =============================================================================
//      HANDLING CHEAT SUPPORT (NOT SUPPORTED)
// =============================================================================


void retro_cheat_reset()
{
    // not implemented
}

// -----------------------------------------------------------------------------

void retro_cheat_set( unsigned index, bool enabled, const char *code )
{
    // not implemented
    (void)index;
    (void)enabled;
    (void)code;
}
