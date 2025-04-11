#include "depth.h"

DepthShader::DepthShader(ID3D11Device* device, HWND hwnd, const wchar_t* vsFileName, const wchar_t* hsFileName, const wchar_t* dsFileName, const wchar_t* psFileName) : BaseShader(device, hwnd) {
    // Initialize the shader with vertex, hull, domain, and pixel shader files
    initShader(vsFileName, hsFileName, dsFileName, psFileName);
}

DepthShader::DepthShader(ID3D11Device* device, HWND hwnd, const wchar_t* vsFileName, const wchar_t* psFileName) : BaseShader(device, hwnd) {
    // Initialize the shader with only vertex and pixel shaders
    initShader(vsFileName, psFileName);
}

DepthShader::~DepthShader()
{
    // Release resources like sampler state, matrix buffer, layout, camera buffer
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

    if (camBuffer) {
        camBuffer->Release();
        camBuffer = 0;
    }

    // Release base shader components
    BaseShader::~BaseShader();
}

void DepthShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
    D3D11_BUFFER_DESC cameraBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;
    D3D11_BUFFER_DESC matrixBufferDesc;

    // Load the shader files (vertex and pixel shaders)
    loadVertexShader(vsFilename);
    loadPixelShader(psFilename);

    // Setup the description for the dynamic matrix constant buffer used in the vertex shader
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;
    renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

    // Set up the texture sampler state for sampling textures
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

    // Setup the camera buffer description
    cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    cameraBufferDesc.ByteWidth = sizeof(CameraBuffer);
    cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cameraBufferDesc.MiscFlags = 0;
    cameraBufferDesc.StructureByteStride = 0;
    renderer->CreateBuffer(&cameraBufferDesc, NULL, &camBuffer);
}

void DepthShader::initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename) {
    // Initialize the shader with vertex, hull, domain, and pixel shader files
    initShader(vsFilename, psFilename);
    loadHullShader(hsFilename);
    loadDomainShader(dsFilename);
}

void DepthShader::setShaderParametersTess(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, XMFLOAT3 camPos, ID3D11ShaderResourceView* heightMap)
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;

    // Transpose the matrices for shader compatibility (since shaders expect column-major order)
    XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
    XMMATRIX tview = XMMatrixTranspose(viewMatrix);
    XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);

    // Lock the constant buffer to update matrix data for vertex and domain shaders
    deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    dataPtr = (MatrixBufferType*)mappedResource.pData;
    dataPtr->world = tworld;
    dataPtr->view = tview;
    dataPtr->projection = tproj;

    // Initialize light view and projection matrices to identity (no lighting data for depth pass)
    dataPtr->lightView[0] = XMMATRIX();
    dataPtr->lightView[1] = XMMATRIX();
    dataPtr->lightProjection[0] = XMMATRIX();
    dataPtr->lightProjection[1] = XMMATRIX();

    deviceContext->Unmap(matrixBuffer, 0);
    deviceContext->HSSetConstantBuffers(0, 1, &matrixBuffer);
    deviceContext->DSSetConstantBuffers(0, 1, &matrixBuffer);

    // Update the camera position buffer for shaders
    CameraBuffer* camPtr;
    deviceContext->Map(camBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    camPtr = (CameraBuffer*)mappedResource.pData;
    camPtr->cameraPosition = XMFLOAT4(camPos.x, camPos.y, camPos.z, 0.f);
    deviceContext->Unmap(camBuffer, 0);
    deviceContext->HSSetConstantBuffers(1, 1, &camBuffer);
    //deviceContext->DSSetConstantBuffers(1, 1, &camBuffer); // Optional: if needed for domain shader

    // Set shader resources (heightmap for tessellation)
    deviceContext->DSSetShaderResources(0, 1, &heightMap);
    deviceContext->DSSetSamplers(0, 1, &sampleState);
}

void DepthShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix)
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;

    // Transpose matrices for shader compatibility
    XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
    XMMATRIX tview = XMMatrixTranspose(viewMatrix);
    XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);

    // Lock the constant buffer to update matrix data for vertex shader
    deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    dataPtr = (MatrixBufferType*)mappedResource.pData;
    dataPtr->world = tworld;
    dataPtr->view = tview;
    dataPtr->projection = tproj;
    dataPtr->lightView[0] = XMMATRIX();
    dataPtr->lightView[1] = XMMATRIX();
    dataPtr->lightProjection[0] = XMMATRIX();
    dataPtr->lightProjection[1] = XMMATRIX();

    deviceContext->Unmap(matrixBuffer, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);
}