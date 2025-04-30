#pragma once

#include "iostream"
#include <D3D.h>
#include <d3d11.h>
#include "DTK\include\DDSTextureLoader.h"
#include "DTK\include\WICTextureLoader.h"
#include <vector>
#include "SimplexNoise.h"
#include "TextureManager.h"

// Class for perlin noise texture
// This uses an implementation of Perlin's Simplex Noise.
// Rombauts, S., (2014). Perlin’s Simplex Noise C++ Implementation [Library/Framework]  Available at: https://github.com/SRombauts/SimplexNoise. (Accessed: 31 January 2025).
class PerlinNoiseTexture {
private:
	// Terrain size and volume size variables
	int terrainSize, volumeSizeX, volumeSizeY, volumeSizeZ;

	// vectors for noise and density data
	std::vector<float> noiseData;
	std::vector<float> densityData;

	// texture and SRV for noise texture
	ID3D11Texture2D* noiseTexture;
	ID3D11ShaderResourceView* noiseTextureSRV;

	// texture and SRV for density texture
	ID3D11Texture3D* densityTexture;
	ID3D11ShaderResourceView* densityTextureSRV;

	// method to create height map texture
	void CreateTextureHM(ID3D11Device* device, TextureManager* textureMgr);

	// method to create density texture
	void CreateTextureDM(ID3D11Device* device, TextureManager* textureMgr);

public:
	// method to generate the height map
	void GeneratePerlinNoiseTextureHM(ID3D11Device* device, TextureManager* textureMgr, float perlinFreq = 0.06, float perlinAmp = 12.5);

	// method to generate density map
	void GeneratePerlinNoiseTextureDM(ID3D11Device* device, TextureManager* textureMgr, float perlinFreq = 0.1);

	// method to fetch the noise texture SRV
	ID3D11ShaderResourceView* getPerlinNoiseTextureSRV() { return noiseTextureSRV; }

	// method to smooth out noise texture values
	void SmoothHeightMap(ID3D11Device* device, TextureManager* textureMgr);

	// method to get the height values at a grid location
	float GetHeightAt(int x, int y) { 
		if (x && y >= 0) {
			if(x && y < terrainSize)
				return noiseData[(y * terrainSize) + x];
			else
				return noiseData[((terrainSize - 1) * terrainSize) + (terrainSize - 1)];
		}
		else 
			return noiseData[(0) + 0];
	}

	// method to get the noise data vector
	std::vector<float> GetHeightDataRaw() { return noiseData; }

	// method to get the terrain size
	int GetTerrainSize() { return terrainSize; }

	// Constructor with size initialisation
	PerlinNoiseTexture(int terrainSize, int volumeSx, int volumeSy, int volumeSz);
	~PerlinNoiseTexture();
};