// Input structure to hold the vertex data for each input element
struct InputType
{
    // Vertex position (3D coordinate)
    float3 position : POSITION; // POSITION: The position of the vertex in space
    
    // Texture coordinates (2D)
    float2 tex : TEXCOORD0; // TEXCOORD0: The texture coordinates used for mapping textures
    
    // Normal vector (for lighting calculations)
    float3 normal : NORMAL; // NORMAL: The normal vector of the vertex
};

// Output structure to hold the data that will be passed to the next stage (hull shader in this case)
struct OutputType
{
    // Output vertex position
    float3 position : POSITION; // POSITION: The position of the vertex to be passed to the next shader stage
    
    // Output texture coordinates
    float2 tex : TEXCOORD0; // TEXCOORD0: The texture coordinates to be passed to the next shader stage
};

// The main function that processes the input vertex data
OutputType main(InputType input)
{
    // Create an output structure to hold the processed vertex data
    OutputType output;

    // Pass the vertex position from input to the output (no modification here)
    output.position = input.position;
    
    // Pass the texture coordinates from input to the output (no modification here)
    output.tex = input.tex;
    
    // Return the processed output structure
    return output;
}