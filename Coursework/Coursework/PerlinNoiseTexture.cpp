#include "PerlinNoiseTexture.h"

// Constructor with initialisation
PerlinNoiseTexture::PerlinNoiseTexture(int terrainS, int volumeSx, int volumeSy, int volumeSz) {
	// Initialising the size for terrain height map and cloud density map
	terrainSize = terrainS;
	volumeSizeX = volumeSx;
	volumeSizeY = volumeSy;
	volumeSizeZ = volumeSz;

	// Noise and density data vector initialisation
	noiseData = std::vector<float>(terrainSize * terrainSize);
	densityData = std::vector<float>(volumeSizeX * volumeSizeY * volumeSizeZ);

	// Initialisation of texture and SRV pointers
	noiseTexture = nullptr;
	noiseTextureSRV = nullptr;
	densityTexture = nullptr;
	densityTextureSRV = nullptr;
}

// Destructor
PerlinNoiseTexture::~PerlinNoiseTexture() {
	noiseTexture->Release();
	noiseTextureSRV->Release();

	densityTexture->Release();
	densityTextureSRV->Release();
}

// Smoothing method (Smoothing by averaging with neighbour values)
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

	CreateTextureHM(device, textureMgr);
}

// Generate Perlin noise texture height map (for terrain)
void PerlinNoiseTexture::GeneratePerlinNoiseTextureHM(ID3D11Device* device, TextureManager* textureMgr, float perlinFreq, float perlinAmp) {
	if (perlinFreq == 0) perlinFreq = 0.001;
	if (perlinAmp == 0) perlinAmp = 0.001;
	float perlinScale = 0.01f;
	SimplexNoise noise = SimplexNoise(perlinFreq);
	for (int y = 0; y < terrainSize; y++) {
		for (int x = 0; x < terrainSize; x++) {
			float height = perlinAmp * noise.fractal(15, x * perlinScale, y * perlinScale);  // .fractal is in [-1, 1]
			noiseData[(y * terrainSize) + x] = height;
		}
	}
	
	CreateTextureHM(device, textureMgr);
}

// Creating a 2D texture using the noise data from generate method
void PerlinNoiseTexture::CreateTextureHM(ID3D11Device* device, TextureManager* textureMgr) {
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

// Generate Perlin noise texture density map (for cloud box)
void PerlinNoiseTexture::GeneratePerlinNoiseTextureDM(ID3D11Device* device, TextureManager* textureMgr, float perlinFreq) {
	float perlinScale = 0.5f;
	SimplexNoise noise = SimplexNoise(perlinFreq);
	for (int z = 0; z < volumeSizeZ; z++) {
		for (int y = 0; y < volumeSizeY; y++) {
			for (int x = 0; x < volumeSizeX; x++) {
				float val = noise.fractal(10, x * perlinScale, y * perlinScale, z * perlinScale);
				//val = (val * 0.5f) + 0.5f;
				//val *= 5;
				densityData[(z * volumeSizeZ * volumeSizeY) + (y * volumeSizeX) + x] = val;
			}
		}
	}

	CreateTextureDM(device, textureMgr);
}

// Creating a 3D texture using the density data from generate method
void PerlinNoiseTexture::CreateTextureDM(ID3D11Device* device, TextureManager* textureMgr) {
	//Creating texture
	D3D11_TEXTURE3D_DESC texDesc{};
	texDesc.Width = volumeSizeX;
	texDesc.Height = volumeSizeY;
	texDesc.Depth = volumeSizeZ;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32_FLOAT;
	texDesc.Usage = D3D11_USAGE_DYNAMIC;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	texDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = densityData.data();
	initData.SysMemPitch = volumeSizeX * sizeof(float);           // bytes per row (X)
	initData.SysMemSlicePitch = volumeSizeX * volumeSizeY * sizeof(float); // bytes per slice (Z)
	densityTexture = nullptr;
	HRESULT hr = device->CreateTexture3D(&texDesc, &initData, &densityTexture);

	//Creating shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	srvDesc.Texture3D.MipLevels = 1;
	densityTextureSRV = nullptr;
	hr = device->CreateShaderResourceView(densityTexture, &srvDesc, &densityTextureSRV);
	textureMgr->addTexture(L"densityVolumeTexture", densityTextureSRV);
}