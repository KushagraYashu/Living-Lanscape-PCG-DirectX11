// Define the number of lights
static const int lightSize = 2;

// Constant buffer to store transformation matrices
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix; // World matrix
    matrix viewMatrix; // View matrix
    matrix projectionMatrix; // Projection matrix
    matrix lightViewMatrix[lightSize]; // View matrices for each light
    matrix lightProjectionMatrix[lightSize]; // Projection matrices for each light
};

// Input structure containing vertex data
struct InputType
{
    float4 position : POSITION; // Vertex position
    float2 tex : TEXCOORD0; // Texture coordinates
    float3 normal : NORMAL; // Normal vector for the vertex
};

// Output structure to store transformed vertex data
struct OutputType
{
    float4 position : SV_POSITION; // Transformed vertex position
    float2 tex : TEXCOORD0; // Texture coordinates passed to the pixel shader
    float4 depthPosition : TEXCOORD1; // Depth value for shadow mapping
    float4 vertexManipulationData : TEXCOORD2; // Data for manipulating the vertex (e.g., height map strength)
    float3 viewVector : TEXCOORD3; // View vector (camera to vertex)
    float4 lightViewPos[lightSize] : TEXCOORD4; // Light view positions for each light
    float3 worldPosition : POSITION0; // World position of the vertex
};

// Main function to process the vertex data
OutputType main(InputType input)
{
    OutputType output;

    // Apply world, view, and projection matrices to the vertex position
    output.position = mul(input.position, worldMatrix); // Apply world transformation
    output.position = mul(output.position, viewMatrix); // Apply view transformation
    output.position = mul(output.position, projectionMatrix); // Apply projection transformation

    // Pass the depth position for depth calculations (e.g., for shadow mapping)
    output.depthPosition = output.position;
    
    // Return the transformed vertex data
    return output;
}