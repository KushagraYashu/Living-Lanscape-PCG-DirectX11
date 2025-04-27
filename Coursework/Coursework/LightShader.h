#pragma once

#include "DXF.h" // Include DirectX Framework for base shader functionalities

// Define the number of lights supported by the light shader
static const int lightSizeLightShader = 2;

using namespace std;
using namespace DirectX;

// LightShader class, derived from BaseShader
class LightShader : public BaseShader {
private:
    // Buffer to store light-related properties
    struct LightBuffer {
        XMFLOAT4 ambient[lightSizeLightShader];         // Ambient color for each light
        XMFLOAT4 diffuse[lightSizeLightShader];         // Diffuse color for each light
        XMFLOAT4 direction[lightSizeLightShader];       // Direction vector for each light
        XMFLOAT4 lightPosition[lightSizeLightShader];   // Position of each light
        XMFLOAT4 specularColour[lightSizeLightShader];  // Specular color for each light
        XMFLOAT4 specularPower[lightSizeLightShader];   // Specular power for each light
        XMFLOAT4 type[lightSizeLightShader];            // Light type: x=point, y=directional, z=spot
        XMFLOAT4 attFactors[lightSizeLightShader];      // Attenuation factors for point/spot lights
    };

    // Buffer to store camera-related properties
    struct CameraBuffer {
        XMFLOAT4 cameraPosition; // Camera position in the scene
        XMFLOAT4 params;
    };

    struct TexturingHeights {
        XMFLOAT2 grassHeights;
        XMFLOAT2 rockHeights;
        XMFLOAT2 snowHeights;
        XMFLOAT2 padding;
    };

public:
    // Constructor for tessellated shaders
    LightShader(ID3D11Device* device, HWND hwnd, const wchar_t* vsFileName, const wchar_t* hsFileName, const wchar_t* dsFileName, const wchar_t* psFileName);

    // Constructor for non-tessellated shaders
    LightShader(ID3D11Device* device, HWND hwnd, const wchar_t* vsFileName, const wchar_t* psFileName);

    // Destructor to clean up allocated resources
    ~LightShader();

    // Set shader parameters for tessellated rendering
    void setShaderParametersTess(ID3D11DeviceContext* deviceContext,
        const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection,
        ID3D11ShaderResourceView* heightMap,
        ID3D11ShaderResourceView* textureGrass, 
        ID3D11ShaderResourceView* textureRock, 
        ID3D11ShaderResourceView* textureSnow, 
        XMFLOAT2 grassHeights, 
        XMFLOAT2 rockHeights, 
        XMFLOAT2 snowHeights,
        Light* light[lightSizeLightShader],
        XMFLOAT4 lightType[lightSizeLightShader],
        XMFLOAT3 camPos,
        ID3D11ShaderResourceView* depthMap[lightSizeLightShader]);

    // Set shader parameters for non-tessellated rendering
    void setShaderParameters(ID3D11DeviceContext* deviceContext,
        const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection,
        ID3D11ShaderResourceView* texture,
        Light* light[lightSizeLightShader],
        XMFLOAT4 lightType[lightSizeLightShader],
        XMFLOAT3 camPos,
        ID3D11ShaderResourceView* depthMap[lightSizeLightShader]);

private:
    // Initialize the shader for non-tessellated rendering
    void initShader(const wchar_t* vs, const wchar_t* ps);

    // Initialize the shader for tessellated rendering
    void initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename);

    // Constant buffers
    ID3D11Buffer* matrixBuffer;       // Buffer for storing transformation matrices
    ID3D11Buffer* lightBuffer;        // Buffer for storing light data
    ID3D11Buffer* camBuffer;          // Buffer for storing camera data
    ID3D11Buffer* texHeightBuffer;    // Buffer for storing texturing height data

    // Sampler state for texture sampling
    ID3D11SamplerState* sampleState;
};