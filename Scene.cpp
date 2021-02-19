//--------------------------------------------------------------------------------------
// Scene geometry and layout preparation
// Scene rendering & update
//--------------------------------------------------------------------------------------

#include "Scene.h"
#include "Mesh.h"
#include "Model.h"
#include "Camera.h"
#include "State.h"
#include "Shader.h"
#include "Input.h"
#include "Common.h"
#include "CVector2.h" 
#include "CVector3.h" 
#include "CMatrix4x4.h"
#include "MathHelpers.h"     // Helper functions for maths
#include "GraphicsHelpers.h" // Helper functions to unclutter the code
#include "ColourRGBA.h" 
#include <sstream>
#include <memory>

//--------------------------------------------------------------------------------------
// Scene Data
//--------------------------------------------------------------------------------------


// Constants controlling speed of movement/rotation (measured in units per second)

const float ROTATION_SPEED = 2.0f;  // 2 radians per second for rotation
const float MOVEMENT_SPEED = 50.0f; // 50 units per second for movement (what a unit of length is depends on 3D model - i.e. an artist decision usually)

// Enums (used in update scene)
valueChange light1Pulse = UP;
valueChange redColour   = UP;
valueChange greenColour = UP;
valueChange lerpEffect  = UP;

// Meshes
Mesh* gPortalMesh;
Mesh* gLightMesh;
Mesh* gFloorMesh;
Mesh* gTeapotMesh;
Mesh* gSphereMesh;
Mesh* gCubeMesh;
Mesh* gTrollMesh;
Mesh* gRobotMesh;

// Models
Model* gFloor;
Model* gTeapot;
Model* gSphere;
Model* gLight1;
Model* gLight2;
Model* gLight3;
Model* gLight4;
Model* gLight5;
Model* gTwoTextureCube;
Model* gAddBlendcube;
Model* gMultiBlendcube;
Model* gAlphaBlendCube;
Model* gNormalMapCube;
Model* gTroll;
Model* gPortal;
Model* gRobot;

// Cameras
Camera* gCamera;
Camera* gPortalCamera;

// Additional light information

CVector3   gAmbientColour   = { 0.2f, 0.2f, 0.3f }; // Background level of light (slightly bluish to match the far background, which is dark blue)
ColourRGBA gBackgroundColor = { 0.2f, 0.2f, 0.3f , 1.0f };
float      gSpecularPower     = 256;                  // Specular power controls shininess - same for all models in this program

// *****************************************************************************************//

// Light 1 - Yellow Light
CVector3 gLight1Colour   = { 1.0f, 0.8f, 0.2f };
float    gLight1Strength = 10;

float    light1Scale     = 0.0f;

// light 2 - Changing colour Light
CVector3 gLight2Colour   = { 1.0f, 1.0f, 1.0f };
float    gLight2Strength = 10;
float    gLight2Red      = 0.1f;
float    gLight2Green    = 0.0f;
float    gLight2blue     = 0.5f;

// Light 3 - White light
CVector3 gLight3Colour   = { 1.0f, 1.0f, 1.0f };
float    gLight3Strength = 20;

// Light 4 - Normal Mapping Cube light
CVector3 gLight4Colour   = { 0, 0.164f, 0.839f };
float    gLight4Strength = 10;

// Light 5 - Parallax Mapping teapot light
CVector3 gLight5Colour   = { 1.0f, 1.0f, 1.0f };
float    gLight5Strength = 10;

// Blending cube rotation values
float rotateX = 0.0f;
float rotateY = 0.0f;
float rotateZ = 0.0f;

// Cell shading data
CVector3 OutlineColour = { 0, 0, 0 };
float    OutlineThickness = 0.050f;

// Variables controlling Lights orbiting of the models

// Light 1
const float gLight2Orbit = 20.0f;
const float gLight3Orbit = 40.0f;
const float gLight4Orbit = 20.0f;
const float gLight5Orbit = 30.0f;
const float gLight1Multiplier = 10.0f;

// Light 2
const float gLight2OrbitSpeed = 0.7f;
const float gLight3OrbitSpeed = 1.0f;
const float gLight4OrbitSpeed = 1.0f;
const float gLight5OrbitSpeed = 1.0f;

// Other
const int   gLightStrengthModifer = 2;
const int   gLightColorModifier   = 5;
const int   gWiggleMultipler      = 6;
const int   gRotateModifer        = 2;

const float gMinLightStrength      = 1.0f;
const float gMaxLightStrength      = 30.0f;
const float gMinLightColour        = 0.2f;
const float gMaxLightColour        = 0.99f;
const float gLerpMinValue          = 0.0f;
const float gLerpMaxValue          = 1.0f;
const float gPortalRotateMultipler = 0.5f;

//--------------------------------------------------------------------------------------
//**** Portal Texture  ****//
//--------------------------------------------------------------------------------------
// This texture will have the scene renderered on it. Then the texture is applied to a model

// Dimensions of portal texture - controls quality of rendered scene in portal
int gPortalWidth  = 1024;
int gPortalHeight = 1024;

// The portal texture - each frame it is rendered to, then it is used as a texture for model
ID3D11Texture2D*          gPortalTexture      = nullptr; // This object represents the memory used by the texture on the GPU
ID3D11RenderTargetView*   gPortalRenderTarget = nullptr; // This object is used when we want to render to the texture above
ID3D11ShaderResourceView* gPortalTextureSRV   = nullptr; // This object is used to give shaders access to the texture above (SRV = shader resource view)

// Also need a depth/stencil buffer for the portal - it's just another kind of texture
// NOTE: ***Can share this depth buffer between multiple portals of the same size***
ID3D11Texture2D*          gPortalDepthStencil          = nullptr; // This object represents the memory used by the texture on the GPU
ID3D11DepthStencilView*   gPortalDepthStencilView      = nullptr; // This object is used when we want to use the texture above as the depth buffer
ID3D11ShaderResourceView* gPortalDiffuseSpecularMapSRV = nullptr;

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
// Variables sent over to the GPU each frame
// The structures are now in Common.h

PerFrameConstants gPerFrameConstants;      // The constants that need to be sent to the GPU each frame (see common.h for structure)
ID3D11Buffer*     gPerFrameConstantBuffer; // The GPU buffer that will recieve the constants above

PerModelConstants gPerModelConstants;      // As above, but constant that change per-model (e.g. world matrix)
ID3D11Buffer*     gPerModelConstantBuffer; // --"--



//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------

// Light
ID3D11Resource*           gLightDiffuseMap    = nullptr;
ID3D11ShaderResourceView* gLightDiffuseMapSRV = nullptr;

// Floor
ID3D11Resource*           gFloorDiffuseSpecularMap    = nullptr;
ID3D11ShaderResourceView* gFloorDiffuseSpecularMapSRV = nullptr;

// Teapot
ID3D11Resource*           gTeapotDiffuseSpecularMap    = nullptr;
ID3D11ShaderResourceView* gTeapotDiffuseSpecularMapSRV = nullptr;

// Sphere
ID3D11Resource*           gSphereDiffuseSpecularMap    = nullptr;
ID3D11ShaderResourceView* gSphereDiffuseSpecularMapSRV = nullptr;

// Two Texture Cube
ID3D11Resource*           gTwoTextureCubeDiffuseSpecularMap1     = nullptr;
ID3D11ShaderResourceView* gTwoTextureCubeDiffuseSpecularMap1SRV  = nullptr;
ID3D11Resource*           gTwoTextureCubeDiffuseSpecularMap2     = nullptr;
ID3D11ShaderResourceView* gTwoTextureCubeDiffuseSpecularMap2SRV  = nullptr;

// Additive Blending Cube
ID3D11Resource*           gAddBlendCubeDiffuseSpecularMap    = nullptr;
ID3D11ShaderResourceView* gAddBlendCubeDiffuseSpecularMapSRV = nullptr;

// Multiplicative Blending Cube
ID3D11Resource*           gMultiBlendCubeDiffuseSpecularMap    = nullptr;
ID3D11ShaderResourceView* gMultiBlendCubeDiffuseSpecularMapSRV = nullptr;

// Alpha Blending Cube
ID3D11Resource*           gAlphaBlendCubeDiffuseSpecularMap    = nullptr;
ID3D11ShaderResourceView* gAlphaBlendCubeDiffuseSpecularMapSRV = nullptr;

// Normal Mapping Cube
ID3D11Resource*           gNormalMapCubeDiffuseSpecularMap    = nullptr; 
ID3D11ShaderResourceView* gNormalMapCubeDiffuseSpecularMapSRV = nullptr; 
ID3D11Resource*           gNormalMapCubeNormalMap             = nullptr;
ID3D11ShaderResourceView* gNormalMapCubeNormalMapSRV          = nullptr;

// Troll CellShading
ID3D11Resource*           gTrollDiffuseMap    = nullptr; 
ID3D11ShaderResourceView* gTrollDiffuseMapSRV = nullptr; 
ID3D11Resource*           gCellMap            = nullptr;
ID3D11ShaderResourceView* gCellMapSRV         = nullptr;

// Robot
ID3D11Resource*           gRobotDiffuseSpecularMap = nullptr;
ID3D11ShaderResourceView* gRobotDiffuseSpecularMapSRV = nullptr;


//--------------------------------------------------------------------------------------
// Initialise scene geometry, constant buffers and states
//--------------------------------------------------------------------------------------

// Prepare the geometry required for the scene
// Returns true on success
bool InitGeometry()
{
	// Load mesh geometry data.
	try
	{
		gLightMesh  = new Mesh(".\\Media\\Light.x");
		gPortalMesh = new Mesh(".\\Media\\Cube.x");
		gFloorMesh  = new Mesh(".\\Media\\Ground.x");
		gTeapotMesh = new Mesh(".\\Media\\Teapot.x");
		gSphereMesh = new Mesh(".\\Media\\Sphere.x");
		gCubeMesh   = new Mesh(".\\Media\\Cube.x");
		gTrollMesh  = new Mesh(".\\Media\\troll.x");
		gRobotMesh  = new Mesh(".\\Media\\Robot.x");
	}
	catch (std::runtime_error e)  
	{
		gLastError = e.what(); // This picks up the error message put in the exception (see Mesh.cpp)
		return false;
	}


	// Load the shaders required for the geometry we will use (see Shader.cpp / .h)
	if (!LoadShaders())
	{
		gLastError = "Error loading shaders";
		return false;
	}

	// Create GPU-side constant buffers to receive the gPerFrameConstants and gPerModelConstants structures above
	// These allow us to pass data from CPU to shaders such as lighting information or matrices
	// See the comments above where these variable are declared and also the UpdateScene function
	gPerFrameConstantBuffer = CreateConstantBuffer(sizeof(gPerFrameConstants));
	gPerModelConstantBuffer = CreateConstantBuffer(sizeof(gPerModelConstants));
	if (gPerFrameConstantBuffer == nullptr || gPerModelConstantBuffer == nullptr)
	{
		gLastError = "Error creating constant buffers";
		return false;
	}

	//// Load / prepare textures on the GPU ////

	// Load textures and create DirectX objects for them
	// The LoadTexture function requires you to pass a ID3D11Resource* (e.g. &gCubeDiffuseMap), which manages the GPU memory for the
	// texture and also a ID3D11ShaderResourceView* (e.g. &gCubeDiffuseMapSRV), which allows us to use the texture in shaders
	// The function will fill in these pointers with usable data. The variables used here are globals found near the top of the file.
	if (!LoadTexture(".\\Media\\Flare.jpg",                  &gLightDiffuseMap,                   &gLightDiffuseMapSRV)          ||
		!LoadTexture(".\\Media\\WoodDiffuseSpecular.dds",    &gFloorDiffuseSpecularMap,           &gFloorDiffuseSpecularMapSRV)  ||
		!LoadTexture(".\\Media\\BrainDiffuseSpecular.dds",   &gTeapotDiffuseSpecularMap,          &gTeapotDiffuseSpecularMapSRV) ||
		!LoadTexture(".\\Media\\StoneDiffuseSpecular.dds",   &gSphereDiffuseSpecularMap,          &gSphereDiffuseSpecularMapSRV) ||
		!LoadTexture(".\\Media\\brick1.jpg",                 &gTwoTextureCubeDiffuseSpecularMap1, &gTwoTextureCubeDiffuseSpecularMap1SRV)  ||
		!LoadTexture(".\\Media\\tiles1.jpg",                 &gTwoTextureCubeDiffuseSpecularMap2, &gTwoTextureCubeDiffuseSpecularMap2SRV)  ||
		!LoadTexture(".\\Media\\flare.jpg",                  &gAddBlendCubeDiffuseSpecularMap,    &gAddBlendCubeDiffuseSpecularMapSRV) ||
		!LoadTexture(".\\Media\\glass.jpg",                  &gMultiBlendCubeDiffuseSpecularMap,  &gMultiBlendCubeDiffuseSpecularMapSRV) ||
		!LoadTexture(".\\Media\\moogle.png",                 &gAlphaBlendCubeDiffuseSpecularMap,  &gAlphaBlendCubeDiffuseSpecularMapSRV) ||
	    !LoadTexture(".\\Media\\PatternDiffuseSpecular.dds", &gNormalMapCubeDiffuseSpecularMap,   &gNormalMapCubeDiffuseSpecularMapSRV) ||
	    !LoadTexture(".\\Media\\PatternNormal.dds",          &gNormalMapCubeNormalMap,            &gNormalMapCubeNormalMapSRV) ||
		!LoadTexture(".\\Media\\Green.png",                  &gTrollDiffuseMap,                   &gTrollDiffuseMapSRV) ||
		!LoadTexture(".\\Media\\CellGradientBlue.png",       &gCellMap,                           &gCellMapSRV) ||
		!LoadTexture(".\\Media\\tech02.jpg",                 &gRobotDiffuseSpecularMap,           &gRobotDiffuseSpecularMapSRV)
		)
	{
		gLastError = "Error loading textures";
		return false;
	}


	//**** Create Portal Texture ****//

	// Using a helper function to load textures from files above. Here we create the portal texture manually
	// as we are creating a special kind of texture (one that we can render to). Many settings to prepare:
	D3D11_TEXTURE2D_DESC portalDesc = {};
	portalDesc.Width = gPortalWidth;  // Size of the portal texture determines its quality
	portalDesc.Height = gPortalHeight;
	portalDesc.MipLevels = 1; // No mip-maps when rendering to textures (or we would have to render every level)
	portalDesc.ArraySize = 1;
	portalDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // RGBA texture (8-bits each)
	portalDesc.SampleDesc.Count = 1;
	portalDesc.SampleDesc.Quality = 0;
	portalDesc.Usage = D3D11_USAGE_DEFAULT;
	portalDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE; // IMPORTANT: Indicate we will use texture as render target, and pass it to shaders
	portalDesc.CPUAccessFlags = 0;
	portalDesc.MiscFlags = 0;
	if (FAILED(gD3DDevice->CreateTexture2D(&portalDesc, NULL, &gPortalTexture)))
	{
		gLastError = "Error creating portal texture";
		return false;
	}

	// We created the portal texture above, now we get a "view" of it as a render target, i.e. get a special pointer to the texture that
	// we use when rendering to it (see RenderScene function below)
	if (FAILED(gD3DDevice->CreateRenderTargetView(gPortalTexture, NULL, &gPortalRenderTarget)))
	{
		gLastError = "Error creating portal render target view";
		return false;
	}

	// We also need to send this texture (resource) to the shaders. To do that we must create a shader-resource "view"
	D3D11_SHADER_RESOURCE_VIEW_DESC srDesc = {};
	srDesc.Format = portalDesc.Format;
	srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srDesc.Texture2D.MostDetailedMip = 0;
	srDesc.Texture2D.MipLevels = 1;
	if (FAILED(gD3DDevice->CreateShaderResourceView(gPortalTexture, &srDesc, &gPortalTextureSRV)))
	{
		gLastError = "Error creating portal shader resource view";
		return false;
	}


	//**** Create Portal Depth Buffer ****//

	// We also need a depth buffer to go with our portal
	//**** This depth buffer can be shared with any other portals of the same size
	portalDesc = {};
	portalDesc.Width = gPortalWidth;
	portalDesc.Height = gPortalHeight;
	portalDesc.MipLevels = 1;
	portalDesc.ArraySize = 1;
	portalDesc.Format = DXGI_FORMAT_D32_FLOAT; // Depth buffers contain a single float per pixel
	portalDesc.SampleDesc.Count = 1;
	portalDesc.SampleDesc.Quality = 0;
	portalDesc.Usage = D3D11_USAGE_DEFAULT;
	portalDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
	portalDesc.CPUAccessFlags = 0;
	portalDesc.MiscFlags = 0;
	if (FAILED(gD3DDevice->CreateTexture2D(&portalDesc, NULL, &gPortalDepthStencil)))
	{
		gLastError = "Error creating portal depth stencil texture";
		return false;
	}

	// Create the depth stencil view, i.e. indicate that the texture just created is to be used as a depth buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC portalDescDSV = {};
	portalDescDSV.Format = portalDesc.Format;
	portalDescDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	portalDescDSV.Texture2D.MipSlice = 0;
	portalDescDSV.Flags = 0;
	if (FAILED(gD3DDevice->CreateDepthStencilView(gPortalDepthStencil, &portalDescDSV, &gPortalDepthStencilView)))
	{
		gLastError = "Error creating portal depth stencil view";
		return false;
	}

	// Create all filtering modes, blending modes etc. used by the app (see State.cpp/.h)
	if (!CreateStates())
	{
		gLastError = "Error creating states";
		return false;
	}

	return true;
}


// Prepare the scene
// Returns true on success
bool InitScene()
{
	//// Set up scene ////

	gFloor          = new Model(gFloorMesh);
	gTeapot         = new Model(gTeapotMesh);
	gSphere         = new Model(gSphereMesh);
	gLight1         = new Model(gLightMesh);
	gLight2         = new Model(gLightMesh);
	gLight3         = new Model(gLightMesh);
	gLight4         = new Model(gLightMesh);
	gLight5         = new Model(gLightMesh);
	gTwoTextureCube = new Model(gCubeMesh);
	gAddBlendcube   = new Model(gCubeMesh);
	gMultiBlendcube = new Model(gCubeMesh);
	gAlphaBlendCube = new Model(gCubeMesh);
	gNormalMapCube  = new Model(gCubeMesh);
	gTroll          = new Model(gTrollMesh);
	gPortal         = new Model(gPortalMesh);
	gRobot          = new Model(gRobotMesh);

	// Initial positions

	// Teapot
	gTeapot->SetPosition({ -10, 0, 0 });

	// Sphere
	gSphere->SetPosition({ 15, 15, 50 });

	// Two Texture Cube
	gTwoTextureCube->SetPosition({ 30,5,-20 });

	// Addative Blending Cube
	gAddBlendcube->SetPosition({ 100,15,-40 });

	// Multiplicative Blending Cube
	gMultiBlendcube->SetPosition({ 100,15,-60 });

	// Alpha Blending Club
	gAlphaBlendCube->SetPosition({ 100,15,-80 });

	// Normal Mapping Cube
	gNormalMapCube->SetPosition({ 30,20,-100 });

	// Troll
	gTroll->SetPosition({ -70, 10,-120 });
	gTroll->SetRotation({ ToRadians(0.0f), ToRadians(60.0f), 0.0f });
	gTroll->SetScale(10.0f);

	// Moving Portal
	gPortal->SetScale({ 3.0f, 3.0f, 0.1f });
	gPortal->SetPosition({ -70, 30, -100 });

	// Robot
	gRobot->SetPosition({ -50, 0,-40 });
	gRobot->SetScale(4.0f);
	gRobot->SetRotation({ ToRadians(0.0f), ToRadians(110.0f), 0.0f });

	// Lights

	gLight1->SetPosition({ 40, 20, 0 });
	gLight1->SetScale(pow(gLight1Strength, 1.5f));

	gLight2->SetPosition({ -10, 10, 0 });
	gLight2->SetScale(pow(gLight2Strength, 1.0f));

	gLight3->SetPosition({ 150,50,-60 });
	gLight3->SetScale(pow(gLight3Strength, 1.0f));
	
	gLight4->SetPosition({ 30,30,-200 });
	gLight4->SetScale(pow(gLight4Strength, 1.0f));

	gLight5->SetPosition({ 150,20,-250 });
	gLight5->SetScale(pow(gLight5Strength, 1.0f));

	//// Set up cameras ////

	gCamera = new Camera();
	gCamera->SetPosition({ 15, 45, -75 });
	gCamera->SetRotation({ ToRadians(30.0f), ToRadians(0.0f), 0.0f });
	gCamera->SetNearClip(0.1f);
	gCamera->SetFarClip(100000.0f);

	// Set up portal camera //

	gPortalCamera = new Camera();
	gPortalCamera->SetPosition({ 45, 45, 85 });
	gPortalCamera->SetRotation({ ToRadians(20.0f), ToRadians(215.0f), 0 });

	return true;
}


// Release the geometry and scene resources created above
void ReleaseResources()
{
	ReleaseStates();

	if (gPortalDepthStencilView)               gPortalDepthStencilView->Release();
	if (gPortalDepthStencil)                   gPortalDepthStencil->Release();
	if (gPortalTextureSRV)                     gPortalTextureSRV->Release();
	if (gPortalRenderTarget)                   gPortalRenderTarget->Release();
	if (gPortalTexture)                        gPortalTexture->Release();
	if (gLightDiffuseMapSRV)                   gLightDiffuseMapSRV->Release();
	if (gLightDiffuseMap)                      gLightDiffuseMap->Release();
	if (gFloorDiffuseSpecularMapSRV)           gFloorDiffuseSpecularMapSRV->Release();
	if (gFloorDiffuseSpecularMap)              gFloorDiffuseSpecularMap->Release();
	if (gTeapotDiffuseSpecularMapSRV)          gTeapotDiffuseSpecularMapSRV->Release();
	if (gTeapotDiffuseSpecularMap)             gTeapotDiffuseSpecularMap->Release();
	if (gSphereDiffuseSpecularMapSRV)          gSphereDiffuseSpecularMapSRV->Release();
	if (gSphereDiffuseSpecularMap)             gSphereDiffuseSpecularMap->Release();
	if (gTwoTextureCubeDiffuseSpecularMap1SRV) gTwoTextureCubeDiffuseSpecularMap1SRV->Release();
	if (gTwoTextureCubeDiffuseSpecularMap1)    gTwoTextureCubeDiffuseSpecularMap1->Release();
	if (gAddBlendCubeDiffuseSpecularMapSRV)    gAddBlendCubeDiffuseSpecularMapSRV->Release();
	if (gAddBlendCubeDiffuseSpecularMap)       gAddBlendCubeDiffuseSpecularMap->Release();
	if (gMultiBlendCubeDiffuseSpecularMapSRV)  gMultiBlendCubeDiffuseSpecularMapSRV->Release();
	if (gMultiBlendCubeDiffuseSpecularMap)     gMultiBlendCubeDiffuseSpecularMap->Release();
	if (gAlphaBlendCubeDiffuseSpecularMapSRV)  gAlphaBlendCubeDiffuseSpecularMapSRV->Release();
	if (gAlphaBlendCubeDiffuseSpecularMap)     gAlphaBlendCubeDiffuseSpecularMap->Release();
	if (gNormalMapCubeDiffuseSpecularMapSRV)   gNormalMapCubeDiffuseSpecularMapSRV->Release();
	if (gNormalMapCubeDiffuseSpecularMap)      gNormalMapCubeDiffuseSpecularMap->Release();
	if (gNormalMapCubeNormalMapSRV)            gNormalMapCubeNormalMapSRV->Release();
	if (gNormalMapCubeNormalMap)               gNormalMapCubeNormalMap->Release();
	if (gTrollDiffuseMapSRV)                   gTrollDiffuseMapSRV->Release();
	if (gTrollDiffuseMap)                      gTrollDiffuseMap->Release();
	if (gCellMapSRV)                           gCellMapSRV->Release();
	if (gCellMap)                              gCellMap->Release();
	if (gRobotDiffuseSpecularMap)              gRobotDiffuseSpecularMap->Release();
	if (gRobotDiffuseSpecularMapSRV)           gRobotDiffuseSpecularMapSRV->Release();


	if (gPerModelConstantBuffer)               gPerModelConstantBuffer->Release();
	if (gPerFrameConstantBuffer)               gPerFrameConstantBuffer->Release();

	ReleaseShaders();

	delete gCamera;         gCamera         = nullptr;
	delete gPortalCamera;   gPortalCamera   = nullptr;
	delete gPortalMesh;     gPortalMesh     = nullptr;
	delete gLightMesh;      gLightMesh      = nullptr;
	delete gTroll;          gTroll          = nullptr;
	delete gFloorMesh;      gFloorMesh      = nullptr;
	delete gTeapotMesh;     gTeapotMesh     = nullptr;
	delete gSphereMesh;     gSphereMesh     = nullptr;
	delete gCubeMesh;       gCubeMesh       = nullptr;
	delete gTrollMesh;      gTrollMesh      = nullptr;
	delete gFloor;          gFloor          = nullptr;
	delete gTeapot;         gTeapot         = nullptr;
	delete gSphere;         gSphere         = nullptr;
	delete gTwoTextureCube; gTwoTextureCube = nullptr;
	delete gAddBlendcube;   gAddBlendcube   = nullptr;
	delete gMultiBlendcube; gMultiBlendcube = nullptr;
	delete gAlphaBlendCube; gAlphaBlendCube = nullptr;
	delete gNormalMapCube;  gNormalMapCube  = nullptr;
	delete gPortal;         gPortal         = nullptr;
	delete gLight1;         gLight1         = nullptr;
	delete gLight2;         gLight2         = nullptr;
	delete gLight3;         gLight3         = nullptr;
	delete gLight4;         gLight4         = nullptr;
	delete gLight5;         gLight5         = nullptr;
	delete gRobot;          gRobot          = nullptr;

}

//--------------------------------------------------------------------------------------
// Scene Rendering
//--------------------------------------------------------------------------------------

// Render everything in the scene from the given camera
// This code is common between rendering the main scene and rendering the scene in the portal
// See RenderScene function below
void RenderSceneFromCamera(Camera* camera)
{
	// Set camera matrices in the constant buffer and send over to GPU
	gPerFrameConstants.viewMatrix           = camera->ViewMatrix();
	gPerFrameConstants.projectionMatrix     = camera->ProjectionMatrix();
	gPerFrameConstants.viewProjectionMatrix = camera->ViewProjectionMatrix();
	gPerFrameConstants.outlineColour        = OutlineColour;
	gPerFrameConstants.outlineThickness     = OutlineThickness;
	UpdateConstantBuffer(gPerFrameConstantBuffer, gPerFrameConstants);

	// Indicate that the constant buffer we just updated is for use in the vertex shader (VS) and pixel shader (PS)
	gD3DContext->VSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer); // First parameter must match constant buffer number in the shader 
	gD3DContext->PSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer);

	//// Render lit models ////

	// RENDER GROUND //

	// Select which shaders to use next
	gD3DContext->VSSetShader(gPixelLightingVertexShader, nullptr, 0);
	gD3DContext->PSSetShader(gPixelLightingPixelShader, nullptr, 0);

	// States - no blending, normal depth buffer and culling
	gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
	gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
	gD3DContext->RSSetState(gCullBackState);

	// Select the approriate textures and sampler to use in the pixel shader
	gD3DContext->PSSetShaderResources(0, 1, &gFloorDiffuseSpecularMapSRV); // First parameter must match texture slot number in the shader
	gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);

	// Render model - it will update the model's world matrix and send it to the GPU in a constant buffer, then it will call
	// the Mesh render function, which will set up vertex & index buffer before finally calling Draw on the GPU
	gFloor->Render();

	// RENDER TEAPOT //

	gD3DContext->PSSetShaderResources(0, 1, &gTeapotDiffuseSpecularMapSRV);
	gTeapot->Render();

	// RENDER ADDITAVE BLENDING CUBE //
	gD3DContext->PSSetShaderResources(0, 1, &gAddBlendCubeDiffuseSpecularMapSRV);
	gD3DContext->RSSetState(gCullNoneState);
	gD3DContext->OMSetBlendState(gAdditiveBlendingState, nullptr, 0xffffff);
	gAddBlendcube->Render();

	// RENDER MULTIPLICATIVE BLENDING CUBE //
	gD3DContext->PSSetShaderResources(0, 1, &gMultiBlendCubeDiffuseSpecularMapSRV);
	gD3DContext->OMSetBlendState(gMultiplicativeBlendingState, nullptr, 0xffffff);
	gMultiBlendcube->Render();

	// RENDER ALPHA BLENDING CUBE //
	gD3DContext->VSSetShader(gAlphaVertexShader, nullptr, 0);
	gD3DContext->PSSetShader(gAlphaPixelShader, nullptr, 0);
	gD3DContext->PSSetShaderResources(0, 1, &gAlphaBlendCubeDiffuseSpecularMapSRV);
	gD3DContext->OMSetBlendState(gAlphaBlendingState, nullptr, 0xffffff);
	gAlphaBlendCube->Render();

	// RENDER NORMAL MAPPING CUBE
	gD3DContext->VSSetShader(gNormalMappingVertexShader, nullptr, 0);
	gD3DContext->PSSetShader(gNormalMappingPixelShader, nullptr, 0);
	gD3DContext->PSSetShaderResources(0, 1, &gNormalMapCubeDiffuseSpecularMapSRV); 
	gD3DContext->PSSetShaderResources(1, 1, &gNormalMapCubeNormalMapSRV);
	gNormalMapCube->Render();

	// RENDER CELL SHADING TROLL - FIRST PASS //

	// Draw models inside out, slightly bigger and black

	gD3DContext->VSSetShader(gCellShadingOutlineVertexShader, nullptr, 0);
	gD3DContext->PSSetShader(gCellShadingOutlinePixelShader, nullptr, 0);

	// States - no blending, normal depth buffer. However, use front culling to draw *inside* of model
	gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
	gD3DContext->RSSetState(gCullFrontState);

	// No textures needed, draws outline in plain colour
	// Render models, no GPU changes needed between rendering them in this case
	gTroll->Render();

	// RENDER CELL SHADING TROLL - SECOND PASS //

	// Main cell shading shaders
	gD3DContext->VSSetShader(gCellShadingVertexShader, nullptr, 0);
	gD3DContext->PSSetShader(gCellShadingPixelShader, nullptr, 0);

	// Switch back to the usual back face culling (not inside out)
	gD3DContext->RSSetState(gCullBackState);

	// Select the troll texture and sampler
	gD3DContext->PSSetShaderResources(0, 1, &gTrollDiffuseMapSRV); // First parameter must match texture slot number in the shaer
	gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);

	// Also, cell shading uses a special 1D "cell map", which uses point sampling
	gD3DContext->PSSetShaderResources(1, 1, &gCellMapSRV); // First parameter must match texture slot number in the shaer
	gD3DContext->PSSetSamplers(1, 1, &gPointSampler);

	// Render troll model
	gTroll->Render();

	// RENDER SPHERE //

	gD3DContext->VSSetShader(gWiggleModelVertexShader, nullptr, 0);
	gD3DContext->PSSetShader(gWiggleModelPixelShader, nullptr, 0);
	gD3DContext->PSSetShaderResources(0, 1, &gSphereDiffuseSpecularMapSRV);
	gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);

	gSphere->Render();

	// RENDER TWO TEXTURE CUBE //

	gD3DContext->VSSetShader(gFadeTwoTexturesVertexShader, nullptr, 0);
	gD3DContext->PSSetShader(gFadeTwoTexturesPixelShader, nullptr, 0);
	gD3DContext->PSSetShaderResources(0, 1, &gTwoTextureCubeDiffuseSpecularMap1SRV);
	gD3DContext->PSSetShaderResources(1, 1, &gTwoTextureCubeDiffuseSpecularMap2SRV);
	gTwoTextureCube->Render();

	// RENDER LIGHTS //

	gD3DContext->VSSetShader(gLightModelVertexShader, nullptr, 0);
	gD3DContext->PSSetShader(gLightModelPixelShader, nullptr, 0);
	gD3DContext->PSSetShaderResources(0, 1, &gLightDiffuseMapSRV); // First parameter must match texture slot number in the shaer
	gD3DContext->OMSetBlendState(gAdditiveBlendingState, nullptr, 0xffffff);
	gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);
	gD3DContext->RSSetState(gCullNoneState);

	// The shaders, texture and states are the same, so no need to set them again to draw the second light

	gPerModelConstants.objectColour = gLight1Colour;
	gLight1->Render();
	gPerModelConstants.objectColour = gLight2Colour;
	gLight2->Render();
	gPerModelConstants.objectColour = gLight3Colour;
	gLight3->Render();
	gPerModelConstants.objectColour = gLight4Colour;
	gLight4->Render();
	gPerModelConstants.objectColour = gLight5Colour;
	gLight5->Render();

	// Render Portal
	gD3DContext->VSSetShader(gPixelLightingVertexShader, nullptr, 0);
	gD3DContext->PSSetShader(gPixelLightingPixelShader, nullptr, 0);
	gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
	gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
	gD3DContext->RSSetState(gCullBackState);

	gD3DContext->PSSetShaderResources(0, 1, &gPortalTextureSRV);
	gPortal->Render();

	// Robot
	gD3DContext->VSSetShader(gPixelLightingVertexShader, nullptr, 0);
	gD3DContext->PSSetShader(gPixelLightingPixelShader, nullptr, 0);
	gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
	gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
	gD3DContext->RSSetState(gCullBackState);
	gRobot->Render();


}


// Rendering the scene now renders everything twice. First it renders the scene for the portal into a texture.
// Then it renders the main scene using the portal texture on a model.
void RenderScene()
{
	//// Common settings for both main scene and portal scene ////

	// Set up the light information in the constant buffer
	// Don't send to the GPU yet, the function RenderSceneFromCamera will do that
	gPerFrameConstants.light1Colour   = gLight1Colour * gLight1Strength;
	gPerFrameConstants.light1Position = gLight1->Position();
	gPerFrameConstants.light2Colour   = gLight2Colour * gLight2Strength;
	gPerFrameConstants.light2Position = gLight2->Position();
	gPerFrameConstants.light3Colour   = gLight3Colour * gLight3Strength;
	gPerFrameConstants.light3Position = gLight3->Position();
	gPerFrameConstants.light4Colour   = gLight4Colour * gLight4Strength;
	gPerFrameConstants.light4Position = gLight4->Position();
	gPerFrameConstants.light5Colour   = gLight5Colour * gLight5Strength;
	gPerFrameConstants.light5Position = gLight5->Position();
	gPerFrameConstants.ambientColour  = gAmbientColour;
	gPerFrameConstants.specularPower  = gSpecularPower;
	gPerFrameConstants.cameraPosition = gCamera->Position();

	//// Portal scene rendering ////

	// Set the portal texture and portal depth buffer as the targets for rendering
	// The portal texture will later be used on models in the main scene
	gD3DContext->OMSetRenderTargets(1, &gPortalRenderTarget, gPortalDepthStencilView);

	// Clear the portal texture to a fixed colour and the portal depth buffer to the far distance
	gD3DContext->ClearRenderTargetView(gPortalRenderTarget, &gBackgroundColor.r);
	gD3DContext->ClearDepthStencilView(gPortalDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Setup the viewport for the portal texture size
	D3D11_VIEWPORT vp;
	vp.Width = static_cast<FLOAT>(gPortalWidth);
	vp.Height = static_cast<FLOAT>(gPortalHeight);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	gD3DContext->RSSetViewports(1, &vp);

	// Render the scene for the portal
	RenderSceneFromCamera(gPortalCamera);


	//// Main scene rendering ////

	// Now set the back buffer as the target for rendering and select the main depth buffer.
	// When finished the back buffer is sent to the "front buffer" - which is the monitor.
	gD3DContext->OMSetRenderTargets(1, &gBackBufferRenderTarget, gDepthStencil);

	// Clear the back buffer to a fixed colour and the depth buffer to the far distance
	gD3DContext->ClearRenderTargetView(gBackBufferRenderTarget, &gBackgroundColor.r);
	gD3DContext->ClearDepthStencilView(gDepthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Setup the viewport to the size of the main window
	vp.Width = static_cast<FLOAT>(gViewportWidth);
	vp.Height = static_cast<FLOAT>(gViewportHeight);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	gD3DContext->RSSetViewports(1, &vp);

	// Render the scene for the main window
	RenderSceneFromCamera(gCamera);

	//// Scene completion ////

	// When drawing to the off-screen back buffer is complete, we "present" the image to the front buffer (the screen)
	gSwapChain->Present(0, 0);
}


//--------------------------------------------------------------------------------------
// Scene Update
//--------------------------------------------------------------------------------------

// Update models and camera. frameTime is the time passed since the last frame
void UpdateScene(float frameTime)
{
	// Pulsate light One
	switch (light1Pulse)
	{
	case UP:
	{
		gLight1Strength += (frameTime * gLight1Multiplier);
		gLight1->SetScale(gLight1Strength / gLightStrengthModifer);
		if (gLight1Strength > gMaxLightStrength) light1Pulse = DOWN;
		break;
	}
	case DOWN:
	{
		gLight1Strength -= (frameTime * gLight1Multiplier);
		gLight1->SetScale(gLight1Strength / gLightStrengthModifer);
		if (gLight1Strength < gMinLightStrength) light1Pulse = UP;
		break;
	}
	}

	// Change colour of light 2
	switch (redColour)
	{
	case UP:
	{
		gLight2Red += (frameTime / gLightColorModifier);
		if (gLight2Red > gMaxLightColour) redColour = DOWN;
		break;
	}
	case DOWN:
	{
		gLight2Red -= (frameTime / gLightColorModifier);
		if (gLight2Red < gMinLightColour) redColour = UP;
		break;
	}
	}

	switch (greenColour)
	{
	case UP:
	{
		gLight2Green += (frameTime / gLightColorModifier);
		if (gLight2Green > gMaxLightColour) greenColour = DOWN;
		break;
	}
	case DOWN:
	{
		gLight2Green -= (frameTime / gLightColorModifier);
		if (gLight2Green < gMinLightColour) greenColour = UP;
		break;
	}
	}
	gLight2Colour = { gLight2Red, 0.5f, 1.0f };

	// Orbit Light2
	static float rotate = 0.0f;
	gLight2->SetPosition(gTeapot->Position() + CVector3{ cos(rotate) * gLight2Orbit, 10.0f, sin(rotate) * gLight2Orbit });
	rotate -= gLight2OrbitSpeed * frameTime;

	// Wiggle effect - Used on sphere
	gPerModelConstants.wiggle += gWiggleMultipler * frameTime;
	gPerModelConstants.rotation += frameTime;

	// Lerp effect - used with cube to change between two skins
	switch (lerpEffect)
	{
	case UP:
	{
		gPerModelConstants.lerp += frameTime;
		if (gPerModelConstants.lerp > gLerpMaxValue) lerpEffect = DOWN;
		break;
	}
	case DOWN:
	{
		gPerModelConstants.lerp -= frameTime;
		if (gPerModelConstants.lerp < gLerpMinValue) lerpEffect = UP;
		break;
	}
	}

	// Rotate blending cubes
	rotateX += frameTime / gRotateModifer;
	rotateY += frameTime / gRotateModifer;
	rotateZ += frameTime / gRotateModifer;

	gAddBlendcube->SetRotation   (CVector3{ 0.0f, rotateY, rotateZ });
	gMultiBlendcube->SetRotation (CVector3{ rotateX, 0.0f, rotateZ });
	gAlphaBlendCube->SetRotation (CVector3{ rotateX, rotateY, 0.0f });

	// Orbit Light3
	static float rotate2 = 0.0f;
	gLight3->SetPosition(gMultiBlendcube->Position() + CVector3{ cos(rotate2) * gLight3Orbit, 5.0f, sin(rotate2) * gLight3Orbit });
	rotate2 -= gLight3OrbitSpeed * frameTime;

	// Orbit Light4
	static float rotate3 = 0.0f;
	gLight4->SetPosition(gNormalMapCube->Position() + CVector3{ cos(rotate3) * gLight4Orbit, .0f, sin(rotate3) * gLight4Orbit });
	rotate3 -= gLight4OrbitSpeed * frameTime;

	// Orbit Light5
	static float rotate4 = 0.0f;
	gLight5->SetPosition(gTroll->Position() + CVector3{ cos(rotate4) * gLight5Orbit, 5.0f, sin(rotate4) * gLight5Orbit });
	rotate4 -= gLight5OrbitSpeed * frameTime;

	// Move portal
	static float rotate5 = 0.0f;
	gPortal->SetPosition(gTwoTextureCube->Position() + CVector3{ cos(rotate5) * 80.0f, 40.0f, sin(rotate5) * 80.0f });
	gPortal->FaceTarget(gTwoTextureCube->Position()); // Face target function. Makes the portal face the cube at all times while moving
	gPortalCamera->SetPosition(gPortal->Position());
	gPortalCamera->FaceTarget(gTwoTextureCube->Position());
	rotate5 -= gPortalRotateMultipler * frameTime;

	// Control camera (will update its view matrix)
	gCamera->Control(frameTime, Key_Up, Key_Down, Key_Left, Key_Right, Key_W, Key_S, Key_A, Key_D);

	// Show frame time / FPS in the window title //
	const float fpsUpdateTime = 0.5f; // How long between updates (in seconds)
	static float totalFrameTime = 0;
	static int frameCount = 0;
	totalFrameTime += frameTime;
	++frameCount;
	if (totalFrameTime > fpsUpdateTime)
	{
		// Displays FPS rounded to nearest int, and frame time (more useful for developers) in milliseconds to 2 decimal places
		float avgFrameTime = totalFrameTime / frameCount;
		std::ostringstream frameTimeMs;
		frameTimeMs.precision(2);
		frameTimeMs << std::fixed << avgFrameTime * 1000;
		std::string windowTitle = "CO2409 Assignment 1: Shaders - Mark Ince - Frame Time: " + frameTimeMs.str() +
			"ms, FPS: " + std::to_string(static_cast<int>(1 / avgFrameTime + 0.5f));
		SetWindowTextA(gHWnd, windowTitle.c_str());
		totalFrameTime = 0;
		frameCount = 0;
	}
}
