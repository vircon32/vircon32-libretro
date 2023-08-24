// *****************************************************************************
    // include emulator headers
    #include "Logging.hpp"
    #include "Globals.hpp"
    
    // include C/C++ headers
    #include <iostream>         // [ C++ STL ] I/O Streams
    #include <stdexcept>        // [ C++ STL ] Exceptions
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//     BASIC LOGGING FUNCTIONS
// =============================================================================


void LOG( const std::string& Message )
{
    if( log_cb )
      log_cb( RETRO_LOG_INFO, "%s\n", Message.c_str()  );
  
    else
    {
        cout << Message << endl;
        if( !cout.bad() ) cout.flush();
    }
}

// -----------------------------------------------------------------------------

[[ noreturn ]] void THROW( const std::string& Message )
{
    if( log_cb )
      log_cb( RETRO_LOG_ERROR, "%s\n", Message.c_str() );
  
    else
    {
        cerr << "ERROR: " << Message << endl;
        if( !cerr.bad() ) cout.flush();
    }
    
    throw runtime_error( Message );
}
