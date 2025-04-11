#pragma once

#include "DXF.h" // Include the DirectX Framework for base functionality and definitions

using namespace std;
using namespace DirectX;

// Class definition for SkyDomeShaderClass, inheriting from BaseShader
// Rastertek (2013) DirectX 11 Terrain Tutorial (Lesson 10) (code version 1) [online tutorial]. Adapted from: https://rastertek.com/tertut10.html.
class SkyDomeShaderClass : public BaseShader {
private:
    // Struct for storing sky dome color data
    struct ColorBufferType {
        XMFLOAT4 apexColor;       // The color at the apex (top) of the sky dome
        XMFLOAT4 centerColor;     // The color at the center (horizon) of the sky dome
        XMFLOAT4 lightPosition;   // The position of the light affecting the sky dome
    };

public:
    // Constructor to initialize the SkyDomeShaderClass with device, window handle, and shader file names
    SkyDomeShaderClass(ID3D11Device* device, HWND hwnd, const wchar_t* vsFileName, const wchar_t* psFileName);

    // Destructor to clean up allocated resources
    ~SkyDomeShaderClass();

    // Function to set shader parameters such as matrices and colors
    void setShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX world, XMMATRIX view, XMMATRIX projection, XMFLOAT4 apexColor, XMFLOAT4 centerColor, XMFLOAT4 lightPosition);

private:
    // Function to initialize vertex and pixel shaders
    void initShader(const wchar_t* vs, const wchar_t* ps);

    ID3D11Buffer* matrixBuffer;     // Buffer for matrix transformations (world, view, projection)
    ID3D11Buffer* m_colorBuffer;    // Buffer for storing sky dome colors and light position
};