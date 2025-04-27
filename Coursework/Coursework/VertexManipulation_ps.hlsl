// Define the number of lights in the scene (currently 2 lights)
static const int lightSize = 2;

Texture2D heightMap : register(t0); // The main texture
SamplerState sampler0 : register(s0);

// Input structure that receives data for each vertex
struct InputType
{
    // The transformed position of the vertex in clip space
    float4 position : SV_POSITION; // SV_POSITION: The vertex position in the shader's coordinate system
    
    // Texture coordinates used for mapping textures onto the geometry
    float2 tex : TEXCOORD0; // TEXCOORD0: Texture coordinates (2D)
    
    // The depth position of the vertex, used for shadow mapping or depth calculations
    float4 depthPosition : TEXCOORD1; // TEXCOORD1: Depth position for shadow mapping or other calculations
    
    // Vertex manipulation data, including the height map strength (in .x) and other data for manipulation
    float4 vertexManipulationData : TEXCOORD2; //.x is height map strength (used in terrain manipulation)
    
    // The view vector from the camera to the vertex
    float3 viewVector : TEXCOORD3; // TEXCOORD3: View vector for lighting or effects calculations
    
    // The light's view of the vertex position (for multiple lights)
    float4 lightViewPos[lightSize] : TEXCOORD4; // TEXCOORD4: The position of the vertex from the light's point of view (multiple lights supported)
    
    // The world position of the vertex
    float3 worldPosition : POSITION0; // POSITION0: The vertex position in world space
};

// Main function that processes the input vertex data and outputs a color
float4 main(InputType input) : SV_TARGET
{
    // Return a white color (opaque) as the output for this shader stage
    return float4(1.f, 1.f, 1.f, 1.f); // The output color of the pixel, white with full opacity
}