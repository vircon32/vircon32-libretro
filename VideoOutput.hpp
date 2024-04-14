// *****************************************************************************
    // start include guard
    #ifndef VIDEOOUTPUT_HPP
    #define VIDEOOUTPUT_HPP
    
    // include common Vircon headers
    #include "VirconDefinitions/DataStructures.hpp"
    #include "VirconDefinitions/Enumerations.hpp"
    
    // include console logic headers
    #include "ConsoleLogic/ExternalInterfaces.hpp"
    
    // include OpenGL headers
    #include "glsym/glsym.h"
// *****************************************************************************


// we will render our quads in groups using a
// fixed size queue; this parameter sets the
// queue size and acts as group size limit
#define QUAD_QUEUE_SIZE 20


// =============================================================================
//      2D-SPECIALIZED OPENGL CONTEXT
// =============================================================================


class VideoOutput
{
    private:
        
        // arrays to hold buffer info
        GLfloat QuadVerticesInfo[ 16 * QUAD_QUEUE_SIZE ];
        GLushort VertexIndices[ 6 * QUAD_QUEUE_SIZE ];
        
        // current color modifiers
        V32::GPUColor MultiplyColor;
        V32::IOPortValues BlendingMode;
        
        // OpenGL IDs of loaded textures
        GLuint BiosTextureID;
        GLuint CartridgeTextureIDs[ V32::Constants::GPUMaximumCartridgeTextures ];
        int32_t SelectedTexture;
        
        // white texture used to draw solid colors
        GLuint WhiteTextureID;
        
        // additional GL objects
        GLuint VAO;
        GLuint VBOVertexInfo;
        GLuint VBOIndices;
        GLuint ShaderProgramID;
        bool IsInitialized;
        
        // rendering control for quad groups
        int QueuedQuads;
        
        // positions of shader parameters
        GLuint VertexInfoLocation;
        GLuint TextureUnitLocation;
        GLuint MultiplyColorLocation;
        
    public:
        
        // instance handling
        VideoOutput();
       ~VideoOutput();
        
        // context handling
        bool CompileShaderProgram();
        void CreateWhiteTexture();
        void ReleaseTexture( GLuint& OpenGLTextureID );
        void InitRendering();
        void Destroy();
        
        // framebuffer render functions
        void RenderToFramebuffer();
        void BeginFrame();
        
        // color control functions
        void SetMultiplyColor( V32::GPUColor NewMultiplyColor );
        V32::GPUColor GetMultiplyColor();
        void SetBlendingMode( V32::IOPortValues BlendingMode );
        V32::IOPortValues GetBlendingMode();
        
        // render functions
        void ClearScreen( V32::GPUColor ClearColor );
        void AddQuadToQueue( const V32::GPUQuad& Quad );
        void RenderQuadQueue();
        
        // texture handling
        void LoadTexture( int GPUTextureID, void* Pixels );
        void UnloadTexture( int GPUTextureID );
        void SelectTexture( int GPUTextureID );
        int32_t GetSelectedTexture();
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
