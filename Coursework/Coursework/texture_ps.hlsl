// **Texture Pixel/Fragment Shader**  
// Basic fragment shader for rendering textured geometry.  
// This shader samples the texture at the given texture coordinates and applies it to the fragment.

// **Texture and Sampler Registers**  
// Registers are used to reference texture and sampler objects for the GPU.
Texture2D texture0 : register(t0); // The texture to be applied to the geometry
SamplerState Sampler0 : register(s0); // The sampler state for texture sampling

// **Input Structure**  
// The input structure defines the data passed from the vertex shader to the pixel shader.
struct InputType
{
    float4 position : SV_POSITION; // Transformed vertex position after applying world, view, and projection matrices.
    float2 tex : TEXCOORD0; // Texture coordinates for mapping the texture to the geometry.
    float3 normal : NORMAL; // Vertex normal for lighting (passed to the shader but unused here).
};

// **Main Function**  
// This is the main entry point for the pixel shader.
// It samples the texture and returns the color to be applied to the fragment.
float4 main(InputType input) : SV_TARGET
{
    // **Sample the Pixel Color**  
    // The texture is sampled at the given texture coordinates (input.tex).
    // The Sampler0 is used to determine how the texture is filtered and wrapped.
    return texture0.Sample(Sampler0, input.tex); // Sample and return the color from the texture
}