#include "GaussianBlurShader.h"

GaussianBlurShader::GaussianBlurShader(ID3D11Device* device, HWND hwnd, const wchar_t* vsFileName, const wchar_t* psFileName) : BaseShader(device, hwnd)
{
    // Initialize the shader with vertex and pixel shader files
    initShader(vsFileName, psFileName);
}

GaussianBlurShader::~GaussianBlurShader()
{
    // Release resources like the sample state, matrix buffer, and layout
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

    if (screenDimenBuffer)
    {
        screenDimenBuffer->Release();
        screenDimenBuffer = 0;
    }

    // Release base shader components
    BaseShader::~BaseShader();
}

void GaussianBlurShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
    D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;
    D3D11_BUFFER_DESC screenDimenBufferDesc;

    // Load and compile shader files (vertex and pixel shaders)
    loadVertexShader(vsFilename);
    loadPixelShader(psFilename);

    // Set up the matrix constant buffer description (for the vertex shader)
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    // Create the matrix buffer
    renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

    // Set up the texture sampler state description (for texture sampling)
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    // Create the sampler state
    renderer->CreateSamplerState(&samplerDesc, &sampleState);

    // Set up screen dimension buffer description (for pixel shader)
    screenDimenBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    screenDimenBufferDesc.ByteWidth = sizeof(ScreenDimensions);
    screenDimenBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    screenDimenBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    screenDimenBufferDesc.MiscFlags = 0;
    screenDimenBufferDesc.StructureByteStride = 0;

    // Create the screen dimension buffer
    auto result = renderer->CreateBuffer(&screenDimenBufferDesc, NULL, &screenDimenBuffer);
}

void GaussianBlurShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture, XMFLOAT2 screenDimen)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;
    XMMATRIX tworld, tview, tproj;

    // Transpose matrices to convert them to the format expected by the shader
    tworld = XMMatrixTranspose(worldMatrix);
    tview = XMMatrixTranspose(viewMatrix);
    tproj = XMMatrixTranspose(projectionMatrix);

    // Map the matrix buffer and send the matrix data
    result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    dataPtr = (MatrixBufferType*)mappedResource.pData;
    dataPtr->world = tworld;
    dataPtr->view = tview;
    dataPtr->projection = tproj;
    deviceContext->Unmap(matrixBuffer, 0);

    // Set the matrix constant buffer to the vertex shader
    deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

    // Map the screen dimension buffer and send the screen dimensions to the pixel shader
    ScreenDimensions* screenDimensions;
    deviceContext->Map(screenDimenBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    screenDimensions = (ScreenDimensions*)mappedResource.pData;
    screenDimensions->screenDimen.x = screenDimen.x;
    screenDimensions->screenDimen.y = screenDimen.y;
    screenDimensions->screenDimen.z = 0.f;
    screenDimensions->screenDimen.w = 0.f;
    deviceContext->Unmap(screenDimenBuffer, 0);

    // Set the screen dimension constant buffer to the pixel shader
    deviceContext->PSSetConstantBuffers(0, 1, &screenDimenBuffer);

    // Set the texture resource and sampler state in the pixel shader
    deviceContext->PSSetShaderResources(0, 1, &texture);
    deviceContext->PSSetSamplers(0, 1, &sampleState);
}