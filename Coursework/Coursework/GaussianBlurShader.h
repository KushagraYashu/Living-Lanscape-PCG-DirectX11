#pragma once

#include "BaseShader.h" // Include the base shader functionality

using namespace std;
using namespace DirectX;

// GaussianBlurShader class, derived from BaseShader
class GaussianBlurShader : public BaseShader {
private:
    // Buffer to store screen dimensions
    struct ScreenDimensions {
        XMFLOAT4 screenDimen; // Screen width and height, along with padding
    };

public:
    // Constructor for initializing the shader
    GaussianBlurShader(ID3D11Device* device, HWND hwnd, const wchar_t* vsFileName, const wchar_t* psFileName);

    // Destructor to clean up resources
    ~GaussianBlurShader();

    // Method to set shader parameters
    void setShaderParameters(ID3D11DeviceContext* deviceContext,
        const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix,
        ID3D11ShaderResourceView* texture, XMFLOAT2 screenDimen);

private:
    // Initialize the shader with vertex and pixel shader files
    void initShader(const wchar_t* vs, const wchar_t* ps);

    // Constant buffers
    ID3D11Buffer* matrixBuffer;       // Buffer for transformation matrices
    ID3D11Buffer* screenDimenBuffer; // Buffer for screen dimensions

    // Sampler state for texture sampling
    ID3D11SamplerState* sampleState;
};