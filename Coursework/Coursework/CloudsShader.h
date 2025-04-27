// CloudsShader.h
// Shader class for rendering animated clouds with scrolling effects.

#pragma once

#include "DXF.h" // Include the DirectX Framework base header.

using namespace std;
using namespace DirectX;

// CloudsShader class definition for handling cloud rendering with scrolling effects
class CloudsShader : public BaseShader {
private:
    // Structure to hold Camera data
    struct CameraBuffer {
        XMFLOAT4 cameraPos;
    };

    // Structure to hold the cloud box data
    struct CloudBoxBuffer {
        XMFLOAT4 centre;    //centre
        XMFLOAT4 halfSize;  // halfSize
    };

    // Structure to hold the light data
    struct LightBuffer {
        XMFLOAT4 lightDirectionAndSigma;    // light direction and sigma value
        XMFLOAT3 lightColor;                // light diffuse color
        float randomVal;                    // a random value for the sampling position
    };

    // Structure to hold the scrolling data
    struct ScrollBuffer {
        XMFLOAT2 scrollSpeed;               // Scrolling speed in X and Y   
        float time;                         // elapsed time
        float padding;                      
    };

    // Structure to hold the gas properties data
    struct GasPropBuffer {
        XMFLOAT4 gasColour;                 // Gas color
        XMFLOAT3 sA_SamNo_G;                // sigma_a, sample number, and g value
        float gasDensity;                   // Gas density
    };

public:
    // Constructor: Initializes the shader with vertex and pixel shader files
    CloudsShader(ID3D11Device* device, HWND hwnd, const wchar_t* vsFileName, const wchar_t* psFileName);

    // Destructor: Cleans up allocated resources
    ~CloudsShader();

    // Method to set parameters for the shader
    void setShaderParameters(ID3D11DeviceContext* deviceContext,
        const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection,
        ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* depthTexture, XMFLOAT3 cameraPos, XMFLOAT3 cloudBoxCentre, XMFLOAT3 halfSize, XMFLOAT3 lightDirection, XMFLOAT4 lightColor, float sigma_s, XMFLOAT2 scrollSpeed, float time, XMFLOAT4 gasColor, XMFLOAT3 sA_SamNo_G, float gasDensity);

private:
    // Initializes the shader and its resources
    void initShader(const wchar_t* vs, const wchar_t* ps);

    // Shader resources
    ID3D11Buffer* matrixBuffer;         // Buffer for transformation matrices
    ID3D11SamplerState* sampleState;    // Sampler state for texture sampling
    ID3D11Buffer* cameraBuffer;     // Buffer for camera data
    ID3D11Buffer* cloudBoxBuffer;   // Buffer for cloud box data
    ID3D11Buffer* lightBuffer;      // Buffer for light data
    ID3D11Buffer* scrollBuffer;     // Buffer for the scrolling speed data
    ID3D11Buffer* gasPropBuffer;    // Buffer for gas properties data
};