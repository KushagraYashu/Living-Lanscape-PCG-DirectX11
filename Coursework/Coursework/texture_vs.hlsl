// **Texture Vertex Shader**  
// Basic shader for rendering textured geometry, applying world, view, and projection transformations to vertices

// Constant buffer that holds world, view, and projection matrices
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix; // World transformation matrix
    matrix viewMatrix; // View transformation matrix
    matrix projectionMatrix; // Projection transformation matrix
};

// **Input and Output Structures**  
// InputType: Structure for input data per vertex
struct InputType
{
    float4 position : POSITION; // Vertex position (4D for homogeneous coordinates)
    float2 tex : TEXCOORD0; // Texture coordinates (used for mapping textures)
    float3 normal : NORMAL; // Vertex normal (used for lighting)
};

// OutputType: Structure for output data per vertex after transformation
struct OutputType
{
    float4 position : SV_POSITION; // Transformed vertex position (will be used for rasterization)
    float2 tex : TEXCOORD0; // Texture coordinates passed to the pixel shader
    float3 normal : NORMAL; // Vertex normal passed to the pixel shader
};

// **Main Function**  
// The main function processes each vertex, applying transformations and passing data to the pixel shader
OutputType main(InputType input)
{
    OutputType output;

    // **Transform the vertex position**  
    // Apply world, view, and projection matrices to transform the vertex into screen space
    output.position = mul(input.position, worldMatrix); // Transform by world matrix
    output.position = mul(output.position, viewMatrix); // Transform by view matrix
    output.position = mul(output.position, projectionMatrix); // Transform by projection matrix

    // **Pass texture coordinates**  
    // Pass the texture coordinates to the pixel shader
    output.tex = input.tex;

    // **Pass normal vector**  
    // Pass the normal vector to the pixel shader for lighting calculations
    output.normal = input.normal;

    return output;
}