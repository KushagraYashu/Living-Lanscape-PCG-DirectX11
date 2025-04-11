#include "SkyDomeShader.h"

// Constructor that initializes the shader by loading the vertex and pixel shaders
SkyDomeShaderClass::SkyDomeShaderClass(ID3D11Device* device, HWND hwnd, const wchar_t* vsFileName, const wchar_t* psFileName) : BaseShader(device, hwnd) {
    // Initialize the shaders by loading the vertex and pixel shaders
    initShader(vsFileName, psFileName);
}

// Destructor that releases resources used by the shader
SkyDomeShaderClass::~SkyDomeShaderClass()
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

    // Release the color buffer.
    if (m_colorBuffer) {
        m_colorBuffer->Release();
        m_colorBuffer = 0;
    }

    // Release base shader components
    BaseShader::~BaseShader();
}

// Initializes the shaders and constant buffers
void SkyDomeShaderClass::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
    D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;
    D3D11_BUFFER_DESC colourBufferDesc;

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
    renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

    // Setup texture sampler state for the pixel shader
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

    // Setup the description of the color buffer used in the pixel shader
    colourBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    colourBufferDesc.ByteWidth = sizeof(ColorBufferType);
    colourBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    colourBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    colourBufferDesc.MiscFlags = 0;
    colourBufferDesc.StructureByteStride = 0;
    renderer->CreateBuffer(&colourBufferDesc, NULL, &m_colorBuffer);
}

// Sets the shader parameters (world, view, projection matrices and colors) for the pixel shader
void SkyDomeShaderClass::setShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT4 apexColour, XMFLOAT4 centreColour, XMFLOAT4 lightPos) {
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;
    XMMATRIX tworld, tview, tproj;

    // Transpose the matrices to prepare them for the shader
    tworld = XMMatrixTranspose(worldMatrix);
    tview = XMMatrixTranspose(viewMatrix);
    tproj = XMMatrixTranspose(projectionMatrix);

    // Send matrix data to the vertex shader constant buffer
    result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    dataPtr = (MatrixBufferType*)mappedResource.pData;
    dataPtr->world = tworld;  // world matrix
    dataPtr->view = tview;    // view matrix
    dataPtr->projection = tproj;  // projection matrix
    deviceContext->Unmap(matrixBuffer, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

    // Set color data in the pixel shader constant buffer
    ColorBufferType* colorPtr;
    deviceContext->Map(m_colorBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    colorPtr = (ColorBufferType*)mappedResource.pData;
    colorPtr->apexColor = apexColour;  // Apex color
    colorPtr->centerColor = centreColour;  // Center color
    colorPtr->lightPosition = lightPos;  // Light position
    deviceContext->Unmap(m_colorBuffer, 0);
    deviceContext->PSSetConstantBuffers(0, 1, &m_colorBuffer);
}