// Constant buffer for storing the world, view, and projection matrices
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix; // World matrix to transform objects from local to world space
    matrix viewMatrix; // View matrix to transform coordinates from world space to camera space
    matrix projectionMatrix; // Projection matrix to transform camera space to clip space
};

// Input structure containing vertex attributes
struct InputType
{
    float4 position : POSITION; // Vertex position
    float2 tex : TEXCOORD0; // Texture coordinates
    float3 normal : NORMAL; // Normal vector for lighting calculations
};

// Output structure that passes the transformed vertex data to the pixel shader
struct OutputType
{
    float4 position : SV_POSITION; // Transformed position after applying world, view, and projection matrices
    float2 tex : TEXCOORD0; // Pass the texture coordinates to the pixel shader
    float3 normal : NORMAL; // Pass the normal vector for lighting calculations
    float4 worldPos : POSITION;
};

// Main vertex shader function
OutputType main(InputType input)
{
    OutputType output;
    
    // Setting world position with world transformation
    output.worldPos = mul(input.position, worldMatrix);
    
    // Apply the world, view, and projection transformations to the vertex position
    output.position = mul(input.position, worldMatrix); // Apply world transformation
    output.position = mul(output.position, viewMatrix); // Apply view transformation
    output.position = mul(output.position, projectionMatrix); // Apply projection transformation

    // Pass the texture coordinates to the pixel shader
    output.tex = input.tex;

    // Pass the normal vector to the pixel shader for lighting calculations
    output.normal = input.normal;

    // Return the transformed output
    return output;
}