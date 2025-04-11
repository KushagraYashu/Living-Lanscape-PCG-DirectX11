#pragma once

#include "BaseShader.h" // Include base shader for inheritance and shared functionality

using namespace std;
using namespace DirectX;

// Class definition for SunShader, inheriting from BaseShader
class SunShader : public BaseShader
{
private:
    // Struct for passing sun color data to the shader
    struct SunColor {
        XMFLOAT4 sunColor; // Represents the RGBA color of the sun
    };

public:
    // Constructor for initializing SunShader with a device, window handle, and shader file names
    SunShader(ID3D11Device* device, HWND hwnd, const wchar_t* vsFileName, const wchar_t* psFileName);

    // Destructor for cleaning up allocated resources
    ~SunShader();

    // Function to set shader parameters like matrices, texture, and sun color
    void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, XMFLOAT4 sunColor);

private:
    // Function to initialize vertex and pixel shaders
    void initShader(const wchar_t* vs, const wchar_t* ps);

private:
    ID3D11Buffer* matrixBuffer; // Buffer for matrix transformations (world, view, projection)
    ID3D11Buffer* sunColorBuffer; // Buffer for storing sun color data

    ID3D11BlendState* blendState = nullptr; // Blend state for controlling blending

    ID3D11SamplerState* sampleState; // Sampler state for texture sampling
};