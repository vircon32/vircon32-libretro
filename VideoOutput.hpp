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


// =============================================================================
//      2D-SPECIALIZED OPENGL CONTEXT
// =============================================================================


class VideoOutput
{
    private:
        
        // arrays to hold buffer info
        GLfloat QuadPositionCoords[ 8 ];
        GLfloat QuadTextureCoords[ 8 ];
        
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
        GLuint VBOPositions;
        GLuint VBOTexCoords;
        GLuint VBOIndices;
        GLuint ShaderProgramID;
        
        // positions of shader parameters
        GLuint PositionsLocation;
        GLuint TexCoordsLocation;
        GLuint TextureUnitLocation;
        GLuint MultiplyColorLocation;
        
    public:
        
        // instance handling
        VideoOutput();
       ~VideoOutput();
        
        // context handling
        bool CompileShaderProgram();
        void UseShaderProgram();
        void CreateWhiteTexture();
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
        void DrawTexturedQuad( const V32::GPUQuad& Quad );
        void ClearScreen( V32::GPUColor ClearColor );
        
        // texture handling
        void SelectTexture( int GPUTextureID );
        void LoadTexture( int GPUTextureID, void* Pixels );
        void UnloadTexture( int GPUTextureID );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
