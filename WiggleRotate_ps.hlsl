#include "Common.hlsli"

Texture2D DiffuseSpecularMap : register(t0);
SamplerState TexSampler : register(s0);

float4 main(WigglePixelShaderInput input) : SV_Target
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

    input.uv.x += gRotation;

    // Sample diffuse material and specular material colour for this pixel from a texture using a given sampler that you set up in the C++ code
    float4 textureColour = DiffuseSpecularMap.Sample(TexSampler, input.uv);
    float3 diffuseMaterialColour = textureColour.rgb; // Diffuse material colour in texture RGB (base colour of model)
    float specularMaterialColour = textureColour.a; // Specular material colour in texture A (shininess of the surface)

    // Combine lighting with texture colours
    float3 finalColour = (gAmbientColour + diffuseLight1 + diffuseLight2 + diffuseLight3 + diffuseLight4 + diffuseLight5) * diffuseMaterialColour +
                         (specularLight1 + specularLight2 + specularLight3 + specularLight4 + specularLight5) * specularMaterialColour;

    finalColour.rg = finalColour.rg + 0.1f;
    finalColour.b = finalColour.b + 0.6f;


    return float4(finalColour, 1.0f); // Always use 1.0f for output alpha - no alpha blending in this lab

}