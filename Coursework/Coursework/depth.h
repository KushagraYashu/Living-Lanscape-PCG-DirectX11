// DepthShader.h
// Shader used for calculating depth values.

#pragma once

#include "DXF.h" // Include base DirectX framework.

using namespace std;
using namespace DirectX;

// DepthShader class, derived from BaseShader
class DepthShader : public BaseShader {
private:
    // Buffer for storing camera-related data
    struct CameraBuffer {
        XMFLOAT4 cameraPosition; // Camera's position in world space
    };

public:
    // Constructors for initializing depth shader with different configurations
    DepthShader(ID3D11Device* device, HWND hwnd, const wchar_t* vsFileName, const wchar_t* hsFileName, const wchar_t* dsFileName, const wchar_t* psFileName);
    DepthShader(ID3D11Device* device, HWND hwnd, const wchar_t* vsFileName, const wchar_t* psFileName);

    // Destructor for cleaning up resources
    ~DepthShader();

    // Set shader parameters for tessellated rendering
    void setShaderParametersTess(ID3D11DeviceContext* deviceContext,
        const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection,
        XMFLOAT3 camPos, ID3D11ShaderResourceView* heightMap);

    // Set shader parameters for non-tessellated rendering
    void setShaderParameters(ID3D11DeviceContext* deviceContext,
        const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection);

private:
    // Initialize the shader for tessellated rendering
    void initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename);

    // Initialize the shader for non-tessellated rendering
    void initShader(const wchar_t* vs, const wchar_t* ps);

private:
    // Constant buffers
    ID3D11Buffer* camBuffer;      // Buffer for camera-specific data
    ID3D11Buffer* matrixBuffer;   // Buffer for transformation matrices

    // Sampler state for texture sampling
    ID3D11SamplerState* sampleState;
};