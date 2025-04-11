// Define the number of light sources
static const int lightSize = 2;

// Constant buffer for transformation matrices
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix; // World transformation matrix
    matrix viewMatrix; // Camera view matrix
    matrix projectionMatrix; // Camera projection matrix
    matrix lightViewMatrix[lightSize]; // View matrices for each light source
    matrix lightProjectionMatrix[lightSize]; // Projection matrices for each light source
};

// Constant buffer for camera information
cbuffer CameraBuffer : register(b1)
{
    float4 cameraPosition; // Camera's position in world space
};

// Input structure for the vertex shader
struct InputType
{
    float4 position : POSITION; // Vertex position
    float2 tex : TEXCOORD0; // Texture coordinates
    float3 normal : NORMAL; // Normal vector
};

// Output structure for the vertex shader
struct OutputType
{
    float4 position : SV_POSITION; // Final position for the pixel shader
    float2 tex : TEXCOORD0; // Texture coordinates
    float4 depthPosition : TEXCOORD1; // Depth position for shadow mapping
    float4 vertexManipulationData : TEXCOORD2; // Data for vertex manipulation (e.g., height map strength)
    float3 viewVector : TEXCOORD3; // View vector (camera to vertex)
    float4 lightViewPos[lightSize] : TEXCOORD4; // Light view positions for shadow mapping
    float3 worldPosition : POSITION0; // Vertex position in world space
    float3 normal : NORMAL; // Normal vector in world space
};

// Main function (vertex shader)
OutputType main(InputType input)
{
    OutputType output;

    // For each light, calculate the light's view position from the vertex position
    for (int i = 0; i < lightSize; i++)
    {
        // Transform vertex position into the light's view space
        output.lightViewPos[i] = mul(input.position, worldMatrix); // Apply world matrix
        output.lightViewPos[i] = mul(output.lightViewPos[i], lightViewMatrix[i]); // Apply light view matrix
        output.lightViewPos[i] = mul(output.lightViewPos[i], lightProjectionMatrix[i]); // Apply light projection matrix
    }

    // Transform vertex position to camera space (view space) and then to clip space
    output.position = mul(input.position, worldMatrix); // World transformation
    output.position = mul(output.position, viewMatrix); // View transformation (camera space)
    output.position = mul(output.position, projectionMatrix); // Projection transformation (clip space)

    // Calculate world position of the vertex
    output.worldPosition = mul(input.position, worldMatrix).xyz; // World position from the world matrix

    // Calculate the view vector (direction from the vertex to the camera)
    output.viewVector = cameraPosition.xyz - output.worldPosition.xyz; // Vector from camera to vertex
    output.viewVector = normalize(output.viewVector); // Normalize the view vector

    // Pass through the texture coordinates
    output.tex = input.tex;

    // Transform the normal vector from object space to world space and normalize it
    output.normal = mul(input.normal, (float3x3) worldMatrix); // Apply world matrix to normal (excluding translation)
    output.normal = normalize(output.normal); // Normalize the normal vector

    return output;
}