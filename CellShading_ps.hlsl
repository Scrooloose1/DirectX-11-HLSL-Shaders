//--------------------------------------------------------------------------------------
// Pixel shader for cell shading
//--------------------------------------------------------------------------------------

#include "Common.hlsli" // Shaders can also use include files - note the extension


//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// Here we allow the shader access to a texture that has been loaded from the C++ side and stored in GPU memory.
// Note that textures are often called maps (because texture mapping describes wrapping a texture round a mesh).
// Get used to people using the word "texture" and "map" interchangably.
Texture2D DiffuseMap : register(t0); // Diffuse map only
Texture2D CellMap : register(t1); // CellMap is a 1D map that is used to limit the range of colours used in cell shading

SamplerState TexSampler : register(s0); // Sampler for use on textures
SamplerState PointSampleClamp : register(s1); // No filtering of cell maps (otherwise the cell edges would be blurred)


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

// Pixel shader entry point - each shader has a "main" function
// This shader just samples a diffuse texture map
float4 main(LightingPixelShaderInput input) : SV_Target
{
    // Lighting equations
    input.worldNormal = normalize(input.worldNormal); // Normal might have been scaled by model scaling or interpolation so renormalise
    float3 cameraDirection = normalize(gCameraPosition - input.worldPosition);

    // Light 1
    float3 diffuseLight1  = CalculateCellShadingDiffuseLight(gLight1Position, input.worldPosition, gLight1Colour, input.worldNormal, CellMap, PointSampleClamp);
    float3 specularLight1 = CalculateSpecularLight(gLight1Position, input.worldPosition, input.worldNormal, cameraDirection, gSpecularPower, diffuseLight1);

    // Light 2
    float3 diffuseLight2 = CalculateCellShadingDiffuseLight(gLight2Position, input.worldPosition, gLight2Colour, input.worldNormal, CellMap, PointSampleClamp);
    float3 specularLight2 = CalculateSpecularLight(gLight2Position, input.worldPosition, input.worldNormal, cameraDirection, gSpecularPower, diffuseLight2);

    // Light 3
    float3 diffuseLight3 = CalculateCellShadingDiffuseLight(gLight3Position, input.worldPosition, gLight3Colour, input.worldNormal, CellMap, PointSampleClamp);
    float3 specularLight3 = CalculateSpecularLight(gLight3Position, input.worldPosition, input.worldNormal, cameraDirection, gSpecularPower, diffuseLight3);

    // Light 4
    float3 diffuseLight4 = CalculateCellShadingDiffuseLight(gLight4Position, input.worldPosition, gLight4Colour, input.worldNormal, CellMap, PointSampleClamp);
    float3 specularLight4 = CalculateSpecularLight(gLight4Position, input.worldPosition, input.worldNormal, cameraDirection, gSpecularPower, diffuseLight4);

    // Light 5
    float3 diffuseLight5 = CalculateCellShadingDiffuseLight(gLight5Position, input.worldPosition, gLight5Colour, input.worldNormal, CellMap, PointSampleClamp);
    float3 specularLight5 = CalculateSpecularLight(gLight5Position, input.worldPosition, input.worldNormal, cameraDirection, gSpecularPower, diffuseLight5);

    // Sample diffuse material colour for this pixel from a texture using a given sampler that you set up in the C++ code
    // Ignoring any alpha in the texture, just reading RGB
    float4 textureColour = DiffuseMap.Sample(TexSampler, input.uv);
    float3 diffuseMaterialColour = textureColour.rgb;
    float specularMaterialColour = textureColour.a;

    float3 finalColour = (gAmbientColour + diffuseLight1 + diffuseLight2 + diffuseLight3 + diffuseLight4 + diffuseLight5) * diffuseMaterialColour +
                         (specularLight1 + specularLight2 + specularLight3 + specularLight4 + specularLight5) * specularMaterialColour;

    return float4(finalColour, 1.0f); // Always use 1.0f for alpha - no alpha blending in this lab
}