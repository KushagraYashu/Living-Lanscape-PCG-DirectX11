// Define the number of light sources
static const int lightSize = 2;

// Texture and sampler registers
Texture2D texture0 : register(t0); // The main texture
SamplerState sampler0 : register(s0); // The sampler for the texture
Texture2D heightMap : register(t1); // The height map texture used for terrain manipulation

// Depth map textures for each light source (used for shadow mapping)
Texture2D depthMapTexture[lightSize] : register(t2);

// Constant buffer containing light properties
cbuffer LightBuffer : register(b0)
{
    float4 ambientColour[lightSize]; // Ambient color for each light source
    float4 diffuseColour[lightSize]; // Diffuse color for each light source
    float4 lightDirection[lightSize]; // Direction of light sources
    float4 lightPosition[lightSize]; // Position of light sources
    float4 specularColour[lightSize]; // Specular color for each light source
    float4 specularPower[lightSize]; // Specular power for each light source
    float4 type[lightSize]; // Type of light (directional, point, etc.)
    float4 attFactors[lightSize]; // Attenuation factors for each light source
};

// Structure to hold the input data for the vertex shader
struct InputType
{
    float4 position : SV_POSITION; // Vertex position in clip space
    float2 tex : TEXCOORD0; // Texture coordinates
    float4 depthPosition : TEXCOORD1; // Depth position for shadow mapping
    float4 vertexManipulationData : TEXCOORD2; // Data for vertex manipulation (e.g., height map strength)
    float3 viewVector : TEXCOORD3; // Vector pointing from the vertex to the camera
    float4 lightViewPos[lightSize] : TEXCOORD4; // View space positions for each light source
    float3 worldPosition : POSITION0; // Vertex position in world space
};

// Function to check if the texture coordinates are within valid bounds (0, 1).
bool hasDepthData(float2 uv)
{
    // Check if the coordinates are outside the range [0, 1] (invalid UV coordinates).
    if (uv.x < 0.f || uv.x > 1.f || uv.y < 0.f || uv.y > 1.f)
    {
        return false; // Invalid UV, return false
    }
    return true; // Valid UV, return true
}

// Function to determine if a pixel is in shadow based on the shadow map.
bool isInShadow(Texture2D sMap, float2 uv, float4 lightViewPosition, float bias)
{
    // Sample the shadow map at the given UV coordinates to get the depth of the geometry.
    float depthValue = sMap.Sample(sampler0, uv).r;

    // Calculate the depth from the light's point of view.
    float lightDepthValue = lightViewPosition.z / lightViewPosition.w; // Perspective divide
    lightDepthValue -= bias; // Apply a small bias to prevent self-shadowing issues (shadow acne)

    // Compare the depth values to decide if the pixel is in shadow.
    if (lightDepthValue < depthValue)
    {
        return false; // The pixel is lit
    }
    return true; // The pixel is in shadow
}

// Function to calculate the projected texture coordinates from the light's view position.
float2 getProjectiveCoords(float4 lightViewPosition)
{
    // Project the light view position into texture coordinates.
    float2 projTex = lightViewPosition.xy / lightViewPosition.w; // Perspective divide
    projTex *= float2(0.5, -0.5); // Scale to match the texture coordinate system
    projTex += float2(0.5f, 0.5f); // Translate into the [0, 1] coordinate space
    return projTex; // Return the computed texture coordinates
}

// Function to calculate attenuation based on the constant, linear, and quadratic factors and the distance.
float calcAttenuation(float constFac, float linFac, float quadFac, float dist)
{
    // The attenuation formula takes into account the distance-based factors and returns the attenuation.
    float attenuation = 1 / (constFac + (linFac * dist) + (quadFac * pow(dist, 2))); //calculating attenuation
    
    return attenuation; //returning attenuation
}

// Function to calculate the specular lighting using the Blinn-Phong model.
float4 calcSpecularLighting(float3 lightDirection, float3 normal, float3 viewVector, float4 type, float4 specularColour, float4 specularPower)
{
	// Blinn-Phong specular calculation
    float3 lightNor;
    float3 halfway;
    float specularIntensity;
    float4 finalSpecular = (0.0f, 0.0f, 0.0f, 0.0f); // Initialize specular result

    // If the light type is directional, calculate the specular component.
    if (type.y == 1.0f) //directional light
    {
        lightNor = normalize(lightDirection); // normalize the light direction
        halfway = normalize((-lightNor) + viewVector); // calculate the halfway vector
        specularIntensity = pow(max(dot(normal, halfway), 0.0), specularPower.x); // specular intensity
        finalSpecular += specularColour * specularIntensity; // add specular reflection
    }
    else
    {
        return finalSpecular; // No specular component for other light types
    }
    
    return saturate(finalSpecular); // clamp the result to [0, 1]
}

// Function to calculate spotlight lighting.
float4 calcSpotLighting(float4 lightDirection, float3 lightVector, float3 normal, float dist, float4 diffuseColour, float4 attFactors, float4 type)
{
    float intensity;
    float4 colour = (0.f, 0.f, 0.f, 0.f); // initialize colour

    // If light type is not spotlight, return zero colour.
    if (type.z != 1) //checking if light is defined as spotlight type
    {
        return colour;
    }

    // Calculate the spot factor (the cosine of the angle between light direction and light vector).
    float spotFactor = max(dot(-lightVector, lightDirection.xyz), 0);

    // If the spot factor exceeds the cutoff value, calculate the spotlight intensity.
    if (spotFactor > 0.866) //0.866 is the cosine of the cutoff angle (30 degrees)
    {
        intensity = saturate(dot(normal, lightVector)); // calculate diffuse intensity
        float spotLightIntensity = (1.f - ((1.f - spotFactor) / (1.f - 0.866))); // spotlight intensity based on angle
        colour = diffuseColour * intensity * spotLightIntensity; // calculate final spotlight colour
        colour = saturate(colour * calcAttenuation(attFactors.x, attFactors.y, attFactors.z, dist)); // apply attenuation
        return colour;
    }
    else
    {
        return float4(0, 0, 0, 0); // if not within spotlight range, return black
    }
}

// Function to calculate directional lighting.
float4 calcDirectionalLighting(float4 lightDirection, float3 normal, float4 diffuseColour, float4 type)
{
    float intensity;
    float4 colour = (0.0f, 0.0f, 0.0f, 0.0f); // initialize colour

    // If the light type is not directional, return zero colour.
    if (type.y != 1.0f)
        return colour;

    // Calculate the intensity of the light based on the dot product of the normal and the light direction.
    intensity = saturate(dot(normal, -lightDirection.xyz)); // calculate diffuse intensity
    colour += saturate(diffuseColour * intensity); // add diffuse colour

    return colour; // return the final colour
}

// Function to get the height from the height map at the given UV coordinates.
// Uses vertexManipulationData.x as a scaling factor for the height.
float GetHeight(float2 UV, float4 vertexManipulationData)
{
    // Sample the height map at the given UV coordinates.
    float offset = heightMap.SampleLevel(sampler0, UV, 0).r;

    // Return the height value scaled by the vertex manipulation factor (x component).
    return offset * vertexManipulationData.x;
}

// Function to calculate the normal vector at a given UV coordinate based on the height map.
// Uses vertex manipulation data for scaling.
float3 CalcNormal(float2 UV, float4 vertexManipulationData)
{
    float tw = 256.0f; // Texture width (height map dimensions)
    float val;

    // Get the dimensions of the height map (assuming square texture).
    heightMap.GetDimensions(0, tw, tw, val);

    // Calculate UV offset (sub-sampling rate).
    float uvOff = 1.f / tw;

    // World space step size, which affects the normal calculation.
    float worldStep = 50.0f * uvOff;

    // Get height values at adjacent points in world space.
    float heightUp = GetHeight(float2(UV.x, UV.y + uvOff), vertexManipulationData);
    float heightDown = GetHeight(float2(UV.x, UV.y - uvOff), vertexManipulationData);
    float heightRight = GetHeight(float2(UV.x + uvOff, UV.y), vertexManipulationData);
    float heightLeft = GetHeight(float2(UV.x - uvOff, UV.y), vertexManipulationData);

    // Get the height at the current point.
    float height = GetHeight(UV, vertexManipulationData);

    // Calculate tangent vectors based on neighboring heights (for cross-product normal calculation).
    float3 tan1 = normalize(float3(1.0f * worldStep, heightRight - height, 0)); // Right direction
    float3 tan2 = normalize(float3(-1.0f * worldStep, heightLeft - height, 0)); // Left direction
    float3 bitan1 = normalize(float3(0, heightUp - height, 1.0f * worldStep)); // Up direction
    float3 bitan2 = normalize(float3(0, heightDown - height, -1.0f * worldStep)); // Down direction

    // Calculate normals using cross products of tangent vectors.
    float3 normal1 = normalize(cross(tan1, bitan2));
    float3 normal2 = normalize(cross(bitan2, tan2));
    float3 normal3 = normalize(cross(tan2, bitan1));
    float3 normal4 = normalize(cross(bitan1, tan1));

    // Combine the normals from all four cross products and normalize.
    return (normal1 + normal2 + normal3 + normal4) * .25f;
}

// Main pixel shader function
// Applies lighting calculations and shadow detection to the texture
float4 main(InputType input) : SV_TARGET
{
    // Sample the texture at the provided texture coordinates
    float4 textureColour;
    textureColour = texture0.Sample(sampler0, input.tex);
    
    // Calculate the normal using the texture and vertex manipulation data
    float3 newNormal = CalcNormal(input.tex, input.vertexManipulationData);
    float3 normal;
    normal = newNormal;
    
    // Initialize light vectors and directions for multiple lights
    float3 lightVector[lightSize];
    float4 lightDirectionNor[lightSize];
    
    // Calculate the vector from each light to the current pixel's world position
    for (int i = 0; i < lightSize; i++)
        lightVector[i] = lightPosition[i].xyz - input.worldPosition;
    
    // Calculate the distance from the light to the pixel
    float distance[lightSize];
    for (int i = 0; i < lightSize; i++)
        distance[i] = length(lightVector[i]);
    
    // Normalize the light vectors and directions
    for (int i = 0; i < lightSize; i++)
    {
        lightVector[i] = normalize(lightVector[i]);
        lightDirectionNor[i] = normalize(lightDirection[i]);
    }
    
    // Define a bias to adjust the depth comparison for shadows
    float shadowMapBias = 0.005f;
    float4 lightColour = (0.0f, 0.0f, 0.0f, 0.0f); // Initialize light color

    // Loop through each light to calculate its contribution
    for (int i = 0; i < lightSize; i++)
    {
        float2 pTexCoord;
        
        // Calculate projective texture coordinates for shadow mapping
        pTexCoord = getProjectiveCoords(input.lightViewPos[i]);
        
        // Check if the texture coordinates are within valid depth map range
        if (hasDepthData(pTexCoord))
        {
            // If the pixel is not in shadow, calculate lighting
            if (!isInShadow(depthMapTexture[i], pTexCoord, input.lightViewPos[i], shadowMapBias))
            {
                // Add contributions from different types of lighting: directional, spot, and specular
                lightColour = saturate(lightColour + calcDirectionalLighting(lightDirectionNor[i], normal, diffuseColour[i], type[i]));
                lightColour = saturate(lightColour + calcSpotLighting(lightDirectionNor[i], lightVector[i], normal, distance[i], diffuseColour[i], attFactors[i], type[i]));
                lightColour = saturate(lightColour + calcSpecularLighting(lightDirectionNor[i].xyz, normal, input.viewVector, type[i], specularColour[i], specularPower[i]));
            }
        }
    }
    
    // Add the ambient lighting contributions from all lights
    for (int i = 0; i < lightSize; i++)
    {
        lightColour = saturate(lightColour + ambientColour[i]);
    }

    // Convert the texture color to linear space by applying gamma correction (2.2)
    textureColour.xyz = pow(texture0.Sample(sampler0, input.tex), 2.2f);
    
    // Multiply the light color by the texture color to get the final color
    float4 finalColour = lightColour * textureColour;
    
    // Apply gamma correction to the final color
    finalColour.xyz = pow(finalColour.xyz, 1.0f / 2.2f);
    
    // Return the final color with applied lighting and texture
    return finalColour;
}