// BlendShader.h
// Shader class for handling blending operations between textures.

#pragma once

#include "BaseShader.h"  // Include the base shader class for DirectX.

using namespace std;
using namespace DirectX;

// BlendShader class definition for handling texture blending operations.
class BlendShader : public BaseShader
{
public:
    // Constructor: Initializes the shader with vertex and pixel shader files.
    BlendShader(ID3D11Device* device, HWND hwnd, const wchar_t* vsFileName, const wchar_t* psFileName);

    // Destructor: Cleans up resources allocated for the shader.
    ~BlendShader();

    // Method to set parameters for the blend shader.
    void setShaderParameters(ID3D11DeviceContext* deviceContext,
        const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection,
        ID3D11ShaderResourceView* sourceTexture, ID3D11ShaderResourceView* bloomTexture);

private:
    // Initializes the shader and its associated resources.
    void initShader(const wchar_t* vs, const wchar_t* ps);

    // Shader resources.
    ID3D11Buffer* matrixBuffer;         // Buffer for transformation matrices (world, view, projection).
    ID3D11BlendState* blendState = nullptr;  // Blend state for controlling blending behavior.
    ID3D11Buffer* screenDimenBuffer;    // Optional buffer for screen dimensions (if needed in shader).
    ID3D11SamplerState* sampleState;    // Sampler state for texture sampling.
};