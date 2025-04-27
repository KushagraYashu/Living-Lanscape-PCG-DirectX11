#include "TextureShader.h"

// Constructor that initializes the shader
TextureShader::TextureShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
    // Initialize the shaders by loading the vertex and pixel shaders from their respective files
    initShader(L"texture_vs.cso", L"texture_ps.cso");
}

// Destructor that releases the resources
TextureShader::~TextureShader()
{
    // Release the sampler state.
    if (sampleState)
    {
        sampleState->Release();
        sampleState = 0;
    }

    // Release the matrix constant buffer.
    if (matrixBuffer)
    {
        matrixBuffer->Release();
        matrixBuffer = 0;
    }

    // Release the layout.
    if (layout)
    {
        layout->Release();
        layout = 0;
    }

    // Release base shader components
    BaseShader::~BaseShader();
}

// Initializes the shaders and constant buffers
void TextureShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
    D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;

    // Load the vertex and pixel shaders
    loadVertexShader(vsFilename);
    loadPixelShader(psFilename);

    // Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    // Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
    renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

    // Create a blend state (used for alpha blending)
    CD3D11_BLEND_DESC blendDesc;
    renderer->CreateBlendState(&blendDesc, &blendState);

    // Create a texture sampler state description.
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    // Create the texture sampler state.
    renderer->CreateSamplerState(&samplerDesc, &sampleState);
}

// Sets the shader parameters (world, view, projection matrices and texture) for the pixel shader
void TextureShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;
    XMMATRIX tworld, tview, tproj;

    // Transpose the matrices to prepare them for the shader.
    tworld = XMMatrixTranspose(worldMatrix);
    tview = XMMatrixTranspose(viewMatrix);
    tproj = XMMatrixTranspose(projectionMatrix);

    // Send matrix data to the constant buffer
    result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    dataPtr = (MatrixBufferType*)mappedResource.pData;
    dataPtr->world = tworld; // world matrix
    dataPtr->view = tview;   // view matrix
    dataPtr->projection = tproj; // projection matrix
    deviceContext->Unmap(matrixBuffer, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

    // Set the blend state
    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    //deviceContext->OMSetBlendState(blendState, blendFactor, 0xFFFFFFFF);

    // Set shader texture and sampler resource in the pixel shader
    deviceContext->PSSetShaderResources(0, 1, &texture);
    deviceContext->PSSetSamplers(0, 1, &sampleState);
}