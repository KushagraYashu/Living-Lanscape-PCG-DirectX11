// Texture and sampler registers
Texture2D texture0 : register(t0); // The texture to be sampled
SamplerState Sampler0 : register(s0); // Sampler state for texture sampling

// Constant buffer holding color grading parameters
cbuffer ColorGradingData : register(b0)
{
    float4 tintColor; // Tint color (affects the hue of the image)
    float4 filters; // Holds the values for brightness, contrast, and saturation adjustments
}

// Input structure containing vertex attributes
struct InputType
{
    float4 position : SV_POSITION; // Vertex position in screen space
    float2 tex : TEXCOORD0; // Texture coordinates
    float3 normal : NORMAL; // Normal vector
};

// Main function: Processes the pixel and applies color grading effects
// Akenine-Möller, T., Haines, E., & Hoffman, N. (2018). Real-Time Rendering (4th ed.) [Book]. Available at: https://www.realtimerendering.com/
float4 main(InputType input) : SV_TARGET
{
    // Sample the color from the texture using the provided texture coordinates
    float4 color = texture0.Sample(Sampler0, input.tex);
    
    // Convert the color to linear space by applying gamma correction
    color.rgb = pow(color.rgb, 2.2f);
    
    // Apply brightness adjustment
    float brightness = 1.5;
    brightness = filters.r; // Use the brightness value from the filters buffer
    color.rgb = color.rgb * brightness;
    
    // Apply contrast adjustment
    float contrast = 1.02f;
    contrast = filters.g; // Use the contrast value from the filters buffer
    color.rgb = ((color.rgb - 0.5f) * contrast) + 0.5f; // Adjust the color based on contrast
    
    // Apply saturation adjustment
    float luminance = dot(color.rgb, float3(0.2126, 0.7152, 0.0722)); // Calculate the luminance (grayscale value)
    float3 grayscale = float3(luminance, luminance, luminance); // Convert to grayscale
    float saturationFactor = 1.35;
    saturationFactor = filters.b; // Use the saturation factor from the filters buffer
    color.rgb = lerp(grayscale, color.rgb, saturationFactor); // Mix between grayscale and original color based on saturation factor
    
    // Ensure that the color values do not go negative
    color.r = max(color.r, 0);
    color.g = max(color.g, 0);
    color.b = max(color.b, 0);
    
    // Apply tinting to the color (affects the hue based on the tint color)
    color.rgb = lerp(color.rgb, tintColor.rgb, tintColor.a / 8); // Tinting is controlled by the alpha component of tintColor
    
    // Convert the color back to gamma space after adjustments 
    color.xyz = pow(color.xyz, 1.0f / 2.2f);
    
    // Return the final color after all adjustments
    return color;
}