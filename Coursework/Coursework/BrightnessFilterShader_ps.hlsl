// Texture and sampler registers
Texture2D texture0 : register(t0); // The texture to sample
SamplerState Sampler0 : register(s0); // The sampler state for the texture

// Input structure containing vertex attributes
struct InputType
{
    float4 position : SV_POSITION; // Vertex position in screen space
    float2 tex : TEXCOORD0; // Texture coordinates
    float3 normal : NORMAL; // Normal vector (not used in this shader, but passed through)
};

// Main function for pixel/fragment shader
float4 main(InputType input) : SV_TARGET
{
    // Sample the color from the texture using the texture coordinates
    float4 color = texture0.Sample(Sampler0, input.tex);

    // Convert the color to linear space from gamma space for proper lighting calculations
    color.xyz = pow(color.xyz, 2.2f);

    // Set a brightness threshold for filtering pixels based on their intensity
    float brightnessThreshold = 0.6;

    // Calculate the intensity (brightness) of the color using relative luminance formula
    // This is a weighted sum of the RGB channels based on human visual perception
    float intensity = dot(color.rgb, float3(0.2126, 0.7152, 0.0722)); // Relative luminance in linear space

    // If the intensity is above the threshold, keep the color
    // Otherwise, set the color to black (fade out dark pixels)
    if (intensity >= brightnessThreshold)
    {
        return color; // Return the color if it's bright enough
    }
    else
    {
        return float4(0.0, 0.0, 0.0, color.a); // Set dark pixels to black
    }
}