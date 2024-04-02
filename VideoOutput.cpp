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
    "attribute vec4 VertexInfo;                                                                 \n"
    "varying highp vec2 TextureCoordinate;                                                      \n"
    "                                                                                           \n"
    "void main()                                                                                \n"
    "{                                                                                          \n"
    "    // (1) first convert coordinates to the standard OpenGL screen space                   \n"
    "                                                                                           \n"
    "    // x is transformed from (0.0,640.0) to (-1.0,+1.0)                                    \n"
    "    gl_Position.x = (VertexInfo.x / (640.0/2.0)) - 1.0;                                    \n"
    "                                                                                           \n"
    "    // y is transformed from (0.0,360.0) to (+1.0,-1.0), so it undoes its inversion        \n"
    "    gl_Position.y = 1.0 - (VertexInfo.y / (360.0/2.0));                                    \n"
    "                                                                                           \n"
    "    // even in 2D we may also need to set z and w                                          \n"
    "    gl_Position.z = 0.0;                                                                   \n"
    "    gl_Position.w = 1.0;                                                                   \n"
    "                                                                                           \n"
    "    // (2) now texture coordinate is just provided as is to the fragment shader            \n"
    "    // (it is only needed here because fragment shaders cannot take inputs directly)       \n"
    "    TextureCoordinate = VertexInfo.zw;                                                     \n"
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
    // default values
    SelectedTexture = -1;
    QueuedQuads = 0;
    
    // all texture IDs are initially 0
    BiosTextureID = 0;
    WhiteTextureID = 0;
    
    for( int i = 0; i < Constants::GPUMaximumCartridgeTextures; i++ )
      CartridgeTextureIDs[ i ] = 0;
    
    // all OpenGL IDs are initially 0
    VAO = 0;
    VBOVertexInfo = 0;
    VBOIndices = 0;
    ShaderProgramID = 0;
    
    // initialize vertex indices; they are organized
    // assuming each quad will be given as 4 vertices,
    // as in a GL_TRIANGLE_STRIP
    for( int i = 0; i < QUAD_QUEUE_SIZE; i++ )
    {
        VertexIndices[ 6*i + 0 ] = 4*i + 0;
        VertexIndices[ 6*i + 1 ] = 4*i + 1;
        VertexIndices[ 6*i + 2 ] = 4*i + 2;
        VertexIndices[ 6*i + 3 ] = 4*i + 1;
        VertexIndices[ 6*i + 4 ] = 4*i + 2;
        VertexIndices[ 6*i + 5 ] = 4*i + 3;
    }
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
    LOG( "Compiling shader program" );
    
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
        delete[] GLInfoLog;
        
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
        delete[] GLInfoLog;
        
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
        delete[] GLInfoLog;
        
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
    
    // log basic information for the received OpenGL context
    LOG( "OpenGL context info:" );
    LOG( string("Version: ") + (char*)glGetString( GL_VERSION ) );
    LOG( string("Vendor: ") + (char*)glGetString( GL_VENDOR ) );
    LOG( string("Renderer: ") + (char*)glGetString( GL_RENDERER ) );
    LOG( string("GLSL version: ") + (char*)glGetString( GL_SHADING_LANGUAGE_VERSION ) );
    
    // compile our shader program
    LOG( "Compiling GLSL shader program" );
    ClearOpenGLErrors();
    
    if( !CompileShaderProgram() )
      THROW( "Cannot compile GLSL shader program" );
    
    // now we can enable our program
    glUseProgram( ShaderProgramID );
    
    // find the position for all our input variables within the shader program
    LOG( "Finding variables in shader program" );
    VertexInfoLocation = glGetAttribLocation( ShaderProgramID, "VertexInfo" );
    
    // find the position for all our input uniforms within the shader program
    TextureUnitLocation = glGetUniformLocation( ShaderProgramID, "TextureUnit" );
    MultiplyColorLocation = glGetUniformLocation( ShaderProgramID, "MultiplyColor" );
    
    LOG( "Creating vertex arrays and buffers" );
    
    // on a core OpenGL profile, we need this since
    // the default VAO is not valid!
    #if defined(EMUELEC) || defined(HAVE_OPENGLES2)
      glGenVertexArraysOES( 1, &VAO );
      glBindVertexArrayOES( VAO );
    #else
      glGenVertexArrays( 1, &VAO );
      glBindVertexArray( VAO );
    #endif
    
    // we will also need this for a core OpenGL
    // profile. For an OpenGL ES profile, instead,
    // it is enough to just use VAO without VBO
    glGenBuffers( 1, &VBOVertexInfo );
    glGenBuffers( 1, &VBOIndices );
    
    // bind our textures to GPU's texture unit 0
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, 0 );      // set no texture until we load one
    glEnable( GL_TEXTURE_2D );
    
    // initialize our multiply color to neutral
    SetMultiplyColor( GPUColor{ 255, 255, 255, 255 } );
    
    // initialize blending
    glEnable( GL_BLEND );
    SetBlendingMode( IOPortValues::GPUBlendingMode_Alpha );
    
    // create a white texture to draw solid color
    CreateWhiteTexture();
    
    // allocate memory for vertex info in the GPU
    LOG( "Initializing vertex info buffer" );
    glBindBuffer( GL_ARRAY_BUFFER, VBOVertexInfo );
    
    glBufferData
    (
        GL_ARRAY_BUFFER,
        sizeof( QuadVerticesInfo ),
        QuadVerticesInfo,
        GL_STREAM_DRAW
    );
    
    // define format for vertex info
    glVertexAttribPointer
    (
        VertexInfoLocation, // location (0-based index) within the shader program
        4,                  // 4 components per vertex (x,y,tex_x,tex_y)
        GL_FLOAT,           // each component is of type GLfloat
        GL_FALSE,           // do not normalize values (convert directly to fixed-point)
        0,                  // no gap between values (adjacent in memory)
        (void*)0            // starts at offset 0
    );
    
    glEnableVertexAttribArray( VertexInfoLocation );    
    
    // allocate memory for vertex indices in the GPU
    // (vertices are given as triangle strip pairs)
    LOG( "Initializing vertex index buffer" );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, VBOIndices );
    
    glBufferData
    (
        GL_ELEMENT_ARRAY_BUFFER,
        QUAD_QUEUE_SIZE * 6 * sizeof( GLushort ),
        VertexIndices,
        GL_STATIC_DRAW
    );
    
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

void VideoOutput::ReleaseTexture( GLuint& OpenGLTextureID )
{
    // this check should not be needed, but for some reason
    // glDeleteTextures will crash on MacOS without it
    if( OpenGLTextureID != 0 )
    {
        LOG( "Releasing OpenGL texture with ID = " + to_string(OpenGLTextureID) );
        glDeleteTextures( 1, &OpenGLTextureID );
    }
    
    OpenGLTextureID = 0;
}

// -----------------------------------------------------------------------------

void VideoOutput::Destroy()
{
    LOG( "Destroying video context" );
    
    // release all textures
    LOG( "Releasing all textures" );
    ReleaseTexture( WhiteTextureID );
    
    for( int i = -1; i < Constants::GPUMaximumCartridgeTextures; i++ )
      UnloadTexture( i );
    
    // delete our buffers
    LOG( "Deleting OpenGL vertex buffers" );
    glDeleteBuffers( 1, &VBOVertexInfo );
    glDeleteBuffers( 1, &VBOIndices );
    
    LOG( "Destroying OpenGL vertex arrays" );
    #if defined(EMUELEC) || defined(HAVE_OPENGLES2)
      glDeleteVertexArraysOES( 1, &VAO );
    #else
      glDeleteVertexArrays( 1, &VAO );
    #endif
    
    // delete our shader program
    LOG( "Deleting shader program" );
    glDeleteProgram( ShaderProgramID );
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
    SelectTexture( SelectedTexture );
    SetBlendingMode( BlendingMode );
    SetMultiplyColor( MultiplyColor );
    
    // tell the GPU which of its texture processors to use
    glUniform1i( TextureUnitLocation, 0 );  // texture unit 0 is for decal textures
    
    // define storage and format for vertex info
    glBindBuffer( GL_ARRAY_BUFFER, VBOVertexInfo );
    
    glVertexAttribPointer
    (
        VertexInfoLocation, // location (0-based index) within the shader program
        4,                  // 4 components per vertex (x,y,tex_x,tex_y)
        GL_FLOAT,           // each component is of type GLfloat
        GL_FALSE,           // do not normalize values (convert directly to fixed-point)
        0,                  // no gap between values (adjacent in memory)
        (void*)0            // starts at offset 0
    );
    
    glEnableVertexAttribArray( VertexInfoLocation );    
    
    // allocate memory for vertex indices in the GPU
    // (vertices are given as triangle strip pairs)
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, VBOIndices );
    
    glBufferSubData
    (
        GL_ELEMENT_ARRAY_BUFFER,
        0,
        QUAD_QUEUE_SIZE * 6 * sizeof( GLushort ),
        VertexIndices
    );
}


// =============================================================================
//      VIDEO OUTPUT: COLOR FUNCTIONS
// =============================================================================


void VideoOutput::SetMultiplyColor( GPUColor NewMultiplyColor )
{
    // we must render any pending quads before
    // applying any new render configurations
    RenderQuadQueue();
    
    MultiplyColor = NewMultiplyColor;
    
    // send our multiply color to the GPU
    glUniform4f
    (
        MultiplyColorLocation,      // location (0-based index) within the shader program
        MultiplyColor.R / 255.0,    // the 4 color components (RGBA) in range [0.0-1.0]
        MultiplyColor.G / 255.0,
        MultiplyColor.B / 255.0,
        MultiplyColor.A / 255.0
    );
}

// -----------------------------------------------------------------------------

GPUColor VideoOutput::GetMultiplyColor()
{
    return MultiplyColor;
}

// -----------------------------------------------------------------------------

void VideoOutput::SetBlendingMode( IOPortValues NewBlendingMode )
{
    // we must render any pending quads before
    // applying any new render configurations
    RenderQuadQueue();
    
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


void VideoOutput::AddQuadToQueue( const GPUQuad& Quad )
{
    // copy information from the received GPU quad
    const int SizePerQuad = 16 * sizeof( float );
    memcpy( &QuadVerticesInfo[ QueuedQuads * 16 ], &Quad.Vertices, SizePerQuad );
    
    // update the queue
    QueuedQuads++;
    
    // force queue draw if it becomes full
    if( QueuedQuads >= QUAD_QUEUE_SIZE )
      RenderQuadQueue();
}

// -----------------------------------------------------------------------------

void VideoOutput::RenderQuadQueue()
{
    if( QueuedQuads == 0 ) return;
    
    // send attributes (i.e. shader input variables)
    glBindBuffer( GL_ARRAY_BUFFER, VBOVertexInfo );
    
    // send updated vertex info to the GPU; note that
    // we would normally not update the whole buffer
    // every time, but some mobile GPUs have a bug
    // which causes very low performance on partial
    // GPU buffer updates
    glBufferSubData
    (
        GL_ARRAY_BUFFER,
        0,
        sizeof( QuadVerticesInfo ),
        QuadVerticesInfo
    );
    
    // draw each quad as 2 triangles
    glDrawElements
    (
        GL_TRIANGLES,         // independent triangles
        QueuedQuads * 6,      // number of indices
        GL_UNSIGNED_SHORT,    // format of indices
        (void*)0              // starts at offset 0
    );
    
    // reset the queue
    QueuedQuads = 0;
}

// -----------------------------------------------------------------------------

void VideoOutput::ClearScreen( GPUColor ClearColor )
{
    // temporarily replace multiply color with clear color
    GPUColor PreviousMultiplyColor = MultiplyColor;
    SetMultiplyColor( ClearColor );
    
    // bind white texture
    glBindTexture( GL_TEXTURE_2D, WhiteTextureID );
    
    // set a full-screen quad with the same texture pixel
    const GPUQuad ScreenQuad =
    {
        {
            // 4x (vertex position + texture coordinates)
            { 0, 0, 0.5, 0.5 },
            { Constants::ScreenWidth, 0, 0.5, 0.5 },
            { 0, Constants::ScreenHeight, 0.5, 0.5 },
            { Constants::ScreenWidth, Constants::ScreenHeight, 0.5, 0.5 }
        }
    };
    
    // draw this quad separately, since we are
    // using different render configurations
    AddQuadToQueue( ScreenQuad );
    RenderQuadQueue();
    
    // restore previous multiply color and texture
    SetMultiplyColor( PreviousMultiplyColor );
    SelectTexture( SelectedTexture );
}


// =============================================================================
//      VIDEO OUTPUT: TEXTURE HANDLING
// =============================================================================


void VideoOutput::LoadTexture( int GPUTextureID, void* Pixels )
{
    LOG( "Loading texture with ID = " + to_string(GPUTextureID) );
    
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
    if( GPUTextureID >= 0 )
      ReleaseTexture( CartridgeTextureIDs[ GPUTextureID ] );
    else
      ReleaseTexture( BiosTextureID );
}

// -----------------------------------------------------------------------------

void VideoOutput::SelectTexture( int GPUTextureID )
{
    // we must render any pending quads before
    // applying any new render configurations
    RenderQuadQueue();
    
    SelectedTexture = GPUTextureID;
    GLuint* OpenGLTextureID = &BiosTextureID;
    
    if( GPUTextureID >= 0 )
      OpenGLTextureID = &CartridgeTextureIDs[ GPUTextureID ];
    
    glBindTexture( GL_TEXTURE_2D, *OpenGLTextureID );
    glEnable( GL_TEXTURE_2D );
}

// -----------------------------------------------------------------------------

int32_t VideoOutput::GetSelectedTexture()
{
    return SelectedTexture;
}
