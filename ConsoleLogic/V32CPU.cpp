// *****************************************************************************
    // include common Vircon32 headers
    #include "../VirconDefinitions/Constants.hpp"
    #include "../VirconDefinitions/Enumerations.hpp"
    
    // include console logic headers
    #include "V32CPU.hpp"
    #include "ExternalInterfaces.hpp"
    
    // include C/C++ headers
    #include <cstring>          // [ ANSI C ] Strings
    #include <stdexcept>        // [ C++ STL ] Exceptions
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      INSTRUCTION PROCESSORS TABLES
    // =============================================================================
    
    
    typedef void (*InstructionProcessor)( V32CPU&, CPUInstruction );
    
    // -----------------------------------------------------------------------------
    
    // dispatch vector table for all 64 instructions
    const InstructionProcessor InstructionProcessorTable[] =
    {
        ProcessHLT,
        ProcessWAIT,
        ProcessJMP,
        ProcessCALL,
        ProcessRET,
        ProcessJT,
        ProcessJF,
        ProcessIEQ,
        ProcessINE,
        ProcessIGT,
        ProcessIGE,
        ProcessILT,
        ProcessILE,
        ProcessFEQ,
        ProcessFNE,
        ProcessFGT,
        ProcessFGE,
        ProcessFLT,
        ProcessFLE,
        ProcessMOV,
        ProcessLEA,
        ProcessPUSH,
        ProcessPOP,
        ProcessIN,
        ProcessOUT,
        ProcessMOVS,
        ProcessSETS,
        ProcessCMPS,
        ProcessCIF,
        ProcessCFI,
        ProcessCIB,
        ProcessCFB,
        ProcessNOT,
        ProcessAND,
        ProcessOR,
        ProcessXOR,
        ProcessBNOT,
        ProcessSHL,
        ProcessIADD,
        ProcessISUB,
        ProcessIMUL,
        ProcessIDIV,
        ProcessIMOD,
        ProcessISGN,
        ProcessIMIN,
        ProcessIMAX,
        ProcessIABS,
        ProcessFADD,
        ProcessFSUB,
        ProcessFMUL,
        ProcessFDIV,
        ProcessFMOD,
        ProcessFSGN,
        ProcessFMIN,
        ProcessFMAX,
        ProcessFABS,
        ProcessFLR,
        ProcessCEIL,
        ProcessROUND,
        ProcessSIN,
        ProcessACOS,
        ProcessATAN2,
        ProcessLOG,
        ProcessPOW
    };
    
    // -----------------------------------------------------------------------------
    
    // dispatch vector table for all 8 MOV variants
    const InstructionProcessor MOVProcessorTable[] =
    {
        ProcessMOVRegFromImm,
        ProcessMOVRegFromReg,
        ProcessMOVRegFromImmAdd,
        ProcessMOVRegFromRegAdd,
        ProcessMOVRegFromAddOff,
        ProcessMOVImmAddFromReg,
        ProcessMOVRegAddFromReg,
        ProcessMOVAddOffFromReg
    };
    
    
    // =============================================================================
    //      CLASS: V32 CPU
    // =============================================================================
    
    
    V32CPU::V32CPU()
    {
        MemoryBus = nullptr;
        ControlBus = nullptr;
    }
    
    // -----------------------------------------------------------------------------
    
    void V32CPU::Reset()
    {
        // clear general purpose registers
        memset( &Registers[ 0 ], 0, 16 * sizeof(V32Word) );
        
        // point BP and SP at the last word of RAM
        StackPointer.AsBinary = Constants::RAMFirstAddress + Constants::RAMSize - 1;
        BasePointer = StackPointer;
        
        // set program counter to start off the BIOS
        InstructionPointer.AsBinary = Constants::BiosProgramROMFirstAddress + 4;
        
        // clear state flags
        Halted = false;
        Waiting = false;
        
        // clear instruction registers
        memset( &Instruction, 0, sizeof(V32Word) );
        ImmediateValue.AsBinary = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void V32CPU::ChangeFrame()
    {
        Waiting = false;
    }
    
    // -----------------------------------------------------------------------------
    
    void V32CPU::RunNextCycle()
    {
        // fetch next instruction
        MemoryBus->ReadAddress( InstructionPointer.AsInteger++, (V32Word&)Instruction );
        
        // fetch its immediate value, if needed
        if( Instruction.UsesImmediate )
          MemoryBus->ReadAddress( InstructionPointer.AsInteger++, ImmediateValue );
        
        // run the instruction
        // (redirect to the needed specific processor)
        int32_t OpCode = Instruction.OpCode;
        
        if( OpCode == (int32_t)InstructionOpCodes::MOV )
          MOVProcessorTable[ Instruction.AddressingMode ]( *this, Instruction );
        else
          InstructionProcessorTable[ Instruction.OpCode ]( *this, Instruction );
    }
    
    // -----------------------------------------------------------------------------
    
    void V32CPU::RaiseHardwareError( CPUErrorCodes Code )
    {
        // use registers to pass values
        // (don't use stack, since it may fail)
        Registers[ 0 ].AsInteger = (int32_t)Code;
        Registers[ 1 ] = InstructionPointer;
        Registers[ 2 ].AsInstruction = Instruction;
        Registers[ 3 ] = ImmediateValue;
        
        // reset the stack, since it may have failed and
        // we will change all execution context anyway
        StackPointer.AsBinary = Constants::RAMFirstAddress + Constants::RAMSize - 1;
        BasePointer = StackPointer;
        
        // jump to BIOS handler routine
        InstructionPointer.AsInteger = Constants::BiosProgramROMFirstAddress;
        
        // abort any normal instruction processing
        throw CPUException();
    }
}
