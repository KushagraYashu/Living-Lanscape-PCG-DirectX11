// Texture and sampler for applying textures
Texture3D texture0 : register(t0); // The density texture
Texture2D depthTex : register(t1); // The linear depth texture
SamplerState Sampler0 : register(s0); // The sampler state for the texture

// Constant buffer for camera position
cbuffer CameraBuffer : register(b0)
{
    float4 camPos;
}

// Constant buffer for cloud box size and centre
cbuffer CloudBoxBuffer : register(b1)
{
    float4 centre;
    float4 halfSize;
}

// Constant buffer for light properties
cbuffer LightBuffer : register(b2)
{
    float4 lightDirectionAndSigma;
    float3 lightColor;
    float randVal;
}

// Constant buffer for scrolling speed and time
cbuffer ScrollBuffer : register(b3)
{
    float2 scrollSpeed;
    float time;
    float padding;
}

// Constant buffer for gas properties
cbuffer GasPropBuffer : register(b4)
{
    float4 gasColour;
    float3 sA_SamNo_G; // .x is sigma_a, .y is total sample numbers in ray marching, .z is phase function parameter
    float gasDensity;
}

// Input structure containing vertex attributes
struct InputType
{
    float4 position : SV_POSITION; // Vertex position
    float2 tex : TEXCOORD0; // Texture coordinates for mapping the texture
    float3 normal : NORMAL; // Normal vector for lighting (not used here, but passed through)
    float4 worldPos : POSITION; // World position of the pixel
};

// Computes intersection of a ray with a box
bool intersectAABB(float3 rayOrigin, float3 rayDir, float3 boxMin, float3 boxMax, out float t0, out float t1)
{
    float3 tMin = (boxMin - rayOrigin) / rayDir;
    float3 tMax = (boxMax - rayOrigin) / rayDir;

    float3 t1s = min(tMin, tMax); // entry points
    float3 t2s = max(tMin, tMax); // exit points

    t0 = max(max(t1s.x, t1s.y), t1s.z);
    t1 = min(min(t2s.x, t2s.y), t2s.z);

    return t1 >= max(t0, 0.0);
}

// the Henyey-Greenstein phase function (isotropy depends on the value of g)
float HGPhaseFunc(float g, float cos_theta)
{
    float denom = 1 + g * g - 2 * g * cos_theta;
    return 1 / (4 * 3.14159265358979323846) * (1 - g * g) / (denom * sqrt(denom));
}

// black background color for blending
static const float4 backgroundColor = float4(0, 0., 0., 1);

// Main pixel shader function
float4 main(InputType input) : SV_TARGET
{
    float4 finalColor = float4(0, 0, 0, 0); // Final color
    
    float3 rayDir = normalize(input.worldPos.xyz - camPos.xyz); // ray direction
    float3 rayOrigin = camPos;  // ray start position
    
    float3 boxMin = centre - halfSize;
    float3 boxMax = centre + halfSize;

    float t0, t1;
    if (!intersectAABB(rayOrigin, rayDir, boxMin, boxMax, t0, t1))
    {
        return backgroundColor; // No intersection, return background
    }

    // Depth test
    int width, height;
    depthTex.GetDimensions(width, height);
    float2 samplingPos = input.position.xy / float2(width, height);
    float zFar = 200;
    float depth = depthTex.Sample(Sampler0, samplingPos).r;
    depth *= zFar;
    if (t0 > depth)
    {
        return backgroundColor; // If something exists between the cloud and camera, return
    }
    t1 = min(t1, depth); // Adjusting exit parameter based on depth

    // Calculating entry and exit points
    float3 entryPoint;
    if (t0 >= 0)    // The ray starts out of the cloud box
    {
        entryPoint = rayOrigin + t0 * rayDir;
    }
    else    // The ray starts from inside the clouds (camera is in cloud box)
    {
        entryPoint = rayOrigin;
        t0 = 0; // This makes sure the calculations are correct in terms of the distance
    }
    float3 exitPoint = rayOrigin + t1 * rayDir;
    float distance = length(exitPoint - entryPoint);
    
    float stepSize = distance / sA_SamNo_G.y;
    float3 lightDir = -lightDirectionAndSigma.xyz;
    float sigma_s = lightDirectionAndSigma.w;
    float density = gasDensity; //higher the value, the thicker the gas
    float transparency = 1;
    float4 result = float4(0, 0, 0, 0);
    float cos_thetha;
    float rho;
    for (int i = 0; i < sA_SamNo_G.y; i++)  // Ray marching
    {
        float t = t0 + stepSize * (i + randVal); //parameter 't' to get a random point of the current sample (ray marching box)
        float3 samplePoint = rayOrigin + t * rayDir;
        
        // evaluating the perlin value
        float3 uvw = (samplePoint - boxMin) / (boxMax - boxMin);
        uvw += float3(scrollSpeed.x * time, 0, scrollSpeed.y * time);
        float perlinVal = texture0.SampleLevel(Sampler0, uvw, 0).r;
        
        // Combined density
        rho = density * (perlinVal * 0.5 + 0.5);
        
        // Scattering coefficient (used in Beer's law)
        float _scattCoeff = (sA_SamNo_G.x + sigma_s) * rho;
        
        float sampleTransparency = exp(-stepSize * _scattCoeff); //applying beer's law to the sample hence using stepSize
        
        transparency *= sampleTransparency; //integrating it with existing transparency
        
        if (transparency < 0.001) //forward marching, we dont want to go near to 0 (its just not worth it)
        {
            transparency = 0.001f;
            break;
        }
        
        float light_t0, light_t1; //t0 would always be 0 since this ray starts from inside the sphere
        if (intersectAABB(samplePoint, lightDir, boxMin, boxMax, light_t0, light_t1))
        {
            cos_thetha = dot(normalize(lightDir), normalize(-rayDir)); // cos_thetha for phase function
            
            float lightFalloffFromAbsorption = exp(-light_t1 * _scattCoeff); //beer's law to the ray coming from the light source to the sample
            
            // Integrating all parameters
            result += transparency * HGPhaseFunc(sA_SamNo_G.z, cos_thetha) * float4(max(saturate(lightColor.rgb * 15), 0.01), 1) * lightFalloffFromAbsorption * sigma_s * stepSize;
        }
    }
    
    // Final blend with background
    finalColor.rgb = (backgroundColor.rgb * transparency) + result.rgb;
    
    // Gamma correction
    finalColor.rgb = pow(finalColor.rgb, 1 / 2.2);
    
    // returning with final attenuation, it'd be useful in blending with other render texture
    return float4(finalColor.rgb, transparency);
}