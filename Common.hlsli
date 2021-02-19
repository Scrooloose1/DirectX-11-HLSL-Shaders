//--------------------------------------------------------------------------------------
// Common include file for all shaders
//--------------------------------------------------------------------------------------
// Using include files to define the type of data passed between the shaders


//--------------------------------------------------------------------------------------
// Shader input / output
//--------------------------------------------------------------------------------------

// The structure below describes the vertex data to be sent into the vertex shader.
struct BasicVertex
{
    float3 position : position;
    float3 normal   : normal;
    float2 uv       : uv;
};


// The most basic pixel shader input, just the screen space position for the pixel
struct BasicPixelShaderInput
{
    float4 projectedPosition : SV_Position;
};

struct TangentVertex
{
    float3 position : position;
    float3 normal : normal;
    float3 tangent : tangent;
    float2 uv : uv;
};


// This structure describes what data the lighting pixel shader receives from the vertex shader.
// The projected position is a required output from all vertex shaders - where the vertex is on the screen
// The world position and normal at the vertex are sent to the pixel shader for the lighting equations.
// The texture coordinates (uv) are passed from vertex shader to pixel shader unchanged to allow textures to be sampled
struct LightingPixelShaderInput
{
    float4 projectedPosition : SV_Position; // This is the position of the pixel to render, this is a required input
                                            // to the pixel shader and so it uses the special semantic "SV_Position"
                                            // because the shader needs to identify this important information
    
    float3 worldPosition : worldPosition;   // The world position and normal of each vertex is passed to the pixel...
    float3 worldNormal   : worldNormal;     //...shader to calculate per-pixel lighting. These will be interpolated
                                            // automatically by the GPU (rasterizer stage) so each pixel will know
                                            // its position and normal in the world - required for lighting equations
    
    float2 uv : uv; // UVs are texture coordinates. The artist specifies for every vertex which point on the texture is "pinned" to that vertex.
};

struct WigglePixelShaderInput
{
    float4 projectedPosition : SV_Position; // This is the position of the pixel to render, this is a required input
                                            // to the pixel shader and so it uses the special semantic "SV_Position"
                                            // because the shader needs to identify this important information
    
    float3 worldPosition : worldPosition; // The world position and normal of each vertex is passed to the pixel...
    float3 worldNormal : worldNormal; //...shader to calculate per-pixel lighting. These will be interpolated
                                            // automatically by the GPU (rasterizer stage) so each pixel will know
                                            // its position and normal in the world - required for lighting equations
    
    float2 uv : uv; // UVs are texture coordinates. The artist specifies for every vertex which point on the texture is "pinned" to that vertex.
};

struct TwoTexturesPixelShaderInput
{
    float4 projectedPosition : SV_Position; // This is the position of the pixel to render, this is a required input
                                            // to the pixel shader and so it uses the special semantic "SV_Position"
                                            // because the shader needs to identify this important information
    
    float3 worldPosition : worldPosition; // The world position and normal of each vertex is passed to the pixel...
    float3 worldNormal : worldNormal; //...shader to calculate per-pixel lighting. These will be interpolated
                                            // automatically by the GPU (rasterizer stage) so each pixel will know
                                            // its position and normal in the world - required for lighting equations
    
    float2 uv : uv; // UVs are texture coordinates. The artist specifies for every vertex which point on the texture is "pinned" to that vertex.
};

struct AlphaPixelShaderInput
{
    float4 projectedPosition : SV_Position; // This is the position of the pixel to render, this is a required input
                                            // to the pixel shader and so it uses the special semantic "SV_Position"
                                            // because the shader needs to identify this important information
    
    float3 worldPosition : worldPosition; // The world position and normal of each vertex is passed to the pixel...
    float3 worldNormal : worldNormal; //...shader to calculate per-pixel lighting. These will be interpolated
                                            // automatically by the GPU (rasterizer stage) so each pixel will know
                                            // its position and normal in the world - required for lighting equations
    
    float2 uv : uv; // UVs are texture coordinates. The artist specifies for every vertex which point on the texture is "pinned" to that vertex.
};

struct NormalMappingPixelShaderInput
{
    float4 projectedPosition : SV_Position; // This is the position of the pixel to render, this is a required input
                                            // to the pixel shader and so it uses the special semantic "SV_Position"
                                            // because the shader needs to identify this important information
    
    float3 worldPosition : worldPosition; // Data required for lighting calculations in the pixel shader
    float3 modelNormal   : modelNormal; // --"--
    float3 modelTangent  : modelTangent; // --"--
    
    float2 uv : uv; // UVs are texture coordinates. The artist specifies for every vertex which point on the texture is "pinned" to that vertex.
};




// This structure is similar to the one above but for the light models, which aren't themselves lit
struct SimplePixelShaderInput
{
    float4 projectedPosition : SV_Position;
    float2 uv : uv;
};




//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

// These structures are "constant buffers" - a way of passing variables over from C++ to the GPU
// They are called constants but that only means they are constant for the duration of a single GPU draw call.
// These "constants" correspond to variables in C++ that we will change per-model, or per-frame etc.

// In this exercise the matrices used to position the camera are updated from C++ to GPU every frame along with lighting information
// These variables must match exactly the gPerFrameConstants structure in Scene.cpp
cbuffer PerFrameConstants : register(b0) // The b0 gives this constant buffer the number 0 - used in the C++ code
{
    float4x4 gViewMatrix;
    float4x4 gProjectionMatrix;
    float4x4 gViewProjectionMatrix; // The above two matrices multiplied together to combine their effects

    float3   gLight1Position; // 3 floats: x, y z
    float    padding1;        // IMPORTANT technical point: shaders work with float4 values. If constant buffer variables don't align
                              // to the size of a float4 then HLSL (GPU) will insert padding, which can cause problems matching 
                              // structure between C++ and GPU. So add these unused padding variables to both HLSL and C++ structures.
    float3   gLight1Colour;
    float    padding2;

    float3   gLight2Position;
    float    padding3;
    float3   gLight2Colour;
    float    padding4;

    float3   gLight3Position;
    float    padding5;
    float3   gLight3Colour;
    float    padding6;

    float3   gLight4Position;
    float    padding7;
    float3   gLight4Colour;
    float    padding8;

    float3   gLight5Position;
    float    padding9;
    float3   gLight5Colour;
    float    padding10;

    float3   gAmbientColour;
    float    gSpecularPower;  // In this case we actually have a useful float variable that we can use to pad to a float4

    float3   gCameraPosition;
    float    padding11;

    float3   gOutlineColour; // Cell shading outline colour
    float    gOutlineThickness; // Controls thickness of outlines for cell shading
      

    
}
// Note constant buffers are not structs: we don't use the name of the constant buffer, these are really just a collection of global variables (hence the 'g')


// If we have multiple models then we need to update the world matrix from C++ to GPU multiple times per frame because we
// only have one world matrix here. Because this data is updated more frequently it is kept in a different buffer for better performance.
// We also keep other data that changes per-model here
// These variables must match exactly the gPerModelConstants structure in Scene.cpp
cbuffer PerModelConstants : register(b1) // The b1 gives this constant buffer the number 1 - used in the C++ code
{
    float4x4 gWorldMatrix;

    float3   gObjectColour;
    float    gWiggle;
    float    gLerp;
    float    gRotation;
    float    padding12;  // See notes on padding in structure above
}

float3 CalculateDiffuseLight(float3 lightPosition, float3 worldPosition, float3 lightColour, float3 worldNormal)
{
    float3 light1Vector = lightPosition - worldPosition;
    float3 lightDistance = length(light1Vector);
    float3 lightDirection = light1Vector / lightDistance;
    float3 diffuseLight = lightColour * max(dot(worldNormal, lightDirection), 0) / lightDistance;

    return diffuseLight;
}

float3 CalculateSpecularLight(float3 lightPosition, float3 worldPosition, float3 worldNormal, float3 cameraDirection, float specularPower, float3 diffuseLight)
{
    float3 light1Vector = lightPosition - worldPosition;
    float3 lightDistance = length(light1Vector);
    float3 lightDirection = light1Vector / lightDistance;
    float3 halfway = normalize(lightDirection + cameraDirection);
    float3 specularLight = diffuseLight * pow(max(dot(worldNormal, halfway), 0), specularPower);

    return specularLight;

}

float3 CalculateCellShadingDiffuseLight(float3 lightPosition, float3 worldPosition, float3 lightColour, float3 worldNormal, Texture2D cellMap, SamplerState pointSampleClamp)
{
    float3 lightVector      = lightPosition - worldPosition;
    float  lightDistance    = length(lightVector);
    float3 lightDirection   = lightVector / lightDistance; // Quicker than normalising as we have length for attenuation
    float  diffuseLevel     = max(dot(worldNormal, lightDirection), 0);
    float  cellDiffuseLevel = cellMap.Sample(pointSampleClamp, diffuseLevel).r;
    float3 diffuseLight     = lightColour * cellDiffuseLevel / lightDistance;

    return diffuseLight;

}


float3 CalculateLighting(float3 worldNormal, float3 worldPosition, Texture2D map, SamplerState TexSampler, float2 uv)
{
    worldNormal = normalize(worldNormal);
    float3 cameraDirection = normalize(gCameraPosition - worldPosition);

    float3 light1Vector = gLight1Position - worldPosition;
    float3 light1Dist = length(light1Vector);
    float3 light1Direction = light1Vector / light1Dist;
    float3 diffuseLight1 = gLight1Colour * max(dot(worldNormal, light1Direction), 0) / light1Dist;
    float3 halfway = normalize(light1Direction + cameraDirection);
    float3 specularLight1 = diffuseLight1 * pow(max(dot(worldNormal, halfway), 0), gSpecularPower);

    float3 light2Vector = gLight2Position - worldPosition;
    float3 light2Dist = length(light2Vector);
    float3 light2Direction = light2Vector / light2Dist;
    float3 diffuseLight2 = gLight2Colour * max(dot(worldNormal, light2Direction), 0) / light2Dist;
    halfway = normalize(light2Direction + cameraDirection);
    float3 specularLight2 = diffuseLight2 * pow(max(dot(worldNormal, halfway), 0), gSpecularPower);

    float3 light3Vector = gLight3Position - worldPosition;
    float3 light3Dist = length(light3Vector);
    float3 light3Direction = light3Vector / light3Dist;
    float3 diffuseLight3 = gLight3Colour * max(dot(worldNormal, light3Direction), 0) / light3Dist;
    halfway = normalize(light3Direction + cameraDirection);
    float3 specularLight3 = diffuseLight3 * pow(max(dot(worldNormal, halfway), 0), gSpecularPower);

    float3 light4Vector = gLight4Position - worldPosition;
    float3 light4Dist = length(light4Vector);
    float3 light4Direction = light4Vector / light4Dist;
    float3 diffuseLight4 = gLight4Colour * max(dot(worldNormal, light4Direction), 0) / light4Dist;
    halfway = normalize(light4Direction + cameraDirection);
    float3 specularLight4 = diffuseLight4 * pow(max(dot(worldNormal, halfway), 0), gSpecularPower);

    float3 light5Vector = gLight5Position - worldPosition;
    float3 light5Dist = length(light5Vector);
    float3 light5Direction = light5Vector / light5Dist;
    float3 diffuseLight5 = gLight5Colour * max(dot(worldNormal, light5Direction), 0) / light5Dist;
    halfway = normalize(light5Direction + cameraDirection);
    float3 specularLight5 = diffuseLight5 * pow(max(dot(worldNormal, halfway), 0), gSpecularPower);

    float4 textureColour = map.Sample(TexSampler, uv);

    float3 diffuseMaterialColour = textureColour.rgb;
    float specularMaterialColour = textureColour.a;

    // Combine lighting with texture colours
    float3 finalColour = (gAmbientColour + diffuseLight1 + diffuseLight2 + diffuseLight3 + diffuseLight4 + diffuseLight5) * diffuseMaterialColour +
                         (specularLight1 + specularLight2 + specularLight3 + specularLight4 + specularLight5) * specularMaterialColour;

    return finalColour;

}