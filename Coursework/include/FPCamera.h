#pragma once

#include "camera.h"
#include "input.h"
#include <vector>

using namespace DirectX;

class FPCamera : public Camera
{
public:
	std::vector<float> noiseData;
	int size;
	bool flightMode = false;
	/*void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}

	void operator delete(void* p)
	{
		_mm_free(p);
	}*/

	FPCamera(Input* in, int width, int height, HWND hnd);	///< Initialised default camera object
	//~FPCamera();

	void move(float dt);	///< Move camera, handles basic camera movement

private:
	Input* input;
	int winWidth, winHeight;///< stores window width and height
	int deltax, deltay;		///< for mouse movement
	POINT cursor;			///< Used for converting mouse coordinates for client to screen space
	HWND wnd;				///< handle to the window
};