// ColorGradingShader.h
// Shader class for applying color grading effects including tinting, brightness, contrast, and saturation.

#pragma once

#include "BaseShader.h" // Include base shader functionality.

using namespace std;
using namespace DirectX;

// Class definition for the ColorGradingShader
class ColorGradingShader : public BaseShader {
private:
    // Structure for holding color grading parameters
    struct ColorGradingData {
        XMFLOAT4 tintColor; // Tint color with strength stored in .w
        XMFLOAT4 filters;   // Filters for brightness (.x), contrast (.y), and saturation (.z)
    };

public:
    // Constructor for initializing the shader with vertex and pixel shaders
    ColorGradingShader(ID3D11Device* device, HWND hwnd, const wchar_t* vsFileName, const wchar_t* psFileName);

    // Destructor for cleaning up resources
    ~ColorGradingShader();

    // Set parameters for the shader
    void setShaderParameters(ID3D11DeviceContext* deviceContext,
        const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection,
        ID3D11ShaderResourceView* sourceTexture,
        XMFLOAT3 tintColor, float tintStrength,
        float brightness, float contrast, float saturation);

private:
    // Initialize the shader and its components
    void initShader(const wchar_t* vs, const wchar_t* ps);

private:
    // Buffers and states
    ID3D11Buffer* matrixBuffer;          // Buffer for transformation matrices
    ID3D11Buffer* colorGradingBuffer;   // Buffer for color grading parameters
    ID3D11BlendState* blendState = nullptr; // Optional blend state for blending effects
    ID3D11SamplerState* sampleState;    // Sampler state for texture sampling
};