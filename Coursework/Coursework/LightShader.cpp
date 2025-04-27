#include "LightShader.h"

// Constructor for the LightShader class that initializes shaders based on the provided filenames.
// This version also loads Hull and Domain shaders (for tessellation).
LightShader::LightShader(ID3D11Device* device, HWND hwnd, const wchar_t* vsFileName, const wchar_t* hsFileName, const wchar_t* dsFileName, const wchar_t* psFileName) : BaseShader(device, hwnd) {
	initShader(vsFileName, hsFileName, dsFileName, psFileName);
}

// Constructor for the LightShader class when Hull and Domain shaders are not needed.
LightShader::LightShader(ID3D11Device* device, HWND hwnd, const wchar_t* vsFileName, const wchar_t* psFileName) : BaseShader(device, hwnd) {
	initShader(vsFileName, psFileName);
}

// Destructor that releases all the resources allocated by the shader class.
LightShader::~LightShader()
{
	// Release sampler state.
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}

	// Release matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	// Release the input layout used by the shaders.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	// Release the light constant buffer.
	if (lightBuffer)
	{
		lightBuffer->Release();
		lightBuffer = 0;
	}

	// Release the camera constant buffer.
	if (camBuffer) {
		camBuffer->Release();
		camBuffer = 0;
	}

	// Release base shader components.
	BaseShader::~BaseShader();
}

// Initializes the shader by loading vertex and pixel shaders, and setting up the necessary constant buffers.
void LightShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_BUFFER_DESC texHeightBufferDesc;

	// Load vertex and pixel shaders from the provided files.
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup the matrix constant buffer description for use in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	// Create a texture sampler state for texture filtering.
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

	// Setup the description for the light constant buffer used by the pixel shader.
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBuffer);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	auto result = renderer->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);

	// Setup the description for the camera constant buffer.
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBuffer);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&cameraBufferDesc, NULL, &camBuffer);
	
	texHeightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	texHeightBufferDesc.ByteWidth = sizeof(TexturingHeights);
	texHeightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	texHeightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	texHeightBufferDesc.MiscFlags = 0;
	texHeightBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&texHeightBufferDesc, NULL, &texHeightBuffer);
}

void LightShader::initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename) {
	// Calls the initShader function for vertex and pixel shaders, and then loads hull and domain shaders.
	initShader(vsFilename, psFilename);
	loadHullShader(hsFilename);
	loadDomainShader(dsFilename);
}

void LightShader::setShaderParametersTess(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* heightMap, ID3D11ShaderResourceView* textureGrass, ID3D11ShaderResourceView* textureRock, ID3D11ShaderResourceView* textureSnow, XMFLOAT2 grassHeights, XMFLOAT2 rockHeights, XMFLOAT2 snowHeights, Light* light[lightSizeLightShader], XMFLOAT4 lightType[lightSizeLightShader], XMFLOAT3 camPos, ID3D11ShaderResourceView* depthMap[lightSizeLightShader]) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;

	// Transpose matrices for shader compatibility (HLSL expects column-major format).
	XMMATRIX tworld = XMMatrixTranspose(world);
	XMMATRIX tview = XMMatrixTranspose(view);
	XMMATRIX tproj = XMMatrixTranspose(projection);

	// Preparing light view and projection matrices for each light source.
	XMMATRIX tLightViewMatrix[lightSizeLightShader];
	XMMATRIX tLightProjectionMatrix[lightSizeLightShader];
	for (int i = 0; i < lightSizeLightShader; i++) {
		tLightViewMatrix[i] = XMMatrixTranspose(light[i]->getViewMatrix());
		if (lightType[i].y == 1.f)
			tLightProjectionMatrix[i] = XMMatrixTranspose(light[i]->getOrthoMatrix());
		else
			tLightProjectionMatrix[i] = XMMatrixTranspose(light[i]->getProjectionMatrix());
	}

	// Lock the matrix constant buffer for writing.
	deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;
	dataPtr->view = tview;
	dataPtr->projection = tproj;

	// Store light matrices.
	for (int i = 0; i < lightSizeLightShader; i++) {
		dataPtr->lightView[i] = tLightViewMatrix[i];
		dataPtr->lightProjection[i] = tLightProjectionMatrix[i];
	}

	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->HSSetConstantBuffers(0, 1, &matrixBuffer);
	deviceContext->DSSetConstantBuffers(0, 1, &matrixBuffer);

	TexturingHeights* texHPtr;
	deviceContext->Map(texHeightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	texHPtr = (TexturingHeights*)mappedResource.pData;
	texHPtr->grassHeights = grassHeights;
	texHPtr->rockHeights = rockHeights;
	texHPtr->snowHeights = snowHeights;
	deviceContext->Unmap(texHeightBuffer, 0);
	deviceContext->PSSetConstantBuffers(1, 1, &texHeightBuffer);

	// Setting camera parameters for vertex and domain shaders.
	CameraBuffer* camPtr;
	deviceContext->Map(camBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	camPtr = (CameraBuffer*)mappedResource.pData;
	camPtr->cameraPosition = XMFLOAT4(camPos.x, camPos.y, camPos.z, 0.f);
	deviceContext->Unmap(camBuffer, 0);
	deviceContext->HSSetConstantBuffers(1, 1, &camBuffer);
	deviceContext->DSSetConstantBuffers(2, 1, &camBuffer);

	// Setting light parameters for pixel shader.
	LightBuffer* lightPtr;
	deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBuffer*)mappedResource.pData;

	for (int i = 0; i < lightSizeLightShader; i++) {
		lightPtr->ambient[i] = light[i]->getAmbientColour();
		lightPtr->diffuse[i] = light[i]->getDiffuseColour();
		lightPtr->direction[i] = XMFLOAT4(light[i]->getDirection().x, light[i]->getDirection().y, light[i]->getDirection().z, 0.0f);
		lightPtr->lightPosition[i] = XMFLOAT4(light[i]->getPosition().x, light[i]->getPosition().y, light[i]->getPosition().z, 0.0f);
		lightPtr->specularColour[i] = light[i]->getSpecularColour();
		lightPtr->specularPower[i] = XMFLOAT4(light[i]->getSpecularPower(), 0.0f, 0.0f, 0.0f);
		lightPtr->type[i] = lightType[i];
		lightPtr->attFactors[i] = XMFLOAT4(0.7f, 0.2f, 0.05f, 0.0f);
	}

	deviceContext->Unmap(lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);

	// Setting textures for pixel and domain shaders.
	deviceContext->PSSetShaderResources(0, 1, &textureGrass);
	deviceContext->PSSetShaderResources(1, 1, &textureRock);
	deviceContext->PSSetShaderResources(2, 1, &textureSnow);
	deviceContext->PSSetShaderResources(3, 1, &heightMap); // Heightmap for debugging
	deviceContext->PSSetShaderResources(4, 2, depthMap);
	deviceContext->DSSetShaderResources(0, 1, &heightMap); // Heightmap for domain shader

	// Set texture samplers for both pixel and domain shaders.
	deviceContext->PSSetSamplers(0, 1, &sampleState);
	deviceContext->DSSetSamplers(0, 1, &sampleState);
}

void LightShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, Light* light[lightSizeLightShader], XMFLOAT4 lightType[lightSizeLightShader], XMFLOAT3 camPos, ID3D11ShaderResourceView* depthMap[lightSizeLightShader]) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;

	XMMATRIX tworld, tview, tproj;
	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(world);
	tview = XMMatrixTranspose(view);
	tproj = XMMatrixTranspose(projection);
	XMMATRIX tLightViewMatrix[lightSizeLightShader];
	XMMATRIX tLightProjectionMatrix[lightSizeLightShader];
	for (int i = 0;i < lightSizeLightShader;i++) {
		tLightViewMatrix[i] = XMMatrixTranspose(light[i]->getViewMatrix());
		if (lightType[i].y == 1.f)
			tLightProjectionMatrix[i] = XMMatrixTranspose(light[i]->getOrthoMatrix());
		else
			tLightProjectionMatrix[i] = XMMatrixTranspose(light[i]->getProjectionMatrix());
	}

	// Lock the constant buffer so it can be written to.
	deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	for (int i = 0;i < lightSizeLightShader;i++) {
		dataPtr->lightView[i] = tLightViewMatrix[i];
		dataPtr->lightProjection[i] = tLightProjectionMatrix[i];
	}
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	//Additional
	//do constant buffers here
	CameraBuffer* camPtr;
	deviceContext->Map(camBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	camPtr = (CameraBuffer*)mappedResource.pData;
	camPtr->cameraPosition = XMFLOAT4(camPos.x, camPos.y, camPos.z, 0.f);
	deviceContext->Unmap(camBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &camBuffer);

	LightBuffer* lightPtr;
	deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	lightPtr = (LightBuffer*)mappedResource.pData;

	for (int i = 0;i < lightSizeLightShader;i++) {

		lightPtr->ambient[i] = light[i]->getAmbientColour();
		lightPtr->diffuse[i] = light[i]->getDiffuseColour();
		lightPtr->direction[i] = XMFLOAT4(light[i]->getDirection().x, light[i]->getDirection().y, light[i]->getDirection().z, 0.0f);
		lightPtr->lightPosition[i] = XMFLOAT4(light[i]->getPosition().x, light[i]->getPosition().y, light[i]->getPosition().z, 0.0f);
		lightPtr->specularColour[i] = light[i]->getSpecularColour();
		lightPtr->specularPower[i] = XMFLOAT4(light[i]->getSpecularPower(), 0.0f, 0.0f, 0.0f);
		lightPtr->type[i] = lightType[i];
		lightPtr->attFactors[i] = XMFLOAT4(0.7f, 0.2f, 0.05f, 0.0f);
	}
	deviceContext->Unmap(lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);

	// Set shader texture resource in the pixel and vertex shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetShaderResources(1, 2, depthMap);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
}