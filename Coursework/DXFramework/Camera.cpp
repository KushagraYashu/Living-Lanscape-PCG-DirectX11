// Camera class
// Represents a single 3D camera with basic movement.
#include "camera.h"

// Configure defaul camera (including positions, rotation and ortho matrix)
Camera::Camera()
{
	position = XMFLOAT3(25.f, 14.f, 25.f);
	rotation = XMFLOAT3(0.f, 0.f, 0.f);

	lookSpeed = 4.0f;

	// Generate ortho matrix
	XMVECTOR up, position, lookAt;
	up = XMVectorSet(0.0f, 1.0, 0.0, 1.0f);
	position = XMVectorSet(0.0f, 0.0, -10.0, 1.0f);
	lookAt = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0);
	orthoMatrix = XMMatrixLookAtLH(position, lookAt, up);
}

Camera::~Camera()
{
}

// Store frame/delta time.
void Camera::setFrameTime(float t)
{
	frameTime = t;
}

void Camera::setPosition(float lx, float ly, float lz)
{
	position.x = lx;
	position.y = ly;
	position.z = lz;
}

void Camera::setRotation(float lx, float ly, float lz)
{
	rotation.x = lx;
	rotation.y = ly;
	rotation.z = lz;
}

XMFLOAT3 Camera::getPosition()
{
	return position;
}

XMFLOAT3 Camera::getRotation()
{
	return rotation;
}

// Re-calucation view Matrix.
void Camera::update()
{
	XMVECTOR up, positionv, lookAt;
	float yaw, pitch, roll;
	XMMATRIX rotationMatrix;
	
	// Setup the vectors
	up = XMVectorSet(0.0f, 1.0, 0.0, 1.0f);
	positionv = XMLoadFloat3(&position);
	lookAt = XMVectorSet(0.0, 0.0, 1.0f, 1.0f);
	
	// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
	pitch = rotation.x * 0.0174532f;
	yaw = rotation.y * 0.0174532f;
	roll = rotation.z * 0.0174532f;

	// Create the rotation matrix from the yaw, pitch, and roll values.
	rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	// Transform the lookAt and up vector by the rotation matrix so the view is correctly rotated at the origin.
	lookAt = XMVector3TransformCoord(lookAt, rotationMatrix);
	up = XMVector3TransformCoord(up, rotationMatrix);
	
	// Translate the rotated camera position to the location of the viewer.
	lookAt = positionv + lookAt;

	// Finally create the view matrix from the three updated vectors.
	viewMatrix = XMMatrixLookAtLH(positionv, lookAt, up);
}


XMMATRIX Camera::getViewMatrix()
{
	return viewMatrix;
}

XMMATRIX Camera::getOrthoViewMatrix()
{
	return orthoMatrix;
}

XMMATRIX Camera::getProjectionMatrix(float fovRad, float screenNear, float screenFar, float aspectRatio) {
	XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(
		fovRad, // FOV
		aspectRatio,
		screenNear,
		screenFar
	);

	return projectionMatrix;
}

float Camera::lerp(const float& a, const float& b, float t){
	// Clamp t to the range [0, 1] to avoid going beyond the two values.
	if (t <= 0) {
		return a;  // If t is 0, return the starting value a.
	}
	if (t >= 1) {
		return b;  // If t is 1, return the ending value b.
	}
	return a + t * (b - a);  // Linearly interpolate between a and b.
}

void Camera::moveForward(std::vector<float> heightData, int size, bool flightMode)
{
	float radians = rotation.y * 0.0174532f;

	// Predict next position
	speed = frameTime * 10.f;
	XMFLOAT3 predPos = position;
	predPos.x += sinf(radians) * speed;
	predPos.z += cosf(radians) * speed;

	if (!flightMode) {
		// Terrain boundaries
		bool insideBounds =
			predPos.x >= 0 && predPos.x < (float)(size - 1) &&
			predPos.z >= 0 && predPos.z < (float)(size - 1);

		if (insideBounds)
		{
			// Safe to move — update position
			int gridX = (int)predPos.x;
			int gridZ = (int)predPos.z;

			// Sample terrain height
			float terrainHeight = heightData[(gridZ * size) + gridX];

			// Move XZ
			position.x = predPos.x;
			position.z = predPos.z;

			// Smooth Y interpolation
			float targetY = terrainHeight + 3.0f;
			float smoothing = 0.05f;
			position.y = lerp(position.y, targetY, smoothing);
		}
	}
	else {
		position.x = predPos.x;
		position.z = predPos.z;
	}
}

void Camera::moveBackward(std::vector<float> heightData, int size, bool flightMode)
{
	float radians = rotation.y * 0.0174532f;

	// Predict next position
	speed = frameTime * 10.f;
	XMFLOAT3 predPos = position;
	predPos.x -= sinf(radians) * speed;
	predPos.z -= cosf(radians) * speed;

	if (!flightMode) {
		// Terrain boundaries
		bool insideBounds =
			predPos.x >= 0 && predPos.x < (float)(size - 1) &&
			predPos.z >= 0 && predPos.z < (float)(size - 1);

		if (insideBounds)
		{
			// Safe to move — update position
			int gridX = (int)predPos.x;
			int gridZ = (int)predPos.z;

			// Sample terrain height
			float terrainHeight = heightData[(gridZ * size) + gridX];

			// Move XZ
			position.x = predPos.x;
			position.z = predPos.z;

			// Smooth Y interpolation
			float targetY = terrainHeight + 3.0f;
			float smoothing = 0.05f;
			position.y = lerp(position.y, targetY, smoothing);
		}
	}
	else {
		position.x = predPos.x;
		position.z = predPos.z;
	}
}

void Camera::moveUpward()
{
	// Update the upward movement based on the frame time
	speed = frameTime * 10.f;// *0.5f;
	
	// Update the height position.
	position.y += speed;
}


void Camera::moveDownward()
{
	// Update the downward movement based on the frame time
	speed = frameTime * 10.f;// *0.5f;

	// Update the height position.
	position.y -= speed;
}


void Camera::turnLeft()
{
	// Update the left turn movement based on the frame time 
	speed = frameTime * 25.0f;
	
	// Update the rotation.
	rotation.y -= speed;

	// Keep the rotation in the 0 to 360 range.
	if (rotation.y < 0.0f)
	{
		rotation.y += 360.0f;
	}
}


void Camera::turnRight()
{
	// Update the right turn movement based on the frame time
	speed = frameTime * 25.0f;
	
	// Update the rotation.
	rotation.y += speed;

	// Keep the rotation in the 0 to 360 range.
	if (rotation.y > 360.0f)
	{
		rotation.y -= 360.0f;
	}

}


void Camera::turnUp()
{
	// Update the upward rotation movement based on the frame time
	speed = frameTime * 25.0f;
	
	// Update the rotation.
	rotation.x -= speed;

	// Keep the rotation maximum 90 degrees.
	if (rotation.x > 90.0f)
	{
		rotation.x = 90.0f;
	}
}


void Camera::turnDown()
{
	// Update the downward rotation movement based on the frame time
	speed = frameTime * 25.0f;

	// Update the rotation.
	rotation.x += speed;

	// Keep the rotation maximum 90 degrees.
	if (rotation.x < -90.0f)
	{
		rotation.x = -90.0f;
	}
}


void Camera::turn(int x, int y)
{
	// Update the rotation.
	rotation.y += (float)x/lookSpeed;// m_speed * x;

	rotation.x += (float)y/lookSpeed;// m_speed * y;
}

void Camera::strafeRight(std::vector<float> heightData, int size, bool flightMode)
{
	float radians = rotation.y * 0.0174532f;

	// Predict next position
	speed = frameTime * 10.f;
	XMFLOAT3 predPos = position;
	predPos.x += cosf(radians) * speed;
	predPos.z -= sinf(radians) * speed;

	if (!flightMode) {
		// Terrain boundaries
		bool insideBounds =
			predPos.x >= 0 && predPos.x < (float)(size - 1) &&
			predPos.z >= 0 && predPos.z < (float)(size - 1);

		if (insideBounds)
		{
			// Safe to move — update position
			int gridX = (int)predPos.x;
			int gridZ = (int)predPos.z;

			// Sample terrain height
			float terrainHeight = heightData[(gridZ * size) + gridX];

			// Move XZ
			position.x = predPos.x;
			position.z = predPos.z;

			// Smooth Y interpolation
			float targetY = terrainHeight + 3.0f;
			float smoothing = 0.05f;
			position.y = lerp(position.y, targetY, smoothing);
		}
	}
	else {
		position.x = predPos.x;
		position.z = predPos.z;
	}
}

void Camera::strafeLeft(std::vector<float> heightData, int size, bool flightMode)
{
	float radians = rotation.y * 0.0174532f;

	// Predict next position
	speed = frameTime * 10.f;
	XMFLOAT3 predPos = position;
	XMFLOAT3 right = { cosf(radians), 0, sinf(radians) };
	predPos.x -= cosf(radians) * speed;
	predPos.z += sinf(radians) * speed;

	if (!flightMode) {
		// Terrain boundaries
		bool insideBounds =
			predPos.x >= 0 && predPos.x < (float)(size - 1) &&
			predPos.z >= 0 && predPos.z < (float)(size - 1);

		if (insideBounds)
		{
			// Safe to move — update position
			int gridX = (int)predPos.x;
			int gridZ = (int)predPos.z;

			// Sample terrain height
			float terrainHeight = heightData[(gridZ * size) + gridX];

			// Move XZ
			position.x = predPos.x;
			position.z = predPos.z;

			// Smooth Y interpolation
			float targetY = terrainHeight + 3.0f;
			float smoothing = 0.05f;
			position.y = lerp(position.y, targetY, smoothing);
		}
	}
	else {
		position.x = predPos.x;
		position.z = predPos.z;
	}
}