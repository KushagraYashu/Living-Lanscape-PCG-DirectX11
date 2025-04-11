#pragma once

#include "iostream"
#include <D3D.h>
#include <d3d11.h>
#include "DTK\include\DDSTextureLoader.h"
#include "DTK\include\WICTextureLoader.h"
#include <vector>
#include "SimplexNoise.h"
#include "TextureManager.h"
//#include 

class PerlinNoiseTexture {
private:
	int call;
	int terrainSize;
	std::vector<float> noiseData;

	ID3D11Texture2D* noiseTexture;
	ID3D11ShaderResourceView* noiseTextureSRV;

	void CreateTexture(ID3D11Device* device, TextureManager* textureMgr);

public:
	void GeneratePerlinNoiseTexture(ID3D11Device* device, TextureManager* textureMgr);
	ID3D11ShaderResourceView* getPerlinNoiseTextureSRV() { return noiseTextureSRV; }

	void SmoothHeightMap(ID3D11Device* device, TextureManager* textureMgr);

	void SaveTextureAsPNG(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Texture2D* texture, const std::wstring& filename);

	PerlinNoiseTexture(int terrainSize);
	~PerlinNoiseTexture();
};