// *****************************************************************************
    // include libretro headers
    #include "glsym/glsym.h"
    
    // include emulator headers
    #include "Savestates.hpp"
    #include "VideoOutput.hpp"
    #include "Globals.hpp"
    #include "Logging.hpp"
    
    // include C/C++ headers
    #include <string.h>           // [ ANSI C ] Strings
    
    // declare used namespaces
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      SERIALIZATION (SAVE CONSOLE STATE TO BUFFER)
// =============================================================================


void SaveCPUState( CPUState& State )
{
    // all fields should be adjacent in memory
    // so read registers and flags together
    memcpy( &State, Console.CPU.Registers, sizeof(CPUState) );
}

// -----------------------------------------------------------------------------

void SaveGPUState( GPUState& State )
{
    V32GPU& GPU = Console.GPU;
    
    // read all registers as adjacent
    memcpy( State.Registers, &GPU.Command, sizeof(State.Registers) );
    
    // copy only the needed cartridge textures
    unsigned TexturesSize = sizeof(GPUTexture) * GPU.LoadedCartridgeTextures;
    memcpy( State.CartridgeTextures, &GPU.CartridgeTextures[ 0 ], TexturesSize );
}

// -----------------------------------------------------------------------------

void SaveSPUState( SPUState& State )
{
    V32SPU& SPU = Console.SPU;
    
    // read all registers as adjacent
    memcpy( State.Registers, &SPU.Command, sizeof(State.Registers) );
    
    // read all channels as adjacent
    memcpy( State.Channels, &SPU.Channels, sizeof(State.Channels) );
    
    // copy only the needed cartridge sounds
    // (size will stay the same, but speed will increase)
    unsigned CartridgeSounds = SPU.LoadedCartridgeSounds;
    
    // do not read data for all sounds as a block!!
    // we don't want to copy the sample vector in each sound
    for( unsigned SoundID = 0; SoundID < CartridgeSounds; SoundID++ )
      memcpy( &State.CartridgeSounds[ SoundID ], &SPU.CartridgeSounds[ SoundID ], 4 * sizeof(int32_t) );
}

// -----------------------------------------------------------------------------

void SaveGamepadControllerState( GamepadControllerState& State )
{
    State.SelectedGamepad = Console.GamepadController.SelectedGamepad;
    
    // read all gamepad states as adjacent
    memcpy( State.GamepadStates, Console.GamepadController.RealTimeGamepadStates, sizeof(State.GamepadStates) );
}

// -----------------------------------------------------------------------------

void SaveOtherConsoleState( OtherConsoleState& State )
{
    // save state for minor chips
    memcpy( State.TimerRegisters, &Console.Timer.CurrentDate, sizeof(State.TimerRegisters) );
    State.RNGCurrentValue = Console.RNG.CurrentValue;
    
    // save the full RAM
    memcpy( State.RAM, &Console.RAM.Memory[ 0 ], sizeof(State.RAM) );
}

// -----------------------------------------------------------------------------

void SaveGameInfo( GameInfo& Info )
{
    // ensure title does not exceed 64 bytes and
    // that its unused characters are all null
    memset( Info.CartridgeTitle, 0, sizeof(Info.CartridgeTitle) );
    strncpy( Info.CartridgeTitle, Console.CartridgeController.CartridgeTitle.c_str(), sizeof(Info.CartridgeTitle) - 1 );
    
    Info.ProgramROMSize = Console.CartridgeController.MemorySize;
    Info.NumberOfTextures = Console.CartridgeController.NumberOfTextures;
    Info.NumberOfSounds = Console.CartridgeController.NumberOfSounds;
}

// -----------------------------------------------------------------------------

bool SaveState( ConsoleState* State )
{
    // save info to identify the game
    SaveGameInfo( State->Game );
    
    // save console state
    SaveCPUState( State->CPU );
    SaveGPUState( State->GPU );
    SaveSPUState( State->SPU );
    SaveGamepadControllerState( State->GamepadController );
    SaveOtherConsoleState( State->Others );
    
    // saving should never fail
    return true;
}


// =============================================================================
//      DESERIALIZATION (LOAD CONSOLE STATE FROM BUFFER)
// =============================================================================


void LoadCPUState( const CPUState& State )
{
    // all fields should be adjacent in memory
    // so write registers and flags together
    memcpy( Console.CPU.Registers, &State, sizeof(CPUState) );
}

// -----------------------------------------------------------------------------

bool LoadGPUState( const GPUState& State )
{
    V32GPU& GPU = Console.GPU;
    
    // write all registers as adjacent
    memcpy( &GPU.Command, State.Registers, sizeof(State.Registers) );
    
    // copy only the needed cartridge textures
    unsigned TexturesSize = sizeof(GPUTexture) * GPU.LoadedCartridgeTextures;
    memcpy( &GPU.CartridgeTextures[ 0 ], State.CartridgeTextures, TexturesSize );
    
    // update GPU pointers for the loaded selections
    if( GPU.SelectedTexture == -1 )
      GPU.PointedTexture = &GPU.BiosTexture;
    else
      GPU.PointedTexture = &GPU.CartridgeTextures[ GPU.SelectedTexture ];
    
    GPU.PointedRegion = &GPU.PointedTexture->Regions[ GPU.SelectedRegion ];
    
    // reset any previous OpenGL errors
    while( glGetError() != GL_NO_ERROR )
    {
        // (empty block instead of ";" to avoid warnings)
    }
    
    // make the needed updates in video output
    Video.SelectTexture( GPU.SelectedTexture );
    Video.SetMultiplyColor( GPU.MultiplyColor );
    Video.SetBlendingMode( (IOPortValues)GPU.ActiveBlending );
    
    // check for success
    return (glGetError() == GL_NO_ERROR);
}

// -----------------------------------------------------------------------------

void LoadSPUState( const SPUState& State )
{
    V32SPU& SPU = Console.SPU;
    
    // write all registers as adjacent
    memcpy( &SPU.Command, State.Registers, sizeof(State.Registers) );
    
    // write all channels as adjacent
    memcpy( &SPU.Channels, State.Channels, sizeof(State.Channels) );
    
    // copy only the needed cartridge sounds
    // (size will stay the same, but speed will increase)
    unsigned CartridgeSounds = SPU.LoadedCartridgeSounds;
    
    // do not load data for all sounds as a block!!
    // we must not overwrite the sample vector in each sound
    for( unsigned SoundID = 0; SoundID < CartridgeSounds; SoundID++ )
      memcpy( &SPU.CartridgeSounds[ SoundID ], &State.CartridgeSounds[ SoundID ], 4 * sizeof(int32_t) );
    
    // make the needed updates in audio objects
    V32Word WordValue;
    
    WordValue.AsInteger = SPU.SelectedSound;
    SPU.WritePort( (int32_t)SPU_LocalPorts::SelectedSound, WordValue );
    
    WordValue.AsInteger = SPU.SelectedChannel;
    SPU.WritePort( (int32_t)SPU_LocalPorts::SelectedChannel, WordValue );
    
    // update SPU pointers for the loaded selections
    if( SPU.SelectedSound == -1 )
      SPU.PointedSound = &SPU.BiosSound;
    else
      SPU.PointedSound = &SPU.CartridgeSounds[ SPU.SelectedSound ];
    
    SPU.PointedChannel = &SPU.Channels[ SPU.SelectedChannel ];
}

// -----------------------------------------------------------------------------

void LoadGamepadControllerState( const GamepadControllerState& State )
{
    // write the single exposed register
    Console.GamepadController.SelectedGamepad = State.SelectedGamepad;
    
    // write all gamepad states as adjacent
    memcpy( Console.GamepadController.RealTimeGamepadStates, State.GamepadStates, sizeof(State.GamepadStates) );
}

// -----------------------------------------------------------------------------

void LoadOtherConsoleState( const OtherConsoleState& State )
{
    // load state for minor chips
    memcpy( &Console.Timer.CurrentDate, State.TimerRegisters, sizeof(State.TimerRegisters) );
    Console.RNG.CurrentValue = State.RNGCurrentValue;
    
    // load the full RAM
    memcpy( &Console.RAM.Memory[ 0 ], State.RAM, sizeof(State.RAM) );
}

// -----------------------------------------------------------------------------

bool LoadState( const ConsoleState* State )
{
    // try to identify the game and see it it matches the
    // current one, to avoid loading incompatible states
    GameInfo CurrentGame;
    SaveGameInfo( CurrentGame );
    
    if( memcmp( &State->Game, &CurrentGame, sizeof(GameInfo) ) )
    {
        LOG( "ERROR: Cannot load saved state. Current cartridge is not the same one that was saved" );
        return false;
    }
    
    // load console state
    // (first do stages that cannot fail)
    LoadCPUState( State->CPU );
    LoadSPUState( State->SPU );
    LoadGamepadControllerState( State->GamepadController );
    LoadOtherConsoleState( State->Others );
    
    // now check for success at this stage
    if( !LoadGPUState( State->GPU ) )
      return false;
    
    return true;
}
