// *****************************************************************************
    // include common Vircon headers
    #include "VirconDefinitions/Constants.hpp"
    
    // include emulator headers
    #include "VideoOutput.hpp"
    #include "Globals.hpp"
    #include "Logging.hpp"
    
    // include C/C++ headers
    #include <cstring>          // [ ANSI C ] Strings
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      LIBRETRO DEFINITIONS FOR OPENGL
// =============================================================================


// OpenGL framebuffer constants may depend on the target system
#if defined( HAVE_PSGL )
    #define RARCH_GL_FRAMEBUFFER GL_FRAMEBUFFER_OES
    #define RARCH_GL_FRAMEBUFFER_COMPLETE GL_FRAMEBUFFER_COMPLETE_OES
    #define RARCH_GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_EXT
#elif defined( OSX_PPC )
    #define RARCH_GL_FRAMEBUFFER GL_FRAMEBUFFER_EXT
    #define RARCH_GL_FRAMEBUFFER_COMPLETE GL_FRAMEBUFFER_COMPLETE_EXT
    #define RARCH_GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_EXT
#else
    #define RARCH_GL_FRAMEBUFFER GL_FRAMEBUFFER
    #define RARCH_GL_FRAMEBUFFER_COMPLETE GL_FRAMEBUFFER_COMPLETE
    #define RARCH_GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0
#endif


// =============================================================================
//      AUXILIARY FUNCTIONS TO CHECK AND LOG ERRORS
// =============================================================================


void ClearOpenGLErrors()
{
    while( true )
      if( glGetError() == GL_NO_ERROR )
        return;
}

// -----------------------------------------------------------------------------

// these OpenGL constants may not defined in some systems
#define GL_STACK_OVERFLOW  0x0503
#define GL_STACK_UNDERFLOW 0x0504
#define GL_TABLE_TOO_LARGE 0x8031

// use this instead of gluErrorString: it is easy to
// implement, and allows us to not depend on GLU/GLUT
string GLErrorString( GLenum ErrorCode )
{
    switch( ErrorCode )
    {
        // opengl 2 errors (8)
        case GL_NO_ERROR:           return "GL_NO_ERROR";
        case GL_INVALID_ENUM:       return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:      return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:  return "GL_INVALID_OPERATION";
        case GL_STACK_OVERFLOW:     return "GL_STACK_OVERFLOW";
        case GL_STACK_UNDERFLOW:    return "GL_STACK_UNDERFLOW";
        case GL_OUT_OF_MEMORY:      return "GL_OUT_OF_MEMORY";
        case GL_TABLE_TOO_LARGE:    return "GL_TABLE_TOO_LARGE";
        
        // opengl 3 errors (1)
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
        
        default:
            return "Unknown error";
  }
}

// -----------------------------------------------------------------------------

void LogOpenGLResult( const string& EntryText )
{
    GLenum Result = glGetError();
    string ResultString = GLErrorString( Result );
    
    LOG( EntryText + ": " + ResultString );
    
    if( Result != GL_NO_ERROR )
      THROW( "An OpenGL error happened" );
}


// =============================================================================
//      GLSL CODE FOR SHADERS
// =============================================================================


const string VertexShaderCode =
    "#version 100                                                                               \n"
    "                                                                                           \n"
    "attribute vec2 Position;                                                                   \n"
    "attribute vec2 InputTextureCoordinate;                                                     \n"
    "varying highp vec2 TextureCoordinate;                                                      \n"
    "                                                                                           \n"
    "void main()                                                                                \n"
    "{                                                                                          \n"
    "    // (1) first convert coordinates to the standard OpenGL screen space                   \n"
    "                                                                                           \n"
    "    // x is transformed from (0.0,640.0) to (-1.0,+1.0)                                    \n"
    "    gl_Position.x = (Position.x / (640.0/2.0)) - 1.0;                                      \n"
    "                                                                                           \n"
    "    // y is transformed from (0.0,360.0) to (+1.0,-1.0), so it undoes its inversion        \n"
    "    gl_Position.y = 1.0 - (Position.y / (360.0/2.0));                                      \n"
    "                                                                                           \n"
    "    // even in 2D we may also need to set z and w                                          \n"
    "    gl_Position.z = 0.0;                                                                   \n"
    "    gl_Position.w = 1.0;                                                                   \n"
    "                                                                                           \n"
    "    // (2) now texture coordinate is just provided as is to the fragment shader            \n"
    "    // (it is only needed here because fragment shaders cannot take inputs directly)       \n"
    "    TextureCoordinate = InputTextureCoordinate;                                            \n"
    "}                                                                                          \n";

const string FragmentShaderCode =
    "#version 100                                                                    \n"
    "                                                                                \n"
    "uniform mediump vec4 MultiplyColor;                                             \n"
    "uniform sampler2D TextureUnit;                                                  \n"
    "varying highp vec2 TextureCoordinate;                                           \n"
    "                                                                                \n"
    "void main()                                                                     \n"
    "{                                                                               \n"
    "    gl_FragColor = MultiplyColor * texture2D( TextureUnit, TextureCoordinate ); \n"
    "}                                                                               \n";


// =============================================================================
//      VIDEO OUTPUT: INSTANCE HANDLING
// =============================================================================


VideoOutput::VideoOutput()
{
    // all texture IDs are initially 0
    BiosTextureID = 0;
    
    for( int i = 0; i < Constants::GPUMaximumCartridgeTextures; i++ )
      CartridgeTextureIDs[ i ] = 0;
}

// -----------------------------------------------------------------------------

VideoOutput::~VideoOutput()
{
    Destroy();
}


// =============================================================================
//      VIDEO OUTPUT: CONTEXT HANDLING
// =============================================================================


bool VideoOutput::CompileShaderProgram()
{
    GLuint VertexShaderID = 0;
    GLuint FragmentShaderID = 0;
    int Success;
    
    ClearOpenGLErrors();
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // PART 1: Compile our vertex shader
    VertexShaderID = glCreateShader( GL_VERTEX_SHADER );
    const char *VertexShaderPointer = VertexShaderCode.c_str();
    glShaderSource( VertexShaderID, 1, &VertexShaderPointer, nullptr );
    glCompileShader( VertexShaderID );
    glGetShaderiv( VertexShaderID, GL_COMPILE_STATUS, &Success );
    
    if( !Success )
    {
        GLint GLInfoLogLength;
        glGetShaderiv( VertexShaderID, GL_INFO_LOG_LENGTH, &GLInfoLogLength );
        
        GLchar* GLInfoLog = new GLchar[ GLInfoLogLength + 1 ];
        glGetShaderInfoLog( VertexShaderID, GLInfoLogLength, nullptr, GLInfoLog );    
        
        LOG( string("ERROR: Vertex shader compilation failed: ") + (char*)GLInfoLog );
        delete GLInfoLog;
        
        glDeleteShader( VertexShaderID );
        VertexShaderID = 0;
        return false;
    }
    
    LOG( "Vertex shader compiled successfully! ID = " + to_string( VertexShaderID ) );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // PART 2: Compile our fragment shader
    FragmentShaderID = glCreateShader( GL_FRAGMENT_SHADER );
    const char *FragmentShaderPointer = FragmentShaderCode.c_str();
    glShaderSource( FragmentShaderID, 1, &FragmentShaderPointer, nullptr );
    glCompileShader( FragmentShaderID );
    glGetShaderiv( FragmentShaderID, GL_COMPILE_STATUS, &Success );
    
    if( !Success )
    {
        GLint GLInfoLogLength;
        glGetShaderiv( FragmentShaderID, GL_INFO_LOG_LENGTH, &GLInfoLogLength );
        
        GLchar* GLInfoLog = new GLchar[ GLInfoLogLength + 1 ];
        glGetShaderInfoLog( FragmentShaderID, GLInfoLogLength, nullptr, GLInfoLog );    
        
        LOG( string("ERROR: Fragment shader compilation failed: ") + (char*)GLInfoLog );
        delete GLInfoLog;
        
        glDeleteShader( VertexShaderID );
        glDeleteShader( FragmentShaderID );
        VertexShaderID = 0;
        return false;
    }
    
    LOG( "Fragment shader compiled successfully! ID = " + to_string( FragmentShaderID ) );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // PART 3: Link our compiled shaders to form a GLSL program
    ShaderProgramID = glCreateProgram();
    glAttachShader( ShaderProgramID, VertexShaderID );
    glAttachShader( ShaderProgramID, FragmentShaderID );
    glLinkProgram( ShaderProgramID );
    
    glGetProgramiv( ShaderProgramID, GL_LINK_STATUS, &Success );
    
    if( !Success )
    {
        GLint GLInfoLogLength;
        glGetShaderiv( ShaderProgramID, GL_INFO_LOG_LENGTH, &GLInfoLogLength );
        
        GLchar* GLInfoLog = new GLchar[ GLInfoLogLength + 1 ];
        glGetShaderInfoLog( ShaderProgramID, GLInfoLogLength, nullptr, GLInfoLog );    
        
        LOG( string("ERROR: Linking shader program failed: ") + (char*)GLInfoLog );
        delete GLInfoLog;
        
        glDeleteShader( VertexShaderID );
        glDeleteShader( FragmentShaderID );
        return false;
    }
    
    LOG( "Shader program linked successfully! ID = " + to_string( ShaderProgramID ) );
    
    // clean-up temporary compilation objects
    glDetachShader( ShaderProgramID, VertexShaderID );
    glDetachShader( ShaderProgramID, FragmentShaderID );
    glDeleteShader( VertexShaderID );
    glDeleteShader( FragmentShaderID );
    
    return true;
}

// -----------------------------------------------------------------------------

void VideoOutput::UseShaderProgram()
{
    glUseProgram( ShaderProgramID );
}

// -----------------------------------------------------------------------------

void VideoOutput::InitRendering()
{
    LOG( "Initializing rendering" );
    
    // initialize blending
    glEnable( GL_BLEND );
    SetBlendingMode( IOPortValues::GPUBlendingMode_Alpha );
    
    // compile our shader program
    LOG( "Compiling GLSL shader program" );
    ClearOpenGLErrors();
    
    if( !CompileShaderProgram() )
      THROW( "Cannot compile GLSL shader program" );
    
    // now we can enable our program
    glUseProgram( ShaderProgramID );
    
    // find the position for all our input variables within the shader program
    PositionsLocation = glGetAttribLocation( ShaderProgramID, "Position" );
    TexCoordsLocation = glGetAttribLocation( ShaderProgramID, "InputTextureCoordinate" );
    
    // find the position for all our input uniforms within the shader program
    TextureUnitLocation = glGetUniformLocation( ShaderProgramID, "TextureUnit" );
    MultiplyColorLocation = glGetUniformLocation( ShaderProgramID, "MultiplyColor" );
    
    // on a core OpenGL profile, we need this since
    // the default VAO is not valid!
    glGenVertexArrays( 1, &VAO );
    glBindVertexArray( VAO );
    
    // we will also need this for a core OpenGL
    // profile. For an OpenGL ES profile, instead,
    // it is enough to just use VAO without VBO
    glGenBuffers( 1, &VBOPositions );
    glGenBuffers( 1, &VBOTexCoords );
    
    // bind our textures to GPU's texture unit 0
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, 0 );      // set no texture until we load one
    glEnable( GL_TEXTURE_2D );
    
    // initialize our multiply color to neutral
    SetMultiplyColor( GPUColor{ 255, 255, 255, 255 } );
    
    // create a white texture to draw solid color
    CreateWhiteTexture();
    
    // allocate memory for vertex positions in the GPU
    glBindBuffer( GL_ARRAY_BUFFER, VBOPositions );
    
    glBufferData
    (
        GL_ARRAY_BUFFER,
        8 * sizeof( GLfloat ),
        QuadPositionCoords,
        GL_DYNAMIC_DRAW
    );
    
    // define format for vertex positions
    glVertexAttribPointer
    (
        PositionsLocation,  // location (0-based index) within the shader program
        2,                  // 2 components per vertex (x,y)
        GL_FLOAT,           // each component is of type GLfloat
        GL_FALSE,           // do not normalize values (convert directly to fixed-point)
        0,                  // no gap between values (adjacent in memory)
        nullptr             // pointer to the array
    );
    
    glEnableVertexAttribArray( PositionsLocation );
    
    // allocate memory for texture coordinates in the GPU
    glBindBuffer( GL_ARRAY_BUFFER, VBOTexCoords );
    
    glBufferData
    (
        GL_ARRAY_BUFFER,
        8 * sizeof( GLfloat ),
        QuadTextureCoords,
        GL_DYNAMIC_DRAW
    );
    
    // define format for texture coordinates
    glVertexAttribPointer
    (
        TexCoordsLocation,  // location (0-based index) within the shader program
        2,                  // 2 components per vertex (u,v)
        GL_FLOAT,           // each component is of type GLFloat
        GL_FALSE,           // do not normalize values (convert directly to fixed-point)
        0,                  // no gap between values (adjacent in memory)
        nullptr             // pointer to the array
    );
    
    glEnableVertexAttribArray( TexCoordsLocation );
    
    LOG( "Finished initializing rendering" );
}

// -----------------------------------------------------------------------------

void VideoOutput::CreateWhiteTexture()
{
    LOG( "Creating white texture" );
    ClearOpenGLErrors();
    
    // create new texture ID
    glGenTextures( 1, &WhiteTextureID );
    glBindTexture( GL_TEXTURE_2D, WhiteTextureID );
    
    // create our texture from 1 single white pixel
    uint8_t WhitePixel[ 4 ] = { 255, 255, 255, 255 };
    
    glTexImage2D
    (
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        1, 1,       // width, height
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        WhitePixel
    );
    
    // check correct conversion
    if( glGetError() != GL_NO_ERROR )
      THROW( "Could not create 1x1 white texture to draw solid colors" );
    
    // configure the texture
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    
    LOG( "Finished creating white texture" );
}

// -----------------------------------------------------------------------------

void VideoOutput::Destroy()
{
    // release all textures
    UnloadTexture( BiosTextureID );
    UnloadTexture( WhiteTextureID );
    
    for( int i = 0; i < Constants::GPUMaximumCartridgeTextures; i++ )
      if( CartridgeTextureIDs[ i ] != 0 )
        UnloadTexture( CartridgeTextureIDs[ i ] );
}


// =============================================================================
//      VIDEO OUTPUT: FRAMEBUFFER RENDER FUNCTIONS
// =============================================================================


void VideoOutput::RenderToFramebuffer()
{
    // select libretro's framebuffer as the render target
    glBindFramebuffer( RARCH_GL_FRAMEBUFFER, hw_render.get_current_framebuffer() );
    
    // map viewport's rectangle to the framebuffer's screen area
    glViewport( 0, 0, Constants::ScreenWidth, Constants::ScreenHeight );
}

// -----------------------------------------------------------------------------

void VideoOutput::BeginFrame()
{
    UseShaderProgram();
    RenderToFramebuffer();
    glEnable( GL_BLEND );
    SetBlendingMode( BlendingMode );
}


// =============================================================================
//      VIDEO OUTPUT: COLOR FUNCTIONS
// =============================================================================


void VideoOutput::SetMultiplyColor( GPUColor NewMultiplyColor )
{
    MultiplyColor = NewMultiplyColor;
}

// -----------------------------------------------------------------------------

GPUColor VideoOutput::GetMultiplyColor()
{
    return MultiplyColor;
}

// -----------------------------------------------------------------------------

void VideoOutput::SetBlendingMode( IOPortValues NewBlendingMode )
{
    switch( NewBlendingMode )
    {
        case IOPortValues::GPUBlendingMode_Alpha:
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glBlendEquation( GL_FUNC_ADD );
            break;
            
        case IOPortValues::GPUBlendingMode_Add:
            glBlendFunc( GL_SRC_ALPHA, GL_ONE );
            glBlendEquation( GL_FUNC_ADD );
            break;
            
        case IOPortValues::GPUBlendingMode_Subtract:
            glBlendFunc( GL_SRC_ALPHA, GL_ONE );
            glBlendEquation( GL_FUNC_REVERSE_SUBTRACT );
            break;
        
        default:
            // ignore invalid values
            return;
    }
    
    BlendingMode = NewBlendingMode;
}

// -----------------------------------------------------------------------------

IOPortValues VideoOutput::GetBlendingMode()
{
    return BlendingMode;
}


// =============================================================================
//      VIDEO OUTPUT: BASE RENDER FUNCTIONS
// =============================================================================


void VideoOutput::DrawTexturedQuad( const GPUQuad& Quad )
{
    // copy information from the received GPU quad
    memcpy( QuadPositionCoords, Quad.VertexPositions, 8 * sizeof( float ) );
    memcpy( QuadTextureCoords, Quad.VertexTexCoords, 8 * sizeof( float ) );
    
    // PART 1: Update uniforms (i.e. shader globals)
    // - - - - - - - - - - - - - - - - - - - - - - - -
    
    // tell the GPU which of its texture processors to use
    glUniform1i( TextureUnitLocation, 0 );  // texture unit 0 is for decal textures
    
    // send our multiply color to the GPU
    glUniform4f
    (
        MultiplyColorLocation,      // location (0-based index) within the shader program
        MultiplyColor.R / 255.0,    // the 4 color components (RGBA) in range [0.0-1.0]
        MultiplyColor.G / 255.0,
        MultiplyColor.B / 255.0,
        MultiplyColor.A / 255.0
    );
    
    // PART 2: Send attributes (i.e. shader input variables)
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    glBindBuffer( GL_ARRAY_BUFFER, VBOPositions );

    // send updated vertex positions to the GPU
    glBufferSubData
    (
        GL_ARRAY_BUFFER,
        0,
        8 * sizeof( GLfloat ),
        QuadPositionCoords
    );
    
    // define storage and format for vertex positions
    glVertexAttribPointer
    (
        PositionsLocation,  // location (0-based index) within the shader program
        2,                  // 2 components per vertex (x,y)
        GL_FLOAT,           // each component is of type GLfloat
        GL_FALSE,           // do not normalize values (convert directly to fixed-point)
        0,                  // no gap between values (adjacent in memory)
        nullptr             // pointer to the array
    );
    
    glEnableVertexAttribArray( PositionsLocation );
    
    // send updated vertex texture coordinates to the GPU
    glBindBuffer( GL_ARRAY_BUFFER, VBOTexCoords );
    
    glBufferSubData
    (
        GL_ARRAY_BUFFER,
        0,
        8 * sizeof( GLfloat ),
        QuadTextureCoords
    );
    
    // define storage and format for texture coordinates
    glVertexAttribPointer
    (
        TexCoordsLocation,  // location (0-based index) within the shader program
        2,                  // 2 components per vertex (u,v)
        GL_FLOAT,           // each component is of type GLFloat
        GL_FALSE,           // do not normalize values (convert directly to fixed-point)
        0,                  // no gap between values (adjacent in memory)
        nullptr             // pointer to the array
    );
    
    glEnableVertexAttribArray( TexCoordsLocation );
    
    // PART 3: Draw geometry
    // - - - - - - - - - - - - -
    
    // draw the quad as 2 connected triangles
    glDrawArrays
    (
        GL_TRIANGLE_STRIP,  // determines order and interpretation of succesive vertices
        0,                  // begin from the first index
        4                   // use 4 consecutive indices
    );
}

// -----------------------------------------------------------------------------

void VideoOutput::ClearScreen( GPUColor ClearColor )
{
    // temporarily replace multiply color with clear color
    GPUColor PreviousMultiplyColor = MultiplyColor;
    MultiplyColor = ClearColor;
    
    // bind white texture
    glBindTexture( GL_TEXTURE_2D, WhiteTextureID );
    
    // set a full-screen quad with the same texture pixel
    const GPUQuad ScreenQuad =
    {
        // vertex positions
        {
            { 0, 0 },
            { Constants::ScreenWidth, 0 },
            { 0, Constants::ScreenHeight },
            { Constants::ScreenWidth, Constants::ScreenHeight }
        },
        
        // texture coordinates
        {
            { 0.5, 0.5 },
            { 0.5, 0.5 },
            { 0.5, 0.5 },
            { 0.5, 0.5 }
        }
    };
    
    // draw quad as "textured"
    DrawTexturedQuad( ScreenQuad );
    
    // restore previous multiply color
    MultiplyColor = PreviousMultiplyColor;
}


// =============================================================================
//      VIDEO OUTPUT: TEXTURE HANDLING
// =============================================================================


void VideoOutput::LoadTexture( int GPUTextureID, void* Pixels )
{
    GLuint* OpenGLTextureID = &BiosTextureID;
    
    if( GPUTextureID >= 0 )
      OpenGLTextureID = &CartridgeTextureIDs[ GPUTextureID ];
    
    // create a new OpenGL texture and select it
    glGenTextures( 1, OpenGLTextureID );
    glBindTexture( GL_TEXTURE_2D, *OpenGLTextureID );
    
    // check correct texture ID
    if( !OpenGLTextureID )
      THROW( "OpenGL failed to generate a new texture" );
    
    // clear OpenGL errors
    glGetError();
    
    // create an OpenGL texture from the received pixel data
    glTexImage2D
    (
        GL_TEXTURE_2D,              // texture is a 2D rectangle
        0,                          // level of detail (0 = normal size)
        GL_RGBA,                    // color components in the texture
        Constants::GPUTextureSize,  // texture width in pixels
        Constants::GPUTextureSize,  // texture height in pixels
        0,                          // border width (must be 0 or 1)
        GL_RGBA,                    // color components in the source
        GL_UNSIGNED_BYTE,           // each color component is a byte
        Pixels                      // buffer storing the texture data
    );
    
    // check correct conversion
    if( glGetError() != GL_NO_ERROR )
      THROW( "Could not create an OpenGL texture from pixel data" );
    
    // textures must be scaled using only nearest neighbour
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );         
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    
    // out-of-texture coordinates must clamp, not wrap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
}

// -----------------------------------------------------------------------------

void VideoOutput::UnloadTexture( int GPUTextureID )
{
    GLuint* OpenGLTextureID = &BiosTextureID;
    
    if( GPUTextureID >= 0 )
      OpenGLTextureID = &CartridgeTextureIDs[ GPUTextureID ];
    
    glDeleteTextures( 1, OpenGLTextureID );
    *OpenGLTextureID = 0;
}

// -----------------------------------------------------------------------------

void VideoOutput::SelectTexture( int GPUTextureID )
{
    GLuint* OpenGLTextureID = &BiosTextureID;
    
    if( GPUTextureID >= 0 )
      OpenGLTextureID = &CartridgeTextureIDs[ GPUTextureID ];
    
    glBindTexture( GL_TEXTURE_2D, *OpenGLTextureID );
    glEnable( GL_TEXTURE_2D );
}
