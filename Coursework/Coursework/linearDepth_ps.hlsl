// Constant buffer for camera position
cbuffer CameraBuffer : register(b0)
{
    float4 camPos;
}

// Input structure containing vertex and depth data for each pixel
struct InputType
{
    float4 position : SV_POSITION; // The transformed vertex position in screen space
    float2 tex : TEXCOORD0; // Texture coordinates
    float4 depthPosition : TEXCOORD1; // The depth of the pixel from the light's view
};

// Main function: Processes the pixel and outputs its depth value as a greyscale color
float4 main(InputType input) : SV_TARGET
{
    float depthValue; // linear depth value
    
    // Screen depth value (check SCREEN_DEPTH on the CPU side)
    float zFar = 200;
    
    // Performing linear depth calculations
    depthValue = length(input.depthPosition.xyz - camPos.xyz) / zFar;
    
    // Return a greyscale color based on the depth value
    return float4(depthValue, depthValue, depthValue, 1.0f);
}