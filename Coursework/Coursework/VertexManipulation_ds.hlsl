// **Tessellation Domain Shader**  
// After tessellation, this domain shader processes all the vertices and calculates the final positions.

#include "ClassicNoise2D.hlsl" // For perlin noise

Texture2D heightMap : register(t0); // Texture for height map (used to manipulate the terrain's height)
SamplerState sampler0 : register(s0); // Sampler state to sample the height map texture

static const int lightSize = 2; // Number of lights in the scene

// **Constant Buffers**  
// MatrixBuffer: Contains transformation matrices for the world, view, projection, and light matrices
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix; // World transformation matrix
    matrix viewMatrix; // View transformation matrix
    matrix projectionMatrix; // Projection transformation matrix
    matrix lightViewMatrix[lightSize]; // View matrices for each light
    matrix lightProjectionMatrix[lightSize]; // Projection matrices for each light
};

// Height data buffer
//cbuffer HeightDataBuffer : register(b1)
//{
    
//}

// CameraBuffer: Contains the camera position in world space
cbuffer CameraBuffer : register(b2)
{
    float4 cameraPosition; // Camera position
    float4 params;
};

// **Input and Output Structures**  
// ConstantOutputType: Structure to hold tessellation factors for edges and the inside of the patch
struct ConstantOutputType
{
    float edges[3] : SV_TessFactor; // Tessellation factors for the three edges of the triangle
    float inside : SV_InsideTessFactor; // Tessellation factor for the inside of the triangle
};

// InputType: Structure for input data per vertex, such as position and texture coordinates
struct InputType
{
    float4 position : POSITION; // Vertex position
    float2 tex : TEXCOORD0; // Texture coordinates
};

// OutputType: Structure for output data per vertex, including transformed position, texture coordinates, depth, and other information
struct OutputType
{
    float4 position : SV_POSITION; // Transformed vertex position
    float2 tex : TEXCOORD0; // Texture coordinates
    float4 depthPosition : TEXCOORD1; // Depth position for shadow mapping
    float4 vertexManipulationData : TEXCOORD2; // Height map strength
    float3 viewVector : TEXCOORD3; // Vector from the vertex to the camera
    float4 lightViewPos[lightSize] : TEXCOORD4; // Vertex position from the point of view of each light
    float3 worldPosition : POSITION0; // Vertex position in world space
};

// **Height Calculation**  
// Function to calculate the height at a given texture coordinate by sampling the height map
float GetHeight(float2 UV)
{
    float offset = heightMap.SampleLevel(sampler0, UV, 0).r; // Sample height map to get height offset
    return offset;// * 5.f; // Multiply by height map strength
}

// **Main Function**  
// Domain shader entry point. It calculates the final position and attributes for the tessellated vertices.
[domain("tri")] // Domain shader for triangles
OutputType main(ConstantOutputType input, float3 uvwCoord : SV_DomainLocation, const OutputPatch<InputType, 3> patch)
{
    float3 vertexPosition; // Final vertex position
    OutputType output; // Final output structure
    
    // **Triangle Calculation**  
    // Calculate the vertex position by performing barycentric interpolation using uvwCoords and patch vertices
    vertexPosition = uvwCoord.x * patch[0].position.xyz + uvwCoord.y * patch[1].position.xyz + uvwCoord.z * patch[2].position.xyz;
    
    // Interpolate texture coordinates using uvwCoords and patch vertices' texture coordinates
    float2 UV = uvwCoord.x * patch[0].tex + uvwCoord.y * patch[1].tex + uvwCoord.z * patch[2].tex;
    
    // Modify the Y position of the vertex based on the height map#
    //vertexPosition.y = 0;
    vertexPosition.y += GetHeight(UV); // Add height offset to the Y position
    //vertexPosition.y += PeriodicNoise(UV * params.x, params.y) * params.z;
    //vertexPosition.y += ClassicNoise(UV * params.x) * params.z;
    
    // Set the output position in world space
    output.position = float4(vertexPosition, 1.f);

    // **Light View Calculation**  
    // Calculate the light's view position for each light by transforming the vertex position into the light's view space
    for (int i = 0; i < lightSize; i++)
    {
        output.lightViewPos[i] = mul(output.position, worldMatrix); // Transform position to world space
        output.lightViewPos[i] = mul(output.lightViewPos[i], lightViewMatrix[i]); // Apply light view matrix
        output.lightViewPos[i] = mul(output.lightViewPos[i], lightProjectionMatrix[i]); // Apply light projection matrix
    }
    
    // Set the world position of the vertex
    output.worldPosition = mul(float4(vertexPosition, 1.f), worldMatrix).xyz;
    
    // Calculate the view vector (direction from the vertex to the camera)
    output.viewVector = cameraPosition.xyz - mul(float4(vertexPosition, 1.f), worldMatrix).xyz;
    output.viewVector = normalize(output.viewVector); // Normalize the view vector
    
    // **Final Position Calculation**  
    // Transform the vertex position by the world, view, and projection matrices to get the final screen space position
    output.position = mul(float4(vertexPosition, 1.0f), worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // Set the depth position (for depth or shadow calculations)
    output.depthPosition = output.position;
    
    // Set the vertex manipulation data (for passing through the height map strength)
    output.vertexManipulationData = 30.f; // Height map strength value

    // Send the input color into the pixel shader.
    output.tex = UV; // Set texture coordinates

    return output;
}
