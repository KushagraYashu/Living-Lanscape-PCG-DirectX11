// Texture and sampler registers
Texture2D sourceTexture : register(t0); // The source texture, already gamma corrected (from light_ps)
Texture2D bloomTexture : register(t1); // The bloom texture that will be blended with the source texture
SamplerState Sampler0 : register(s0); // Sampler state for the textures

// Input structure for the vertex shader outputs
struct InputType
{
    float4 position : SV_POSITION; // Position of the vertex in screen space
    float2 tex : TEXCOORD0; // Texture coordinates
    float3 normal : NORMAL; // Normal vector (not used in this shader)
};

// Main function for the pixel shader
float4 main(InputType input) : SV_TARGET
{
    // Initialize the color variable
    float4 colour;
    
    // Sample the source color from the source texture
    float4 sourceColour = sourceTexture.Sample(Sampler0, input.tex); // Source texture is already gamma corrected
    
    // Sample the bloom color from the bloom texture
    float4 bloomColour = bloomTexture.Sample(Sampler0, input.tex);

    // Convert the source color to linear space from gamma space
    sourceColour.xyz = pow(sourceColour.xyz, 2.2f); // Gamma correction to linear space
    
    // Additive blending: combine the source color with the bloom color with a certain intensity
    float bloomIntensity = 0.5f; // Control the intensity of the bloom effect
    colour.xyz = sourceColour.xyz + (bloomColour.xyz * bloomIntensity); // Add the bloom color to the source color
    
    // Convert the final color back to gamma space
    colour.xyz = pow(colour.xyz, 1.0f / 2.2f); // Reapply gamma correction
    
    // Setting alpha
    colour.w = 1.f;
    
    // Return the final color, which is the result of the bloom effect applied to the source texture
    return colour;
}