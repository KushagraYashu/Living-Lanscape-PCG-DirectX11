// **Constant Buffer**
// Holds colors and light position for rendering the sky dome.
cbuffer ColorBuffer : register(b0)
{
    float4 apexColor; // Color at the top of the sky dome (sky color at the zenith).
    float4 centerColor; // Color at the center of the sky dome (horizon color).
    float4 lightPosition; // Position of the light source (typically the sun or directional light).
};

// **Input Structure**  
// Represents the input data passed to the pixel shader from the vertex shader.
struct InputType
{
    float4 position : SV_POSITION; // Final transformed vertex position.
    float4 domePosition : TEXCOORD0; // Original (non-transformed) position of the vertex, used for blending.
};

// **Pixel Shader Main Function**
// This function calculates the final color of each pixel based on its height and the dome's color blending.
// Rastertek (2013) DirectX 11 Terrain Tutorial (Lesson 10) (code version 1) [online tutorial]. Adapted from: https://rastertek.com/tertut10.html.
float4 main(InputType input) : SV_TARGET
{
    float height;
    float4 outputColor;

    // **Height Calculation**  
    // Use the Y component of the dome position to calculate the height (elevation) of the pixel.
    height = input.domePosition.y;
    
    // **Blend Factor Calculation**  
    // Calculate the blending factor based on the height of the pixel, with a maximum height of 0.4 (used for blending).
    float blendFactor = clamp(height / 0.4f, 0.0f, 1.0f); // Blend factor ranges from 0 (center) to 1 (top).

    // **Color Blending**  
    // Lerp (linearly interpolate) between the center color and apex color based on the blend factor.
    outputColor = lerp(centerColor, apexColor, blendFactor);

    // **Gamma Correction**  
    // Apply gamma correction (2.2) to the color to adjust for display gamma.
    outputColor.xyz = pow(outputColor.xyz, 1.0f / 2.2f);

    return outputColor; // Return the final color for the pixel.
}