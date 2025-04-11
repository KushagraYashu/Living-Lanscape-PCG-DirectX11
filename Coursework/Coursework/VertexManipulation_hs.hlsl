// **Tessellation Hull Shader**  
// This shader prepares the control points for tessellation in a triangle patch.

static const int lightSize = 2; // Number of lights in the scene

// **Constant Buffers**  
// MatrixBuffer: Contains transformation matrices for the scene (world, view, projection) and light views/projections for each light
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix; // World transformation matrix
    matrix viewMatrix; // View transformation matrix
    matrix projectionMatrix; // Projection transformation matrix
    matrix lightViewMatrix[lightSize]; // View matrices for each light
    matrix lightProjectionMatrix[lightSize]; // Projection matrices for each light
};

// Camera buffer: Contains the camera's position in world space
cbuffer CameraBuffer : register(b1)
{
    float4 cameraPosition; // Camera's position in world space
    float4 params;
};

// **Input and Output Structures**
// InputType: Structure for input data to the hull shader, representing the vertex position and texture coordinates.
struct InputType
{
    float3 position : POSITION; // Vertex position
    float2 tex : TEXCOORD0; // Texture coordinates
};

// ConstantOutputType: Output structure for tessellation factors for the edges and inside tessellation factor.
struct ConstantOutputType
{
    float edges[3] : SV_TessFactor; // Tessellation factors for each edge of the triangle
    float inside : SV_InsideTessFactor; // Tessellation factor for the inside of the triangle
};

// OutputType: Output structure for the final vertex position and texture coordinates for each tessellated control point.
struct OutputType
{
    float3 position : POSITION; // Final position of the control point after tessellation
    float2 tex : TEXCOORD0; // Final texture coordinates
};

// **Tessellation Factor Calculation**  
// This function calculates the tessellation factor based on the distance between the camera and the midpoint of the triangle's edges.
float CalculateTessellationFactor(float3 pointA, float3 pointB, float3 camPosition)
{
    float3 edgeMidpoint = (pointA + pointB) * 0.5; // Midpoint of the edge

    float distance = length(camPosition - edgeMidpoint); // Distance from the camera to the edge midpoint

    // Minimum and maximum distance values for adjusting tessellation factor
    float d_min = 1.0f;
    float d_max = 23.0;

    // Normalize the distance to [0, 1] and calculate the tessellation factor based on the distance
    float t = saturate((distance - d_min) / (d_max - d_min));
    float tessFactor = lerp(64.0f, 1.0f, t); // Linearly interpolate between maximum and minimum tessellation factors

    return tessFactor; // Return the computed tessellation factor
}

// **Patch Constant Function**  
// This function calculates tessellation factors for the edges and inside of the triangle patch.
ConstantOutputType PatchConstantFunction(InputPatch<InputType, 3> inputPatch, uint patchId : SV_PrimitiveID)
{
    ConstantOutputType output;

    // Calculate tessellation factors for each edge of the triangle
    output.edges[0] = CalculateTessellationFactor(inputPatch[1].position, inputPatch[2].position, cameraPosition.xyz);
    output.edges[1] = CalculateTessellationFactor(inputPatch[2].position, inputPatch[0].position, cameraPosition.xyz);
    output.edges[2] = CalculateTessellationFactor(inputPatch[0].position, inputPatch[1].position, cameraPosition.xyz);

    // Calculate the average tessellation factor for the inside of the triangle
    output.inside = (output.edges[0] + output.edges[1] + output.edges[2]) / 3.0;

    return output; // Return the computed tessellation factors
}

// **Main Function**  
// This is the entry point for the hull shader, where the control points for tessellation are defined.
[domain("tri")] // Specifies that this is a domain shader for triangles
[partitioning("integer")] // Partitioning method for the tessellation control points
[outputtopology("triangle_cw")] // Output topology (clockwise triangles)
[outputcontrolpoints(3)] // Output 3 control points (for triangles)
[patchconstantfunc("PatchConstantFunction")] // Patch constant function used for tessellation factor calculations
OutputType main(InputPatch<InputType, 3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    OutputType output;

    // Set the position for this control point as the output position.
    output.position = patch[pointId].position; // Pass the position from the input patch to the output

    // Set the input texture coordinates as the output texture coordinates.
    output.tex = patch[pointId].tex; // Pass the texture coordinates from the input patch to the output

    return output; // Return the final output control point
}