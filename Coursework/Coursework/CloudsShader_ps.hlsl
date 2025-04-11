// Texture and sampler for applying textures
Texture2D texture0 : register(t0); // The texture to apply to the geometry
SamplerState Sampler0 : register(s0); // The sampler state for the texture

// Constant buffer for camera position and ray intersection test
cbuffer CameraBuffer : register(b0)
{
    float4 camPos;
}

cbuffer SphereBuffer : register(b1)
{
    float4 sphereCentreAndRadius;
}

cbuffer LightBuffer : register(b2)
{
    float4 lightDirectionAndSigma;
    float4 lightColor;
}

// Input structure containing vertex attributes
struct InputType
{
    float4 position : SV_POSITION; // Vertex position
    float2 tex : TEXCOORD0; // Texture coordinates for mapping the texture
    float3 normal : NORMAL; // Normal vector for lighting (not used here, but passed through)
    float4 worldPos : POSITION;
};

// Computes intersection of a ray with a sphere
bool intersectSphere(float3 ro, float3 rd, out float t0, out float t1)
{
    float3 sphere_center = sphereCentreAndRadius.xyz;
    float sphere_radius = sphereCentreAndRadius.w;
    
    
    float3 L = sphere_center - ro;
    float tca = dot(L, rd);
    
    float dSq = dot(L, L) - tca * tca;
    float rSq = sphere_radius * sphere_radius;
    
    if (dSq > rSq)
        return false;

    float thc = sqrt(rSq - dSq);
    
    
    t0 = tca - thc;
    t1 = tca + thc;
    
    if (dot(L, L) < rSq)
    {
        t0 = 0;
    }

    return true;
}

static const float4 backgroundColor = float4(0, 0, 0, 0);
static const float4 gasColor = float4(1, 1, 1, 1);
static const float sigma_a = .4f; //the higher, the more absorption (if want thick gas, increase the value)

static const float sampleNumbers = 200;


// Main pixel shader function
float4 main(InputType input) : SV_TARGET
{
    float3 rayDir = normalize(input.worldPos - camPos);
    float3 rayOrigin = camPos;

    float t0, t1;
    if (!intersectSphere(rayOrigin, rayDir, t0, t1))
    {
        return backgroundColor; // No intersection, return background
    }

    float3 entryPoint = rayOrigin + t0 * rayDir;
    float3 exitPoint = rayOrigin + t1 * rayDir;

    float distance = length(exitPoint - entryPoint);
    
    //In-Scattering
    float stepSize = distance / sampleNumbers;
    
        //ray-marching?
    float3 lightDir = -lightDirectionAndSigma.xyz;
    float sigma_s = lightDirectionAndSigma.w;
    float density = 1.f; //higher the value, the thicker the gas
    float transparency = 1;
    float4 result = float4(0, 0, 0, 0);
    for (int i = 0; i < sampleNumbers; i++)
    {
        float t = t0 + stepSize * (i + 0.5); //parameter 't' to get the mid point of the current sample (ray marching box) (+0.5 is done to get the centre)

        float3 sampleMidPoint = rayOrigin + t * rayDir;
        
        float sampleTransparency = exp(-stepSize * (density) * (sigma_a + sigma_s)); //applying beer's law to the sample hence using stepSize
        
        transparency *= sampleTransparency; //integrating it with existing transparency
        
        if (transparency < 0.01) //forward marching, we dont want to go near to 0 (its just not worth it)
        {
            transparency = 0.01f;
            //break;
        }
        
        //Now in-scattering (check with sphere and do shit)
        float light_t0, light_t1; //t0 would always be 0 since the ray starts from inside the sphere
        if (intersectSphere(sampleMidPoint, lightDir, light_t0, light_t1))
        {
            float lightFalloffFromAbsorption = exp(-light_t1 * (density) * (sigma_a + sigma_s)); //beer's law to the ray coming from the light source to the sample
            result += gasColor * float4(lightColor.rgb * 10 /*10 is intensity (sort of)*/, 1) * lightFalloffFromAbsorption * sigma_s * (density) * stepSize; //combining all the results
        }

    }
    
    //return float4(transparency, transparency, transparency, 1);
    
    // Final blend with background
    float4 finalColor = (backgroundColor + result) * (1 / transparency);
    finalColor.rgb = pow(finalColor.rgb, 1 / 2.2);
    return finalColor;
    
    //scrolling clouds (texture)
    //float4 colour; // Variable to store the texture color
    //float2 scrollUV = input.tex; // Get the texture coordinates from the input

    //// Apply scrolling effect by modifying the texture's x-coordinate based on the scroll speed
    //scrollUV.x += scrollSpeed.y * scrollSpeed.x; // Horizontal scroll based on speed

    //// Sample the texture at the modified coordinates
    //colour = texture0.Sample(Sampler0, scrollUV);

    //// Calculate the luminance of the texture color using a weighted sum of RGB channels
    //float luminance = dot(colour.rgb, float3(0.299, 0.587, 0.114));

    //// Calculate alpha based on the luminance
    //float alpha = pow(luminance, 2.0);

    //// Mix the original color with black based on the calculated alpha value
    //float4 finalColour = lerp(float4(0, 0, 0, 0), colour, alpha);

    //// Apply gamma correction to the final color to convert from linear to sRGB space
    //finalColour.rgb = pow(finalColour.rgb, 1 / 2.2);

    ////trying to attempt beer's law (volumetric rendering)
    ////float sigma_a = 0.1;
    ////float distance = 10;
    ////float T = exp(-distance * sigma_a);
    ////finalColour *= T;
    
    //return finalColour; // Return the final color to be rendered
}