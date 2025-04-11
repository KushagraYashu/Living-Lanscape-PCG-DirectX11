#pragma once

#include "BaseShader.h" // Include the base shader header for inheritance

using namespace std;
using namespace DirectX;

// Class definition for TextureShader, inheriting from BaseShader
class TextureShader : public BaseShader
{
public:
    // Constructor for initializing the TextureShader with a device and window handle
    TextureShader(ID3D11Device* device, HWND hwnd);

    // Destructor for cleaning up allocated resources
    ~TextureShader();

    // Function to set shader parameters like matrices and texture
    void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture);

private:
    // Function to initialize shaders from vertex and pixel shader files
    void initShader(const wchar_t* vs, const wchar_t* ps);

private:
    ID3D11Buffer* matrixBuffer; // Buffer for matrix transformations (world, view, projection)

    ID3D11BlendState* blendState = nullptr; // Blend state for controlling blending behavior

    ID3D11SamplerState* sampleState; // Sampler state for handling texture sampling
};