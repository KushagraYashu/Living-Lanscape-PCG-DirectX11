// Define the number of lights
static const int lightSize = 2;

// Input structure containing vertex and depth data for each pixel
struct InputType
{
    float4 position : SV_POSITION; // The transformed vertex position in screen space
    float2 tex : TEXCOORD0; // Texture coordinates
    float4 depthPosition : TEXCOORD1; // The depth of the pixel from the light's view
    float4 vertexManipulationData : TEXCOORD2; // Extra data, e.g., height map strength (manipulation parameter)
    float3 viewVector : TEXCOORD3; // Vector from the camera to the pixel
    float4 lightViewPos[lightSize] : TEXCOORD4; // Positions of the vertex from the viewpoint of each light
    float3 worldPosition : POSITION0; // The vertex position in world space
};

// Main function: Processes the pixel and outputs its depth value as a greyscale color
float4 main(InputType input) : SV_TARGET
{
    float depthValue;
    
    // Compute the depth value by dividing the Z-coordinate by the W-coordinate for homogeneous depth
    depthValue = input.depthPosition.z / input.depthPosition.w;
    
    // Return a greyscale color based on the depth value
    return float4(depthValue, depthValue, depthValue, 1.0f);
}