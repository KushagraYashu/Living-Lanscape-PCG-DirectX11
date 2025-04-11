// **Sun Pixel Shader**  
// This shader samples the texture, applies a sun color tint, and adjusts the brightness using gamma correction.

// **Texture and Sampler Registers**  
// The texture and sampler are bound to registers t0 and s0, respectively.
Texture2D texture0 : register(t0); // The texture to sample from
SamplerState Sampler0 : register(s0); // The sampler state for texture sampling

// **Constant Buffer for Sun Color**  
// The sun color is passed as a constant buffer and applied to the sampled texture.
cbuffer SunColor : register(b0)
{
    float4 sunColor; // The color of the sun, used to tint the texture
}

// **Input Structure**  
// The input structure defines the data passed from the vertex shader to the pixel shader.
struct InputType
{
    float4 position : SV_POSITION; // The position of the pixel (used for rendering but not in this shader).
    float2 tex : TEXCOORD0; // The texture coordinates for mapping the texture.
    float3 normal : NORMAL; // The normal vector (useful for lighting, but not used here).
};

// **Main Pixel Shader Function**  
// This function takes the input from the vertex shader, samples the texture, and applies color modifications.
float4 main(InputType input) : SV_TARGET
{
    // Sample the texture at the given texture coordinates and multiply by the sun color.
    // Then, apply gamma correction by raising the result to the power of (1 / 2.2).
    // This step converts from linear space to gamma-corrected color space.
    return pow(sunColor * texture0.Sample(Sampler0, input.tex), 1 / 2.2);
}