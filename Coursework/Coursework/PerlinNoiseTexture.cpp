#include "PerlinNoiseTexture.h"

PerlinNoiseTexture::PerlinNoiseTexture(int terrainS) {
	call = 0;
	terrainSize = terrainS;
	noiseData = std::vector<float>(terrainSize * terrainSize);
	noiseTexture = nullptr;
	noiseTextureSRV = nullptr;
}

PerlinNoiseTexture::~PerlinNoiseTexture() {
	noiseTexture->Release();
	noiseTextureSRV->Release();
}

void PerlinNoiseTexture::SmoothHeightMap(ID3D11Device* device, TextureManager* textureMgr) {
	std::vector<float> smoothedHeights(terrainSize * terrainSize, 0.0f);

	for (int j = 0; j < terrainSize; j++)
	{
		for (int i = 0; i < terrainSize; i++)
		{
			float sum = 0.0f;
			int count = 0;

			for (int y = -1; y <= 1; y++)
			{
				for (int x = -1; x <= 1; x++)
				{
					int neighborX = i + x;
					int neighborY = j + y;

					if (neighborX >= 0 && neighborX < terrainSize &&
						neighborY >= 0 && neighborY < terrainSize)
					{
						int neighborIndex = (terrainSize * neighborY) + neighborX;
						sum += noiseData[neighborIndex];
						count++;
					}
				}
			}

			int currentIndex = (terrainSize * j) + i;
			smoothedHeights[currentIndex] = sum / count;
		}
	}

	for (int j = 0; j < terrainSize; j++)
	{
		for (int i = 0; i < terrainSize; i++)
		{
			int index = (terrainSize * j) + i;
			noiseData[index] = smoothedHeights[index];
		}
	}

	CreateTexture(device, textureMgr);
}

void PerlinNoiseTexture::GeneratePerlinNoiseTexture(ID3D11Device* device, TextureManager* textureMgr) {
	float perlinFreq = 0.06f;
	float perlinAmp = 125;
	SimplexNoise noise = SimplexNoise(perlinFreq, perlinAmp);
	for (int y = 0; y < terrainSize; y++) {
		for (int x = 0; x < terrainSize; x++) {
			float height = perlinAmp/10 * noise.fractal(15, x, y);  // Should be in [-1, 1]
			noiseData[(y * terrainSize) + x] = height;
		}
	}
	
	CreateTexture(device, textureMgr);
}

void PerlinNoiseTexture::CreateTexture(ID3D11Device* device, TextureManager* textureMgr) {
	//Creating texture
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = terrainSize;
	desc.Height = terrainSize;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R32_FLOAT;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA texData{};
	texData.pSysMem = noiseData.data();
	texData.SysMemPitch = terrainSize * sizeof(float);
	HRESULT hr = device->CreateTexture2D(&desc, &texData, &noiseTexture);

	//Creating shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(noiseTexture, &SRVDesc, &noiseTextureSRV);
	textureMgr->addTexture(L"perlinNoiseHeightMap", noiseTextureSRV);
}