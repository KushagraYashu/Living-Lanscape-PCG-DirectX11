// Texture and sampler registers
Texture2D texture0 : register(t0); // The texture used for sampling
SamplerState Sampler0 : register(s0); // Sampler for texture sampling

// Constant buffer to hold screen dimensions (used for texel size calculation)
cbuffer ScreenDimensions : register(b0)
{
    float4 screenDimen; // Screen dimensions (width, height)
};

// Input structure to hold vertex data
struct InputType
{
    float4 position : SV_POSITION; // Position of the vertex in screen space
    float2 tex : TEXCOORD0; // Texture coordinates
    float3 normal : NORMAL; // Normal vector (not used in this shader but typically used for lighting)
};

// Main shader function to process each pixel
// Wikipedia contributors. (n.d.). Kernel (image processing) [Online article]. Wikipedia. Available at: https://en.wikipedia.org/wiki/Kernel_(image_processing)
float4 main(InputType input) : SV_TARGET
{
    float4 colour; // Initialize the final color of the pixel

    colour = float4(0, 0, 0, 0); // Set the initial color to black
    
    // Calculate texel width and height based on screen dimensions
    float texelWidth = 1.f / screenDimen.x;
    float texelHeight = 1.f / screenDimen.y;
    
    // Gaussian kernel for a 5x5 blur
    float kernel[25] =
    {
        0.0039f, 0.0156f, 0.0234f, 0.0156f, 0.0039f,
        0.0156f, 0.0625f, 0.0938f, 0.0625f, 0.0156f,
        0.0234f, 0.0938f, 0.1406f, 0.0938f, 0.0234f,
        0.0156f, 0.0625f, 0.0938f, 0.0625f, 0.0156f,
        0.0039f, 0.0156f, 0.0234f, 0.0156f, 0.0039f
    };
    
    int index = 0;
    // Loop through the kernel and sample texture at neighboring texel positions
    for (int y = +2; y >= -2; y--) // Loop through vertical neighbors
    {
        for (int x = -2; x <= +2; x++) // Loop through horizontal neighbors
        {
            // Calculate offset for each texel
            float2 offset = float2(x * texelWidth, y * texelHeight);
            // Accumulate the weighted color value from the surrounding texels
            colour += texture0.Sample(Sampler0, input.tex + offset) * kernel[index];
            index++; // Move to the next kernel value
        }
    }

    // Increase the brightness of the final color (to brighten spots intentionally)
    colour *= 1.4f;
    colour.a = 1.f; // Set alpha to fully opaque
    
    // Return the final color value
    return colour;
}