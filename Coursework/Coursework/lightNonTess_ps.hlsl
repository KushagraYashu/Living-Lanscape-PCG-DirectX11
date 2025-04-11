// Define the number of lights in the scene
static const int lightSize = 2;

// **Texture and Sampler Declarations**  
// Declare textures and samplers used in the shader.  
Texture2D texture0 : register(t0); // The main texture to be sampled in the fragment shader
SamplerState sampler0 : register(s0); // Sampler for the main texture
Texture2D heightMap : register(t1); // Height map texture used for height manipulation

Texture2D depthMapTexture[lightSize] : register(t2); // Depth maps for each light, used for shadowing

// **Constant Buffers for Light Parameters**  
// Buffer holding information for light properties such as color, position, and attenuation factors
cbuffer LightBuffer : register(b0)
{
    float4 ambientColour[lightSize]; // Ambient color for each light source
    float4 diffuseColour[lightSize]; // Diffuse color for each light source
    float4 lightDirection[lightSize]; // Direction of each light source (used for directional lights)
    float4 lightPosition[lightSize]; // Position of each light source (used for point and spotlights)
    float4 specularColour[lightSize]; // Specular color for each light source
    float4 specularPower[lightSize]; // Specular power (shininess) for each light source
    float4 type[lightSize]; // Type of the light (e.g., point, directional, spotlight)
    float4 attFactors[lightSize]; // Attenuation factors for each light source (used for point/spotlights)
};

// **Input Structure**  
// The input structure defines the data passed to the pixel shader from the vertex shader.
// It includes position, texture coordinates, depth, and lighting information for each vertex.
struct InputType
{
    float4 position : SV_POSITION; // The final transformed vertex position
    float2 tex : TEXCOORD0; // Texture coordinates to be used for texture sampling
    float4 depthPosition : TEXCOORD1; // Depth position of the vertex for shadow calculations
    float4 vertexManipulationData : TEXCOORD2; // Data used for vertex manipulation (e.g., height map strength)
    float3 viewVector : TEXCOORD3; // The vector from the pixel to the camera (view vector)
    float4 lightViewPos[lightSize] : TEXCOORD4; // The light view position for shadow map comparison
    float3 worldPosition : POSITION0; // The world position of the vertex
    float3 normal : NORMAL; // The normal vector at the vertex for lighting calculations
};

// Function to check if texture coordinates are within valid bounds (0, 1)
// Returns true if the coordinates are valid, false otherwise.
bool hasDepthData(float2 uv)
{
    if (uv.x < 0.f || uv.x > 1.f || uv.y < 0.f || uv.y > 1.f)
    {
        return false; // Return false if the coordinates are out of bounds
    }
    return true; // Return true if coordinates are within valid range
}

// Function to check if a pixel is in shadow based on its depth value in the shadow map.
// It compares the sampled depth value from the shadow map with the depth value of the light.
bool isInShadow(Texture2D sMap, float2 uv, float4 lightViewPosition, float bias)
{
    // Sample the shadow map to get the depth value of the pixel.
    float depthValue = sMap.Sample(sampler0, uv).r;
    
    // Calculate the light depth value using the light's view position.
    float lightDepthValue = lightViewPosition.z / lightViewPosition.w;
    lightDepthValue -= bias; // Apply a small bias to prevent shadow acne

    // If the pixel's depth is greater than the light's depth, it is not in shadow.
    if (lightDepthValue < depthValue)
    {
        return false; // Return false if the pixel is not in shadow
    }
    return true; // Return true if the pixel is in shadow
}

// Function to calculate the projected texture coordinates from the light's view position.
// This is used to map 3D positions into 2D texture space for shadow mapping.
float2 getProjectiveCoords(float4 lightViewPosition)
{
    // Perform perspective divide to get normalized device coordinates (NDC).
    float2 projTex = lightViewPosition.xy / lightViewPosition.w;

    // Scale and translate to fit texture coordinates in the range [0, 1].
    projTex *= float2(0.5, -0.5); // Scale to map to [0, 1] texture space
    projTex += float2(0.5f, 0.5f); // Translate to [0, 1] range

    return projTex; // Return the computed texture coordinates
}

// Function to calculate light attenuation based on distance from the light source.
// The attenuation is calculated using a constant, linear, and quadratic factor based on the distance.
float calcAttenuation(float constFac, float linFac, float quadFac, float dist)
{
    // Formula for calculating attenuation
    float attenuation = 1 / (constFac + (linFac * dist) + (quadFac * pow(dist, 2)));

    return attenuation; // Returning the calculated attenuation value
}

// Function to calculate the specular lighting based on the Blinn-Phong shading model.
// This is used to simulate shiny surfaces under lighting.
float4 calcSpecularLighting(float3 lightDirection, float3 normal, float3 viewVector, float4 type, float4 specularColour, float4 specularPower)
{
    // Initialize variables for Blinn-Phong specular calculation
    float3 lightNor;
    float3 halfway;
    float specularIntensity;
    float4 finalSpecular = float4(0.0f, 0.0f, 0.0f, 0.0f); // Default specular value

    if (type.y == 1.0f) // If the light is directional
    {
        lightNor = normalize(lightDirection); // Normalize the light direction
        halfway = normalize((-lightNor) + viewVector); // Halfway vector for Blinn-Phong
        specularIntensity = pow(max(dot(normal, halfway), 0.0), specularPower.x); // Calculate specular intensity
        finalSpecular += specularColour * specularIntensity; // Apply specular color and intensity
    }
    else
    {
        return finalSpecular; // If not directional light, return default specular value
    }

    return saturate(finalSpecular); // Ensure the result is clamped between 0 and 1
}

// Function to calculate the spot light's contribution to the final color.
// Uses the angle of the spot to calculate the intensity and applies attenuation based on distance.
float4 calcSpotLighting(float4 lightDirection, float3 lightVector, float3 normal, float dist, float4 diffuseColour, float4 attFactors, float4 type)
{
    float intensity;
    float4 colour = float4(0.f, 0.f, 0.f, 0.f); // Initialize color to black

    if (type.z != 1) // Check if the light is of type spotlight
    {
        return colour; // If not a spotlight, return black (no contribution)
    }

    // Calculate the spot factor based on the angle between the light direction and the light vector
    float spotFactor = max(dot(-lightVector, lightDirection.xyz), 0);

    if (spotFactor > 0.866) // Check if the spot factor is above the threshold (cosine of 30 degrees)
    {
        intensity = saturate(dot(normal, lightVector)); // Calculate intensity based on the normal and light vector
        float spotLightIntensity = (1.f - ((1.f - spotFactor) / (1.f - 0.866))); // Apply spotlight intensity based on angle
        colour = diffuseColour * intensity * spotLightIntensity; // Combine diffuse color and intensity

        // Apply attenuation based on distance to light
        colour = saturate(colour * calcAttenuation(attFactors.x, attFactors.y, attFactors.z, dist));
        return colour; // Return the final color after attenuation
    }
    else
    {
        return float4(0, 0, 0, 0); // Return black if the spot factor is too low (outside spotlight angle)
    }
}

// Function to calculate directional lighting (for lights like the sun).
// This function computes the lighting based on the light's direction and the surface normal.
float4 calcDirectionalLighting(float4 lightDirection, float3 normal, float4 diffuseColour, float4 type)
{
    float intensity;
    float4 colour = float4(0.0f, 0.0f, 0.0f, 0.0f); // Initialize color to black

    if (type.y != 1.0f) // Check if the light is of type directional
        return colour; // If not directional, return black (no contribution)

    intensity = saturate(dot(normal, -lightDirection.xyz)); // Calculate intensity based on the dot product of normal and light direction
    colour += saturate(diffuseColour * intensity); // Apply diffuse color based on intensity

    return colour; // Return the final color based on directional light
}

// Main Pixel Shader
// This shader computes lighting and texture mapping for each pixel in the scene.
float4 main(InputType input) : SV_TARGET
{
    // Sample the color from the texture using the provided texture coordinates
    float4 textureColour;
    textureColour = texture0.Sample(sampler0, input.tex);
    
    // Normalize the normal vector for the pixel
    float3 normal;
    normal = normalize(input.normal);
    
    // Calculate light vectors and normalize the direction of each light source
    float3 lightVector[lightSize];
    float4 lightDirectionNor[lightSize];
    
    // For each light source, calculate the vector pointing from the pixel to the light
    for (int i = 0; i < lightSize; i++)
        lightVector[i] = lightPosition[i].xyz - input.worldPosition;
    
    // Calculate the distance to each light source
    float distance[lightSize];
    for (int i = 0; i < lightSize; i++)
        distance[i] = length(lightVector[i]);
    
    // Normalize light vectors and light directions
    for (int i = 0; i < lightSize; i++)
    {
        lightVector[i] = normalize(lightVector[i]);
        lightDirectionNor[i] = normalize(lightDirection[i]);
    }
    
    // Define a small bias value for shadow maps to prevent self-shadowing artifacts
    float shadowMapBias = 0.005f;
    
    // Initialize the light color accumulator to black
    float4 lightColour = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // Loop through each light source
    for (int i = 0; i < lightSize; i++) // TODO: Fix shadow map issues
    {
        float2 pTexCoord;
        
        // Get the projected texture coordinates for the shadow map
        pTexCoord = getProjectiveCoords(input.lightViewPos[i]);
        
        // If the texture coordinates are valid
        if (hasDepthData(pTexCoord))
        {
            // Check if the current pixel is in shadow using the shadow map
            if (!isInShadow(depthMapTexture[i], pTexCoord, input.lightViewPos[i], shadowMapBias))
            {
                // Add the directional lighting contribution for this light source
                lightColour = saturate(lightColour + calcDirectionalLighting(lightDirectionNor[i], normal, diffuseColour[i], type[i]));
                
                // Add the spotlight contribution for this light source
                lightColour = saturate(lightColour + calcSpotLighting(lightDirectionNor[i], lightVector[i], normal, distance[i], diffuseColour[i], attFactors[i], type[i]));
                
                // Add the specular lighting contribution for this light source
                lightColour = saturate(lightColour + calcSpecularLighting(lightDirectionNor[i].xyz, normal, input.viewVector, type[i], specularColour[i], specularPower[i]));
            }
        }
    }
    
    // Add ambient light contributions from all light sources
    for (int i = 0; i < lightSize; i++)
    {
        lightColour = saturate(lightColour + ambientColour[i]);
    }
    
    // Convert the sampled texture color from gamma to linear space for proper lighting calculations
    textureColour.xyz = pow(texture0.Sample(sampler0, input.tex), 2.2f);
    
    // Calculate the final color by multiplying the light color with the texture color
    float4 finalColour = lightColour * textureColour;
    
    // Convert the final color back from linear space to gamma space
    finalColour.xyz = pow(finalColour.xyz, 1.0f / 2.2f);
    
    // Return the final calculated color for this pixel
    return finalColour;
}