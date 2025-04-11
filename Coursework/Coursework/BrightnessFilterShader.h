// BrightnessFilterShader.h
// Shader class for applying brightness filtering to textures.

#pragma once

#include "BaseShader.h" // Include the base shader class for DirectX.

using namespace std;
using namespace DirectX;

// BrightnessFilterShader class definition for handling brightness filtering in post-processing.
class BrightnessFilterShader : public BaseShader
{
public:
    // Constructor: Initializes the shader with vertex and pixel shader files.
    BrightnessFilterShader(ID3D11Device* device, HWND hwnd, const wchar_t* vsFileName, const wchar_t* psFileName);

    // Destructor: Cleans up allocated resources.
    ~BrightnessFilterShader();

    // Method to set parameters for the brightness filter shader.
    void setShaderParameters(ID3D11DeviceContext* deviceContext,
        const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection,
        ID3D11ShaderResourceView* texture);

private:
    // Initializes the shader and its resources.
    void initShader(const wchar_t* vs, const wchar_t* ps);

    // Shader resources.
    ID3D11Buffer* matrixBuffer;         // Buffer for transformation matrices.
    ID3D11Buffer* screenDimenBuffer;    // Optional buffer for screen dimensions (if needed in shader).
    ID3D11SamplerState* sampleState;    // Sampler state for texture sampling.
};