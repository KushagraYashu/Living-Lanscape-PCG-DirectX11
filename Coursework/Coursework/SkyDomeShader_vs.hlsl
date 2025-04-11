// **Constant Buffers**
// Holds transformation matrices for rendering the geometry.
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix; // World transformation matrix.
    matrix viewMatrix; // View transformation matrix (camera position and orientation).
    matrix projectionMatrix; // Projection matrix for camera perspective.
};

// **Input Structure**  
// Represents the data passed to the vertex shader from the input assembler (IA stage).
struct InputType
{
    float4 position : POSITION; // Position of the vertex in 3D space.
};

// **Output Structure**  
// Represents the data that will be passed from the vertex shader to the pixel shader.
struct OutputType
{
    float4 position : SV_POSITION; // The final position of the vertex after transformation.
    float4 domePosition : TEXCOORD0; // Unmodified position to be passed to the pixel shader.
};

// **Main Vertex Shader Function**
// This function transforms the input vertex position by the world, view, and projection matrices.
OutputType main(InputType input)
{
    OutputType output;
    
    // Set the w component of the position to 1.0 for correct transformation (homogeneous coordinates).
    input.position.w = 1.0f;

    // **Transformation Calculations**  
    // First, the vertex position is multiplied by the world matrix, which applies object transformation.
    output.position = mul(input.position, worldMatrix);
    
    // Then, the position is transformed by the view matrix, which adjusts for the camera's position.
    output.position = mul(output.position, viewMatrix);
    
    // Finally, the position is multiplied by the projection matrix, converting the coordinates into screen space.
    output.position = mul(output.position, projectionMatrix);
    
    // Pass the original position (before transformation) as texture coordinates for the pixel shader.
    output.domePosition = input.position;

    return output; // Return the transformed vertex data to the next pipeline stage.
}