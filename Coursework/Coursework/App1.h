// Application.h
// Header file for the App1 class which handles the initialization, rendering, and post-processing of a DirectX-based application.

#ifndef _APP1_H
#define _APP1_H

// Define the light size (number of lights in the scene)
static const int lightSize = 2;

// Includes
#include <locale>
#include <codecvt>
#include "DXF.h"                 // Main DirectX framework header
#include "depth.h"               // Depth shader header
#include "LightShader.h"         // Light shader header
#include "TextureShader.h"       // Texture shader header
#include "SkyDomeShader.h"       // SkyDome shader header
#include "CloudsShader.h"        // Clouds shader header
#include "BrightnessFilterShader.h" // Brightness filter shader header
#include "GaussianBlurShader.h"  // Gaussian blur shader header
#include "BlendShader.h"         // Bloom and blend shader header
#include "ColorGradingShader.h"  // Color grading shader header
#include "SunShader.h"           // Sun shader header for sun rendering
#include "PerlinNoiseTexture.h"  // Perlin noise texture generator for perlin based terrain manipulation

// Main application class that handles initialization, rendering, and various post-processing effects.
class App1 : public BaseApplication
{
public:
    // Constructor and Destructor
    App1();                          // Initializes the App1 object
    ~App1();                         // Cleans up resources during destruction

    // Initializes the application with device context, window handle, and other settings
    void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

    // Runs the frame update (for each frame)
    bool frame();

protected:
    // Protected functions for various rendering passes and post-processing
    void finalRender(RenderTexture* renderTexture);                         // Final rendering pass
    void shadowDepth();                                                     // Render depth for shadows
    void lighting(RenderTexture* renderTexture, bool clear);                // Main lighting calculations
    void renderSunSphere();                                                 // Renders the sun sphere
    void postProcessing();                                                  // Post-processing effects
    void colorFilters(RenderTexture* source, RenderTexture* output);        // Applies color grading and filters
    void bloomPass(RenderTexture* source, RenderTexture* blendWith, RenderTexture* output); // Bloom effect pass
    void SkyBox();                                                          // Renders the skybox
    void Clouds(RenderTexture* source);                                     // Renders volumetric clouds
    bool render();                                                          // Main render loop
    void gui();                                                             // GUI rendering

    // Functions for updating lights and rendering additional passes like bloom and sun sphere post-processing
    void UpdateLights();
    void RenderBloomTexture();
    void RenderSunSpherePP();
    void BloomSunSphere();

    // Function to update positions of objects based on collision and height of others.
    void UpdatePositions();

private:
    // Shader objects
    DepthShader* linearDepthShaderTess;      // Tessellated linear depth shader (for clouds)
    DepthShader* linearDepthShader;          // linear Depth shader (for clouds)
    DepthShader* depthShaderTess;            // Tessellated depth shader
    DepthShader* depthShader;                // Depth shader
    LightShader* lightShaderTess;            // Tessellated light shader
    LightShader* lightShader;                // Light shader
    TextureShader* textureShader;            // Texture shader for basic rendering
    SkyDomeShaderClass* skyDomeShader;       // SkyDome shader for rendering sky
    CloudsShader* cloudsShader;              // Clouds shader for rendering volumetric clouds
    BrightnessFilterShader* brightnessFilterShader; // Brightness filter shader
    BrightnessFilterShader* sunBrightnessFilterShader; // Sun brightness filter shader
    GaussianBlurShader* gaussianBlurShader;  // Gaussian blur shader for blur effects
    BlendShader* blendShader;                // Shader for blending effects like bloom
	BlendShader* cloudBlendShader;		     // Shader for blending clouds with main render
    ColorGradingShader* colorFilterShader;   // Shader for color grading
    SunShader* sunShader;                    // Sun rendering shader

    // Mesh objects
    PlaneMesh* mainMesh;                // Main plane mesh for terrain or background
    PlaneMesh* cloudsPlane;             // Plane mesh for clouds
	CubeMesh* volumetricCloudBox;       // Box mesh for volumetric clouds
    AModel* spotlightModel;             // Model for spotlight
    AModel* cottageModel;               // Model for a cottage or similar object
	AModel* coinModel;			        // Model for a coin or similar object
    SphereMesh* skyDome;                // SkyDome class for rendering the sky
    SphereMesh* sunSphere;              // Sphere mesh for the sun

    // Render targets for various passes
    RenderTexture* renderTextureSource;         // Source texture for the first render pass
    RenderTexture* renderTextureSourceBloom;    // Bloom texture
    RenderTexture* renderTextureSunSphere[14];  // Sun sphere textures for various post-processing effects
    RenderTexture* renderTexturesBloom[6];      // Array of textures for bloom
    RenderTexture* renderTextureColorFilters;   // Texture for color filters
	RenderTexture* renderTextureClouds;         // Texture for clouds rendering
	RenderTexture* renderTextureCloudBlended;   // Texture for clouds blending with source
    RenderTexture* depthTexture;                // Texture for linear depth (used by clouds)
    OrthoMesh* orthoMeshFull;                   // Orthogonal mesh for full-screen rendering

    // Debug rendering
    OrthoMesh* debugOrthoMesh[2];               // Debug meshes for ortho rendering

    // Light objects
    Light* light[lightSize];                    // Array of lights in the scene

    // Shadow map objects
    ShadowMap* shadowMaps[lightSize];           // Array of shadow maps

    // Miscellaneous
    bool wireframeToggle;                       // Flag for enabling/disabling wireframe mode
    PerlinNoiseTexture* perlinNoiseTexture;     // Perlin noise texture generator
};

#endif