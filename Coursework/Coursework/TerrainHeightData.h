#pragma once


class HeightMap{
public:
	struct HeightMapData {
		float x, y, z;
	};

	void GenerateHeightMap(HeightMapData heightMap[]);
};