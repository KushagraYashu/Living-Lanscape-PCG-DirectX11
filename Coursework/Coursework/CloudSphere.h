#pragma once

#include "SphereMesh.h"

class CloudSphere : public SphereMesh
{
public:
	CloudSphere(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution = 20) : SphereMesh(device, deviceContext, resolution) {};
};