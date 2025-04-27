// Texture and sampler registers
Texture2D sourceTexture : register(t0); // The source texture, already gamma corrected (from light_ps)
Texture2D cloudTexture : register(t1); // The cloud texture that will be blended with the source texture
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
    float4 finalColor;
    
    // fetching the source color at the current UVs from the source texture
    float4 sourceColor = sourceTexture.Sample(Sampler0, input.tex);
    sourceColor.xyz = pow(sourceColor.xyz, 2.2f); // Gamma correction to linear space
    
    // fetching the cloud color at the current UVs from the cloud texture
    float4 cloudColor = cloudTexture.Sample(Sampler0, input.tex);
    cloudColor.xyz = pow(cloudColor.xyz, 2.2f); // Gamma correction to linear space
    
    // the blend factor from the cloud texture
    float transparency = cloudColor.a;
    
    // blending
    finalColor.rgb = sourceColor.rgb * transparency + cloudColor.rgb;
    
    // performing gamma correction
    finalColor.rgb = pow(finalColor.rgb, 1 / 2.2f);
    
    // return the final blended gamma corrected value
    return float4(finalColor.rgb, 1);
}