//--------------------------------------------------------------------------------------
// Texture Pixel Shader
//--------------------------------------------------------------------------------------
// Pixel shader simply samples a diffuse texture map and tints with colours from vertex shadeer

#include "Common.hlsli" // Shaders can also use include files - note the extension


//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// Here we allow the shader access to a texture that has been loaded from the C++ side and stored in GPU memory.
// Note that textures are often called maps (because texture mapping describes wrapping a texture round a mesh).
// Get used to people using the word "texture" and "map" interchangably.
Texture2D DiffuseSpecularMap : register(t0); // Diffuse map (main colour) in rgb and specular map (shininess level) in alpha - C++ must load this into slot 0
Texture2D NormalMap : register(t1); // Normal map in rgb - C++ must load this into slot 1
SamplerState TexSampler : register(s0); // A sampler is a filter for a texture like bilinear, trilinear or anisotropic


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

//***| INFO |*********************************************************************************
// Normal mapping pixel shader function. The lighting part of the shader is the same as the
// per-pixel lighting shader - only the source of the surface normal is different
//
// An extra "Normal Map" texture is used - this contains normal (x,y,z) data in place of
// (r,g,b) data indicating the normal of the surface *per-texel*. This allows the lighting
// to take account of bumps on the texture surface. Using these normals is complex:
//    1. We must store a "tangent" vector as well as a normal for each vertex (the tangent
//       is basically the direction of the texture U axis in model space for each vertex)
//    2. Get the (interpolated) model normal and tangent at this pixel from the vertex
//       shader - these are the X and Z axes of "tangent space"
//    3. Use a "cross-product" to calculate the bi-tangent - the missing Y axis
//    4. Form the "tangent matrix" by combining these axes
//    5. Extract the normal from the normal map texture for this pixel
//    6. Use the tangent matrix to transform the texture normal into model space, then
//       use the world matrix to transform it into world space
//    7. This final world-space normal can be used in the usual lighting calculations, and
//       will show the "bumpiness" of the normal map
//
// Note that all this detail boils down to just five extra lines of code here
//********************************************************************************************
float4 main(NormalMappingPixelShaderInput input) : SV_Target
{
	//************************
	// Normal Map Extraction
	//************************

	// Will use the model normal/tangent to calculate matrix for tangent space. The normals for each pixel are *interpolated* from the
	// vertex normals/tangents. This means they will not be length 1, so they need to be renormalised (same as per-pixel lighting issue)
    float3 modelNormal = normalize(input.modelNormal);
    float3 modelTangent = normalize(input.modelTangent);

	// Calculate bi-tangent to complete the three axes of tangent space - then create the *inverse* tangent matrix to convert *from*
	// tangent space into model space. This is just a matrix built from the three axes (very advanced note - by default shader matrices
	// are stored as columns rather than in rows as in the C++. This means that this matrix is created "transposed" from what we would
	// expect. However, for a 3x3 rotation matrix the transpose is equal to the inverse, which is just what we require)
    float3 modelBiTangent = cross(modelNormal, modelTangent);
    float3x3 invTangentMatrix = float3x3(modelTangent, modelBiTangent, modelNormal);
	
	// Get the texture normal from the normal map. The r,g,b pixel values actually store x,y,z components of a normal. However, r,g,b
	// values are stored in the range 0->1, whereas the x, y & z components should be in the range -1->1. So some scaling is needed
    float3 textureNormal = 2.0f * NormalMap.Sample(TexSampler, input.modelTangent.xy).rgb - 1.0f; // Scale from 0->1 to -1->1

  
    // Can change the following calculation to exaggerate the bumpiness on the cube
    textureNormal.z = textureNormal.z - 0.9f;


	// Now convert the texture normal into model space using the inverse tangent matrix, and then convert into world space using the world
	// matrix. Normalise, because of the effects of texture filtering and in case the world matrix contains scaling
    float3 worldNormal = normalize(mul((float3x3) gWorldMatrix, mul(textureNormal, invTangentMatrix)));


	///////////////////////
	// Calculate lighting

   // Lighting equations
    float3 cameraDirection = normalize(gCameraPosition - input.worldPosition);

    // Light 1
    float3 diffuseLight1 = CalculateDiffuseLight(gLight1Position, input.worldPosition, gLight1Colour, worldNormal);
    float3 specularLight1 = CalculateSpecularLight(gLight1Position, input.worldPosition, worldNormal, cameraDirection, gSpecularPower, diffuseLight1);

    // Light 2
    float3 diffuseLight2 = CalculateDiffuseLight(gLight2Position, input.worldPosition, gLight2Colour, worldNormal);
    float3 specularLight2 = CalculateSpecularLight(gLight2Position, input.worldPosition, worldNormal, cameraDirection, gSpecularPower, diffuseLight2);

    // Light 3
    float3 diffuseLight3 = CalculateDiffuseLight(gLight3Position, input.worldPosition, gLight3Colour, worldNormal);
    float3 specularLight3 = CalculateSpecularLight(gLight3Position, input.worldPosition, worldNormal, cameraDirection, gSpecularPower, diffuseLight3);

    // Light 4
    float3 diffuseLight4 = CalculateDiffuseLight(gLight4Position, input.worldPosition, gLight4Colour, worldNormal);
    float3 specularLight4 = CalculateSpecularLight(gLight4Position, input.worldPosition, worldNormal, cameraDirection, gSpecularPower, diffuseLight4);

    // Light 5
    float3 diffuseLight5 = CalculateDiffuseLight(gLight5Position, input.worldPosition, gLight5Colour, worldNormal);
    float3 specularLight5 = CalculateSpecularLight(gLight5Position, input.worldPosition, worldNormal, cameraDirection, gSpecularPower, diffuseLight5);


    // Sample diffuse material colour for this pixel from a texture using a given sampler that you set up in the C++ code
    // Ignoring any alpha in the texture, just reading RGB
    float4 textureColour = DiffuseSpecularMap.Sample(TexSampler, input.modelTangent);
    float3 diffuseMaterialColour = textureColour.rgb;
    float specularMaterialColour = textureColour.a;

    float3 finalColour = (gAmbientColour + diffuseLight1 + diffuseLight2 + diffuseLight3 + diffuseLight4 + diffuseLight5) * diffuseMaterialColour +
                         (specularLight1 + specularLight2 + specularLight3 + specularLight4 + specularLight5) * specularMaterialColour;

    return float4(finalColour, 1.0f); // Always use 1.0f for alpha - no alpha blending in this lab
}