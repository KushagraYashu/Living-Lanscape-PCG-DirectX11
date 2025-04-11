// **Texture and Sampler Registers**  
// The texture and sampler are bound to registers t0 and s0, respectively.
Texture2D texture0 : register(t0); // The texture to sample from
SamplerState Sampler0 : register(s0); // The sampler state for texture sampling

// **Input Structure**  
// The input structure defines the data passed from the vertex shader to the pixel shader.
struct InputType
{
    float4 position : SV_POSITION; // The position of the pixel (used for rendering but not in this shader).
    float2 tex : TEXCOORD0; // The texture coordinates for mapping the texture.
    float3 normal : NORMAL; // The normal vector (useful for lighting, but not used here).
};

// **Main Pixel Shader Function**  
// This function takes the input from the vertex shader, samples the texture, and applies a brightness filter.
float4 main(InputType input) : SV_TARGET
{
    // Sample the color from the texture using the texture coordinates.
    float4 color = texture0.Sample(Sampler0, input.tex);

    // Convert the color to linear space by applying gamma correction (inverse of 2.2 gamma).
    color.xyz = pow(color.xyz, 2.2f);
    
    // Define a brightness threshold. Colors below this value will be set to black.
    float brightnessThreshold = 0.2;

    // Calculate the intensity (brightness) of the color using relative luminance.
    // This uses a weighted average based on the human eye's sensitivity to different color components.
    float intensity = dot(color.rgb, float3(0.2126, 0.7152, 0.0722)); // relative luminance (linear space)
    
    // If the intensity of the color is above the threshold, keep the original color.
    // Otherwise, set the color to black.
    if (intensity >= brightnessThreshold)
    {
        return color; // Retain the color if it's bright enough
    }
    else
    {
        return float4(0.0, 0.0, 0.0, color.a); // Set to black if the intensity is below the threshold
    }
}