#include "BlendShader.h"

// Constructor: Initializes the shader with the vertex and pixel shader filenames.
BlendShader::BlendShader(ID3D11Device* device, HWND hwnd, const wchar_t* vsFileName, const wchar_t* psFileName) : BaseShader(device, hwnd)
{
    initShader(vsFileName, psFileName);
}

// Destructor: Releases resources like sampler state, constant buffers, and layout.
BlendShader::~BlendShader()
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

    if (screenDimenBuffer)
    {
        screenDimenBuffer->Release();
        screenDimenBuffer = 0;
    }

    // Release base shader components
    BaseShader::~BaseShader();
}

// Initialize the shader by loading the vertex and pixel shaders and setting up constant buffers.
void BlendShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
    D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;

    // Load and compile the shader files.
    loadVertexShader(vsFilename);
    loadPixelShader(psFilename);

    // Set up the description of the dynamic matrix constant buffer for the vertex shader.
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;
    renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

    // Create the blend state for additive blending.
    CD3D11_BLEND_DESC blendDesc;
    renderer->CreateBlendState(&blendDesc, &blendState);

    // Set up the texture sampler state for anisotropic filtering.
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

// Set shader parameters, including world, view, and projection matrices, and textures for blending.
void BlendShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* sourceTexture, ID3D11ShaderResourceView* bloomTexture)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;
    XMMATRIX tworld, tview, tproj;

    // Transpose matrices to prepare them for the shader.
    tworld = XMMatrixTranspose(worldMatrix);
    tview = XMMatrixTranspose(viewMatrix);
    tproj = XMMatrixTranspose(projectionMatrix);

    // Map the matrix constant buffer and send the matrices to the shader.
    result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    dataPtr = (MatrixBufferType*)mappedResource.pData;
    dataPtr->world = tworld;
    dataPtr->view = tview;
    dataPtr->projection = tproj;
    deviceContext->Unmap(matrixBuffer, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

    // Set the blend state to perform additive blending.
    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    deviceContext->OMSetBlendState(blendState, blendFactor, 0xFFFFFFFF);

    // Set the texture resources for the pixel shader.
    deviceContext->PSSetShaderResources(0, 1, &sourceTexture);
    deviceContext->PSSetShaderResources(1, 1, &bloomTexture);
    deviceContext->PSSetSamplers(0, 1, &sampleState);
}