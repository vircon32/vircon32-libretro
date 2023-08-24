// *****************************************************************************
    // start include guard
    #ifndef LOGGING_HPP
    #define LOGGING_HPP
    
    // include C/C++ headers
    #include <string>		    // [ C++ STL ] Strings
// *****************************************************************************


// =============================================================================
//     BASIC LOGGING FUNCTIONS
// =============================================================================


// general use funcions for the global log
void LOG( const std::string& Message );
[[ noreturn ]] void THROW( const std::string& Message );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
