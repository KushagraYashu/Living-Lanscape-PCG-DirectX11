#include "CloudsShader.h"

// Constructor: Initializes the shader with vertex and pixel shader filenames.
CloudsShader::CloudsShader(ID3D11Device* device, HWND hwnd, const wchar_t* vsFileName, const wchar_t* psFileName) : BaseShader(device, hwnd)
{
    initShader(vsFileName, psFileName);
}

// Destructor: Releases resources such as sampler state, constant buffers, and layout.
CloudsShader::~CloudsShader()
{
    if (sampleState) {
        sampleState->Release();
        sampleState = 0;
    }

    if (matrixBuffer) {
        matrixBuffer->Release();
        matrixBuffer = 0;
    }

    if (layout) {
        layout->Release();
        layout = 0;
    }

    if (cameraBuffer) {
        cameraBuffer->Release();
        cameraBuffer = 0;
    }

    if (sphereBuffer) {
        sphereBuffer->Release();
        sphereBuffer = 0;
    }

    if (lightBuffer) {
        lightBuffer->Release();
        lightBuffer = 0;
    }

    // Release base shader components.
    BaseShader::~BaseShader();
}

// Initialize the shader: loads the shader files, sets up buffers, sampler state, and blend state.
void CloudsShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
    D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;
    D3D11_BUFFER_DESC cameraBufferDesc;
    D3D11_BUFFER_DESC sphereBufferDesc;
    D3D11_BUFFER_DESC lightBufferDesc;

    // Load and compile the vertex and pixel shader files.
    loadVertexShader(vsFilename);
    loadPixelShader(psFilename);

    // Set up blend state for transparency.
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    renderer->CreateBlendState(&blendDesc, &blendState);

    // Set up the matrix buffer description for the vertex shader.
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;
    renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

    // Create a texture sampler state description for texture filtering and addressing.
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    renderer->CreateSamplerState(&samplerDesc, &sampleState);

    // Set up the scroll data constant buffer.
    cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    cameraBufferDesc.ByteWidth = sizeof(CameraBuffer);
    cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cameraBufferDesc.MiscFlags = 0;
    cameraBufferDesc.StructureByteStride = 0;
    auto result = renderer->CreateBuffer(&cameraBufferDesc, NULL, &cameraBuffer);

    sphereBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    sphereBufferDesc.ByteWidth = sizeof(SphereBuffer);
    sphereBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    sphereBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    sphereBufferDesc.MiscFlags = 0;
    sphereBufferDesc.StructureByteStride = 0;
    result = renderer->CreateBuffer(&sphereBufferDesc, NULL, &sphereBuffer);

    lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    lightBufferDesc.ByteWidth = sizeof(LightBuffer);
    lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    lightBufferDesc.MiscFlags = 0;
    lightBufferDesc.StructureByteStride = 0;
    result = renderer->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);
}

// Set the shader parameters for the pixel and vertex shaders, including the scroll speed and time.
void CloudsShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, XMFLOAT3 cameraPos, XMFLOAT3 sphereCentre, float sphereRadius, XMFLOAT3 lightDirection, XMFLOAT4 lightColor, float sigma_s)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;
    XMMATRIX tworld, tview, tproj;

    // Transpose the matrices to prepare them for the shader.
    tworld = XMMatrixTranspose(world);
    tview = XMMatrixTranspose(view);
    tproj = XMMatrixTranspose(projection);

    // Send matrix data to the vertex shader.
    result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    dataPtr = (MatrixBufferType*)mappedResource.pData;
    dataPtr->world = tworld;
    dataPtr->view = tview;
    dataPtr->projection = tproj;
    deviceContext->Unmap(matrixBuffer, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

    // Send scroll data to the pixel shader (for cloud scrolling effect).
    CameraBuffer* cameraPtr;
    deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    cameraPtr = (CameraBuffer*)mappedResource.pData;
    cameraPtr->cameraPos = XMFLOAT4(cameraPos.x, cameraPos.y, cameraPos.z, 0);
    deviceContext->Unmap(cameraBuffer, 0);
    deviceContext->PSSetConstantBuffers(0, 1, &cameraBuffer);

    SphereBuffer* spherePtr;
    deviceContext->Map(sphereBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    spherePtr = (SphereBuffer*)mappedResource.pData;
    spherePtr->sphereCentreAndRadius = XMFLOAT4(sphereCentre.x, sphereCentre.y, sphereCentre.z, sphereRadius);
    deviceContext->Unmap(sphereBuffer, 0);
    deviceContext->PSSetConstantBuffers(1, 1, &sphereBuffer);

    LightBuffer* lightPtr;
    deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    lightPtr = (LightBuffer*)mappedResource.pData;
    lightPtr->lightColor = lightColor;
    lightPtr->lightDirectionAndSigma = XMFLOAT4(lightDirection.x, lightDirection.y, lightDirection.z, sigma_s);
    deviceContext->Unmap(lightBuffer, 0);
    deviceContext->PSSetConstantBuffers(2, 1, &lightBuffer);

    // Set blend state for proper transparency blending.
    float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    deviceContext->OMSetBlendState(blendState, blendFactor, 0xFFFFFFFF);

    // Set shader texture and sampler resources in the pixel shader.
    deviceContext->PSSetShaderResources(0, 1, &texture);
    deviceContext->PSSetSamplers(0, 1, &sampleState);
}