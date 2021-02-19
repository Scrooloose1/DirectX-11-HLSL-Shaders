#include "Common.hlsli"

Texture2D    DiffuseSpecularMap  : register(t0);
Texture2D    DiffuseSpecularMap2 : register(t1);
SamplerState TexSampler          : register(s0);

float4 main(TwoTexturesPixelShaderInput input) : SV_Target
{
   
    // Lighting equations
    input.worldNormal = normalize(input.worldNormal); // Normal might have been scaled by model scaling or interpolation so renormalise
    float3 cameraDirection = normalize(gCameraPosition - input.worldPosition);

    // Light 1
    float3 diffuseLight1 = CalculateDiffuseLight(gLight1Position, input.worldPosition, gLight1Colour, input.worldNormal);
    float3 specularLight1 = CalculateSpecularLight(gLight1Position, input.worldPosition, input.worldNormal, cameraDirection, gSpecularPower, diffuseLight1);

    // Light 2
    float3 diffuseLight2 = CalculateDiffuseLight(gLight2Position, input.worldPosition, gLight2Colour, input.worldNormal);
    float3 specularLight2 = CalculateSpecularLight(gLight2Position, input.worldPosition, input.worldNormal, cameraDirection, gSpecularPower, diffuseLight2);

    // Light 3
    float3 diffuseLight3 = CalculateDiffuseLight(gLight3Position, input.worldPosition, gLight3Colour, input.worldNormal);
    float3 specularLight3 = CalculateSpecularLight(gLight3Position, input.worldPosition, input.worldNormal, cameraDirection, gSpecularPower, diffuseLight3);

    // Light 4
    float3 diffuseLight4 = CalculateDiffuseLight(gLight4Position, input.worldPosition, gLight4Colour, input.worldNormal);
    float3 specularLight4 = CalculateSpecularLight(gLight4Position, input.worldPosition, input.worldNormal, cameraDirection, gSpecularPower, diffuseLight4);

    // Light 5
    float3 diffuseLight5 = CalculateDiffuseLight(gLight5Position, input.worldPosition, gLight5Colour, input.worldNormal);
    float3 specularLight5 = CalculateSpecularLight(gLight5Position, input.worldPosition, input.worldNormal, cameraDirection, gSpecularPower, diffuseLight5);

    // Sample diffuse material and specular material colour for this pixel from a texture using a given sampler that you set up in the C++ code
    float4 textureColour = DiffuseSpecularMap.Sample(TexSampler, input.uv);
    float4 textureColour2 = DiffuseSpecularMap2.Sample(TexSampler, input.uv);

    float3 diffuseMaterialColour = lerp(textureColour.rgb, textureColour2.rgb, gLerp); // Diffuse material colour in texture RGB (base colour of model)
    float specularMaterialColour = textureColour.a   + textureColour2.a; // Specular material colour in texture A (shininess of the surface)

    // Combine lighting with texture colours
    float3 finalColour = (gAmbientColour + diffuseLight1 + diffuseLight2 + diffuseLight3 + diffuseLight4 + diffuseLight5) * diffuseMaterialColour +
                         (specularLight1 + specularLight2 + specularLight3 + specularLight4 + specularLight5) * specularMaterialColour;

  
    return float4(finalColour, 1.0f); // Always use 1.0f for output alpha - no alpha blending in this lab

}