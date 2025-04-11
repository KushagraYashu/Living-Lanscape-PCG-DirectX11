#include "ColorGradingShader.h"

// Constructor: Initializes the shader with the vertex and pixel shader file names.
ColorGradingShader::ColorGradingShader(ID3D11Device* device, HWND hwnd, const wchar_t* vsFileName, const wchar_t* psFileName) : BaseShader(device, hwnd)
{
    initShader(vsFileName, psFileName);
}

// Destructor: Releases resources such as sampler state, matrix buffer, layout, and color grading buffer.
ColorGradingShader::~ColorGradingShader()
{
    if (sampleState)
    {
        sampleState->Release();
        sampleState = 0;
    }

    if (matrixBuffer)
    {
        matrixBuffer->Release();
        matrixBuffer = 0;
    }

    if (layout)
    {
        layout->Release();
        layout = 0;
    }

    if (colorGradingBuffer)
    {
        colorGradingBuffer->Release();
        colorGradingBuffer = 0;
    }

    // Release base shader components
    BaseShader::~BaseShader();
}

// Initialize the shader: Loads vertex and pixel shaders, sets up constant buffers and sampler state.
void ColorGradingShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
    D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_BUFFER_DESC colorGradingBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;

    // Load shader files (vertex and pixel shaders)
    loadVertexShader(vsFilename);
    loadPixelShader(psFilename);

    // Setup for the matrix constant buffer in the vertex shader
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;
    renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

    // Setup for the color grading constant buffer (tint and filters like brightness, contrast, and saturation)
    colorGradingBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    colorGradingBufferDesc.ByteWidth = sizeof(ColorGradingData);
    colorGradingBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    colorGradingBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    colorGradingBufferDesc.MiscFlags = 0;
    colorGradingBufferDesc.StructureByteStride = 0;
    renderer->CreateBuffer(&colorGradingBufferDesc, NULL, &colorGradingBuffer);

    // Setup the blend state (for handling transparency or blending)
    CD3D11_BLEND_DESC blendDesc;
    renderer->CreateBlendState(&blendDesc, &blendState);

    // Setup texture sampler state for anisotropic filtering and texture addressing
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    renderer->CreateSamplerState(&samplerDesc, &sampleState);
}

// Set the shader parameters, including matrices, texture, and color grading data
void ColorGradingShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* sourceTexture, XMFLOAT3 tintColor, float tintStrength, float brightness, float contrast, float saturation) {
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;
    XMMATRIX tworld, tview, tproj;

    // Transpose the matrices for compatibility with shaders
    tworld = XMMatrixTranspose(worldMatrix);
    tview = XMMatrixTranspose(viewMatrix);
    tproj = XMMatrixTranspose(projectionMatrix);

    // Send the matrix data to the vertex shader
    result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    dataPtr = (MatrixBufferType*)mappedResource.pData;
    dataPtr->world = tworld;
    dataPtr->view = tview;
    dataPtr->projection = tproj;
    deviceContext->Unmap(matrixBuffer, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

    // Set color grading data (tint color, brightness, contrast, and saturation) for the pixel shader
    ColorGradingData* colorGradingPtr;
    deviceContext->Map(colorGradingBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    colorGradingPtr = (ColorGradingData*)mappedResource.pData;
    colorGradingPtr->tintColor = XMFLOAT4(tintColor.x, tintColor.y, tintColor.z, tintStrength);
    colorGradingPtr->filters = XMFLOAT4(brightness, contrast, saturation, 0.f);
    deviceContext->Unmap(colorGradingBuffer, 0);
    deviceContext->PSSetConstantBuffers(0, 1, &colorGradingBuffer);

    // Set the blend state (for controlling how pixels are combined)
    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    deviceContext->OMSetBlendState(blendState, blendFactor, 0xFFFFFFFF);

    // Set the shader texture and sampler in the pixel shader
    deviceContext->PSSetShaderResources(0, 1, &sourceTexture);
    deviceContext->PSSetSamplers(0, 1, &sampleState);
}