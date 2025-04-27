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

    if (cloudBoxBuffer) {
        cloudBoxBuffer->Release();
        cloudBoxBuffer = 0;
    }

    if (lightBuffer) {
        lightBuffer->Release();
        lightBuffer = 0;
    }

    if (scrollBuffer) {
        scrollBuffer->Release();
        scrollBuffer = 0;
    }

    if (gasPropBuffer) {
        gasPropBuffer->Release();
        gasPropBuffer = 0;
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
    D3D11_BUFFER_DESC cloudBoxBufferDesc;
    D3D11_BUFFER_DESC lightBufferDesc;
    D3D11_BUFFER_DESC scrollBufferDesc;
    D3D11_BUFFER_DESC gasPropBufferDesc;

    // Load and compile the vertex and pixel shader files.
    loadVertexShader(vsFilename);
    loadPixelShader(psFilename);

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

    // Set up the camera data constant buffer.
    cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    cameraBufferDesc.ByteWidth = sizeof(CameraBuffer);
    cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cameraBufferDesc.MiscFlags = 0;
    cameraBufferDesc.StructureByteStride = 0;
    auto result = renderer->CreateBuffer(&cameraBufferDesc, NULL, &cameraBuffer);
    
    // Set up the cloud box data constant buffer.
    cloudBoxBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    cloudBoxBufferDesc.ByteWidth = sizeof(CloudBoxBuffer);
    cloudBoxBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cloudBoxBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cloudBoxBufferDesc.MiscFlags = 0;
    cloudBoxBufferDesc.StructureByteStride = 0;
    result = renderer->CreateBuffer(&cloudBoxBufferDesc, NULL, &cloudBoxBuffer);

    // Set up the light data constant buffer.
    lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    lightBufferDesc.ByteWidth = sizeof(LightBuffer);
    lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    lightBufferDesc.MiscFlags = 0;
    lightBufferDesc.StructureByteStride = 0;
    result = renderer->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);
    
    // Set up the scroll speed constant buffer.
    scrollBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    scrollBufferDesc.ByteWidth = sizeof(ScrollBuffer);
    scrollBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    scrollBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    scrollBufferDesc.MiscFlags = 0;
    scrollBufferDesc.StructureByteStride = 0;
    result = renderer->CreateBuffer(&scrollBufferDesc, NULL, &scrollBuffer);

    // Set up the gas properties constant buffer.
    gasPropBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    gasPropBufferDesc.ByteWidth = sizeof(GasPropBuffer);
    gasPropBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    gasPropBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    gasPropBufferDesc.MiscFlags = 0;
    gasPropBufferDesc.StructureByteStride = 0;
    result = renderer->CreateBuffer(&gasPropBufferDesc, NULL, &gasPropBuffer);
}

// Set the shader parameters for the pixel and vertex shaders, including the scroll speed and time.
void CloudsShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* depthTexture, XMFLOAT3 cameraPos, XMFLOAT3 cloudBoxCentre, XMFLOAT3 halfSize, XMFLOAT3 lightDirection, XMFLOAT4 lightColor, float sigma_s, XMFLOAT2 scrollSpeed, float time, XMFLOAT4 gasColor, XMFLOAT3 sA_SamNo_G, float gasDensity)
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

    // Send camera data to the pixel shader.
    CameraBuffer* cameraPtr;
    deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    cameraPtr = (CameraBuffer*)mappedResource.pData;
    cameraPtr->cameraPos = XMFLOAT4(cameraPos.x, cameraPos.y, cameraPos.z, 0);
    deviceContext->Unmap(cameraBuffer, 0);
    deviceContext->PSSetConstantBuffers(0, 1, &cameraBuffer);

    // Send cloud box data
    CloudBoxBuffer* cloudPtr;
    deviceContext->Map(cloudBoxBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    cloudPtr = (CloudBoxBuffer*)mappedResource.pData;
    cloudPtr->centre = XMFLOAT4(cloudBoxCentre.x, cloudBoxCentre.y, cloudBoxCentre.z, 0);
	cloudPtr->halfSize = XMFLOAT4(halfSize.x, halfSize.y, halfSize.z, 0);
    deviceContext->Unmap(cloudBoxBuffer, 0);
    deviceContext->PSSetConstantBuffers(1, 1, &cloudBoxBuffer);

    // Send light data
    LightBuffer* lightPtr;
    deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    lightPtr = (LightBuffer*)mappedResource.pData;
    lightPtr->lightColor = XMFLOAT3(lightColor.x, lightColor.y, lightColor.z);
	lightPtr->randomVal = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    lightPtr->lightDirectionAndSigma = XMFLOAT4(lightDirection.x, lightDirection.y, lightDirection.z, sigma_s);
    deviceContext->Unmap(lightBuffer, 0);
    deviceContext->PSSetConstantBuffers(2, 1, &lightBuffer);

    // Send scrolling speed data
    ScrollBuffer* scrollPtr;
    deviceContext->Map(scrollBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    scrollPtr = (ScrollBuffer*)mappedResource.pData;
    scrollPtr->scrollSpeed = scrollSpeed;
    scrollPtr->time = time;
    scrollPtr->padding = 0.f;
    deviceContext->Unmap(scrollBuffer, 0);
    deviceContext->PSSetConstantBuffers(3, 1, &scrollBuffer);

    // Send gas properties data
    GasPropBuffer* gasPropPtr;
    deviceContext->Map(gasPropBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    gasPropPtr = (GasPropBuffer*)mappedResource.pData;
    gasPropPtr->gasColour = gasColor;
    gasPropPtr->sA_SamNo_G = sA_SamNo_G;
    gasPropPtr->gasDensity = gasDensity;
    deviceContext->Unmap(gasPropBuffer, 0);
    deviceContext->PSSetConstantBuffers(4, 1, &gasPropBuffer);

    // Set shader texture and sampler resources in the pixel shader.
    deviceContext->PSSetShaderResources(0, 1, &texture);
    deviceContext->PSSetShaderResources(1, 1, &depthTexture);
    deviceContext->PSSetSamplers(0, 1, &sampleState);
}