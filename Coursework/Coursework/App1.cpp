#include "App1.h"

XMFLOAT3 params = XMFLOAT3(0, 0, 0);

// Screen-Related Variables
int screenWidthVar, screenHeightVar;  // Holds the width and height of the screen for rendering
float aspectRatio;  // Stores the aspect ratio of the screen for correct projection
float fieldOfView = XM_PI / 4;  // Field of view for the camera, set to 45 degrees by default

// Vertex Manipulation
float heightMapStrength = 30.f;  // Controls how much the height map affects the terrain displacement

// Light Properties
XMFLOAT4 lightType[lightSize];  // Stores the type of each light (e.g., directional, point light)
XMFLOAT4 ambient[lightSize];  // Ambient light color for each light source
XMFLOAT4 diffuse[lightSize];  // Diffuse light color for each light source
XMFLOAT4 direction[lightSize];  // Direction of the light (used for directional lights)
XMFLOAT4 position[lightSize];  // Position of the light (used for point lights)
XMFLOAT4 specularColour[lightSize];  // Specular light color for each light
XMFLOAT4 specularPower[lightSize];  // Power of the specular highlights
float intensity[lightSize];  // Light intensity multiplier
XMFLOAT3 pivot = XMFLOAT3(10, 10, 10);  // Pivot point used for light positioning
XMFLOAT3 radius = XMFLOAT3(70, 70, 70);  // Radius of light's movement

// Positions and Transformations
XMFLOAT3 camPos = XMFLOAT3();  // Camera position in world space
XMFLOAT3 mountainPeak = XMFLOAT3(25, 13.95, 25);  // Position of the mountain peak in the scene
XMFLOAT3 spherePos = XMFLOAT3(25.f, 12.f, 28.f);  // Position of the sphere object
XMFLOAT3 sphereScale = XMFLOAT3(.5, .5, .5);  // Scaling factor for the sphere
XMFLOAT3 cottagePosition = XMFLOAT3(mountainPeak.x + 5, mountainPeak.y - 2.3, mountainPeak.z);  // Position of the cottage
XMFLOAT3 spotlightModelPosition = XMFLOAT3(mountainPeak.x, mountainPeak.y - 1.7f, mountainPeak.z + 1.f);  // Position of the spotlight model
XMFLOAT3 sceneCentre = XMFLOAT3(25, 25, 25);  // The center of the scene, used for light positioning

// Time-Related Variables
float elapsedTime = 0;  // Tracks the total time elapsed in the scene
float timeFloat = 0;  // Used for calculating and tracking the time (likely for day-night cycles)
float dayDuration;  // Duration of one full day cycle in the scene
float timeScale = 1.f;  // A multiplier for adjusting time speed (faster or slower time)

// Sun-Related Variables
XMFLOAT4 sunColor = XMFLOAT4(1, 1, 1, 1);  // Color of the sun (used for lighting)
XMFLOAT4 apexColorVal;
XMFLOAT4 centreColorVal;

// Post-Processing Variables
XMFLOAT3 tintColor = XMFLOAT3(0.5, 0.4, 0.2);  // Tint color applied to the scene (for visual effects)
float tintStrength = 0.045;  // Strength of the tint effect applied
float brightness = 1.0;  // Brightness of the entire scene
float contrast = 1.02;  // Contrast adjustment for the scene
float saturation = 1.25;  // Saturation of the colors in the scene

// Control Booleans
bool postProcessingBool = true;  // Determines if post-processing effects should be applied
bool timeBool = true;  // Determines if the time progression is enabled
bool shadowBool = true;  // Enables or disables shadow rendering
bool resetBool = false;  // Flag for resetting the scene
bool debugBool = false;  // Enables or disables debug mode, typically for troubleshooting
bool smooth = false;


bool generateMap = false;
int call = 0;
std::wstring perlinNoiseTexName = L"perlinNoiseHeightMap" + std::to_wstring(call);

App1::App1()
{

}

// Initializes all the necessary resources for the application.
void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN)
{
	// Step 1: Initialize screen-related variables.
	// Storing screen width, height, and aspect ratio.
	screenHeightVar = screenHeight;
	screenWidthVar = screenWidth;
	aspectRatio = (float)screenWidth / (float)screenHeight;

	// Step 2: Call the base application's initialization function (initializes base resources).
	// Ensuring base application resources are initialized first (such as window setup, input management, etc.).
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Step 3: Initialize textures.
	// Loading various textures for the scene from files.

	// Height diffuse map sourced from https://www.motionforgepictures.com/height-maps/
	//textureMgr->loadTexture(L"heightMap", L"res/HMMR.png"); // Load height map texture.
	textureMgr->loadTexture(L"mountain diffuse", L"res/DFMR.png"); // Load mountain diffuse texture.
	// Spotlight mode and texture sourced from https://www.turbosquid.com/3d-models/free-street-lamp-3d-model/794502
	textureMgr->loadTexture(L"spotlight", L"res/Street_Lamp_DM.dds"); // Load spotlight texture.
	// Cottage is from lab assets
	textureMgr->loadTexture(L"cottage", L"res/cottage_diffuse_1.dds"); // Load cottage texture.
	// Clouds sourced from https://rastertek.com/pic0206.gif
	textureMgr->loadTexture(L"clouds1", L"res/cloud002.dds"); // Load cloud texture.
	// Sun texture sourced from https://it.pinterest.com/pin/417357090443735050/
	textureMgr->loadTexture(L"sunTex", L"res/sunTex.jpg"); // Load sun texture.

	// Step 4: Initialize mesh objects.
	// Create meshes used in the scene (plane, sphere, models).
	mainMesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext(), 50); // Plane mesh.
	sphere = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext()); // Sphere mesh.
	volumetricSphere = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());
	// Spotlight mode and texture sourced from https://www.turbosquid.com/3d-models/free-street-lamp-3d-model/794502
	spotlightModel = new AModel(renderer->getDevice(), "res/models/Street_Lamp.FBX"); // Spotlight model.
	// Cottage is from lab assets
	cottageModel = new AModel(renderer->getDevice(), "res/models/cottage_fbx.fbx"); // Cottage model.
	skyDome = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext()); // Sky dome class for the background.
	cloudsPlane = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext(), 1000); // Clouds plane.
	quad = new QuadMesh(renderer->getDevice(), renderer->getDeviceContext());
	sunSphere = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext(), 10); // Sun sphere.

	// Step 5: Initialize render textures.
	// Set up multiple render textures for various purposes (rendering to a texture for bloom, sun sphere, etc.).
	{
		renderTextureSource = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
		renderTextureSourceBloom = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
		renderTextureSunSphere[0] = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
		renderTextureSunSphere[1] = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
		renderTextureSunSphere[2] = new RenderTexture(renderer->getDevice(), screenWidth / 8, screenHeight / 8, SCREEN_NEAR, SCREEN_DEPTH);
		renderTextureSunSphere[3] = new RenderTexture(renderer->getDevice(), screenWidth / 16, screenHeight / 16, SCREEN_NEAR, SCREEN_DEPTH);
		renderTextureSunSphere[4] = new RenderTexture(renderer->getDevice(), screenWidth / 16, screenHeight / 16, SCREEN_NEAR, SCREEN_DEPTH);
		renderTextureSunSphere[5] = new RenderTexture(renderer->getDevice(), screenWidth / 16, screenHeight / 16, SCREEN_NEAR, SCREEN_DEPTH);
		renderTextureSunSphere[6] = new RenderTexture(renderer->getDevice(), screenWidth / 16, screenHeight / 16, SCREEN_NEAR, SCREEN_DEPTH);
		renderTextureSunSphere[7] = new RenderTexture(renderer->getDevice(), screenWidth / 16, screenHeight / 16, SCREEN_NEAR, SCREEN_DEPTH);
		renderTextureSunSphere[8] = new RenderTexture(renderer->getDevice(), screenWidth / 16, screenHeight / 16, SCREEN_NEAR, SCREEN_DEPTH);
		renderTextureSunSphere[9] = new RenderTexture(renderer->getDevice(), screenWidth / 16, screenHeight / 16, SCREEN_NEAR, SCREEN_DEPTH);
		renderTextureSunSphere[10] = new RenderTexture(renderer->getDevice(), screenWidth / 16, screenHeight / 16, SCREEN_NEAR, SCREEN_DEPTH);
		renderTextureSunSphere[11] = new RenderTexture(renderer->getDevice(), screenWidth / 16, screenHeight / 16, SCREEN_NEAR, SCREEN_DEPTH);
		renderTextureSunSphere[12] = new RenderTexture(renderer->getDevice(), screenWidth / 16, screenHeight / 16, SCREEN_NEAR, SCREEN_DEPTH);
		renderTextureSunSphere[13] = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
		renderTexturesBloom[0] = new RenderTexture(renderer->getDevice(), screenWidth / 6, screenHeight / 6, SCREEN_NEAR, SCREEN_DEPTH);
		renderTexturesBloom[1] = new RenderTexture(renderer->getDevice(), screenWidth / 8, screenHeight / 8, SCREEN_NEAR, SCREEN_DEPTH);
		renderTexturesBloom[2] = new RenderTexture(renderer->getDevice(), screenWidth / 8, screenHeight / 8, SCREEN_NEAR, SCREEN_DEPTH);
		renderTexturesBloom[3] = new RenderTexture(renderer->getDevice(), screenWidth / 8, screenHeight / 8, SCREEN_NEAR, SCREEN_DEPTH);
		renderTexturesBloom[4] = new RenderTexture(renderer->getDevice(), screenWidth / 8, screenHeight / 8, SCREEN_NEAR, SCREEN_DEPTH);
		renderTexturesBloom[5] = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
		renderTextureColorFilters = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	}

	// Step 6: Initialize ortho mesh (for rendering 2D elements like UI overlays).
	orthoMeshFull = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight, 0, 0);

	// Step 7: Set color guides (sky colors for different times of the day).
	// Initializing sunrise colors.
	centreColorVal = (XMFLOAT4(0.9568627450980393, 0.2549019607843137, 0.24705882352941178, 1)); // Set center color for sunrise.
	apexColorVal = (XMFLOAT4(0.00392156862745098, 0.403921568627451, 0.8313725490196079, 1)); // Set apex color for sunrise.
	tintColor = XMFLOAT3(1.0, 0.85, 0.6); // Set tint color for the sunrise effect.
	tintStrength = 0.045; // Set tint strength.
	brightness = 1.5; // Set brightness.
	contrast = 1.02; // Set contrast.
	saturation = 1.25; // Set saturation.
	sunColor = XMFLOAT4(1.0, 0.3, 0.3, 1.0); // Set sun color for sunrise.
	// Noon colors.
	/*skyDome->SetCenterColor(XMFLOAT4(0.9607843137254902, 0.8470588235294118, 0.7294117647058823, 1));
	skyDome->SetApexColor(XMFLOAT4(0.615686274509804, 0.6392156862745098, 0.7294117647058823, 1));
	tintColor = XMFLOAT3(1.0, 1.0, 0.9);
	tintStrength = 0.04;
	brightness = 1.4;
	contrast = 1.015;
	saturation = 1.35;
	sunColor = XMFLOAT4(1.0, 1.0, 0.9, 1.0);*/
	// Sunset colors.
	/*skyDome->SetCenterColor(XMFLOAT4(0.9568627450980393, 0.2549019607843137, 0.24705882352941178, 1));
	skyDome->SetApexColor(XMFLOAT4(0.12941176470588237, 0.043137254901960784, 0.33725490196078434, 1));
	tintColor = XMFLOAT3(1.0, 0.5, 0.3);
	tintStrength = .06;
	brightness = 1.5;
	contrast = 1.02;
	saturation = 1.35;
	sunColor = XMFLOAT4(1.0, 0.2, 0.2, 1.0);*/
	// Night colors.
	/*skyDome->SetCenterColor(XMFLOAT4(0, 0.011764705882352941, 0.019607843137254902, 1));
	skyDome->SetApexColor(XMFLOAT4(0, 0.13725490196078433/3, 0.23137254901960785/3, 1));
	tintColor = XMFLOAT3(0.2, 0.2, 0.8);
	tintStrength = .07;
	brightness = 1.5;
	contrast = 1.02;
	saturation = 1.35;*/

	// Step 8: Initialize shaders.
	// Create instances of different shaders used for rendering (vertex manipulation, depth rendering, lighting, etc.).
	depthShaderTess = new DepthShader(renderer->getDevice(), hwnd, L"VertexManipulation_vs.cso", L"VertexManipulation_hs.cso", L"VertexManipulation_ds.cso", L"depth_ps.cso");
	lightShaderTess = new LightShader(renderer->getDevice(), hwnd, L"VertexManipulation_vs.cso", L"VertexManipulation_hs.cso", L"VertexManipulation_ds.cso", L"VertexManipulation_ps.cso");
	lightShader = new LightShader(renderer->getDevice(), hwnd, L"light_vs.cso", L"lightNonTess_ps.cso");
	depthShader = new DepthShader(renderer->getDevice(), hwnd, L"depth_vs.cso", L"depth_ps.cso");
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	skyDomeShader = new SkyDomeShaderClass(renderer->getDevice(), hwnd, L"SkyDomeShader_vs.cso", L"SkyDomeShader_ps.cso");
	cloudsShader = new CloudsShader(renderer->getDevice(), hwnd, L"CloudsShader_vs.cso", L"CloudsShader_ps.cso");
	brightnessFilterShader = new BrightnessFilterShader(renderer->getDevice(), hwnd, L"texture_vs.cso", L"BrightnessFilterShader_ps.cso");
	sunBrightnessFilterShader = new BrightnessFilterShader(renderer->getDevice(), hwnd, L"texture_vs.cso", L"SunBrightnessFilterShader_ps.cso");
	gaussianBlurShader = new GaussianBlurShader(renderer->getDevice(), hwnd, L"texture_vs.cso", L"GaussianBlurShader_ps.cso");
	blendShader = new BlendShader(renderer->getDevice(), hwnd, L"texture_vs.cso", L"BlendShader_ps.cso");
	colorFilterShader = new ColorGradingShader(renderer->getDevice(), hwnd, L"texture_vs.cso", L"ColorGradingShader_ps.cso");
	sunShader = new SunShader(renderer->getDevice(), hwnd, L"texture_vs.cso", L"SunShader_ps.cso");

	// Step 9: Initialize light objects.
	// Create lights and set their properties (position, direction, intensity, color, etc.).
	for (int i = 0; i < lightSize; i++) {
		light[i] = new Light(); // Create lights.
	}
	lightType[0].y = 1.f;
	lightType[1].z = 1.f;
	intensity[0] = .2;
	intensity[1] = 1;

	// Step 10: Initialize shadow maps.
	// Create shadow maps to store depth information for shadows.
	int shadowmapWidth = 1024 * 4;
	int shadowmapHeight = 1024 * 4;
	for (int i = 0; i < lightSize; i++) {
		shadowMaps[i] = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);
		if (lightType[i].y == 1.f) {
			direction[i] = XMFLOAT4(1.f, -1.f, 0.f, 0.f);
			light[i]->setDirection(direction[i].x, direction[i].y, direction[i].z);
			XMFLOAT3 norLightDir = light[i]->getDirection();
			position[i] = XMFLOAT4(pivot.x - norLightDir.x * radius.x, pivot.y - norLightDir.y * radius.y, pivot.z - norLightDir.z * radius.z, 1.f);
			light[i]->setPosition(position[i].x, position[i].y, position[i].z);
			diffuse[i] = XMFLOAT4(.902f, .455f, .318f, 1.f);
			ambient[i] = XMFLOAT4(.015, .010, .004, 1.f);
			specularColour[i] = XMFLOAT4(.902f, .455f, .318f, 1.f);
			light[i]->setDiffuseColour(intensity[0] * diffuse[i].x, intensity[0] * diffuse[i].y, intensity[0] * diffuse[i].z, 1.f);
			light[i]->setAmbientColour(ambient[i].x, ambient[i].y, ambient[i].z, 1.f);
			light[i]->setSpecularColour(specularColour[i].x, specularColour[i].y, specularColour[i].z, 1.f);
			specularPower[i] = XMFLOAT4(25, 0, 0, 1.f);
			light[i]->setSpecularPower(specularPower[i].x);
			light[i]->generateOrthoMatrix((float)100, (float)100, SCREEN_NEAR, SCREEN_DEPTH);
		}
		else {
			light[i]->setPosition(position[i].x, position[i].y, position[i].z);
			direction[i] = XMFLOAT4(0.f, -1.f, .5f, 0.f);
			position[i] = XMFLOAT4(25.f, 14.65, 26.7, 0.f);
			diffuse[i] = XMFLOAT4(1.f, 0.f, 0.f, 1.f);
			light[i]->setDirection(direction[i].x, direction[i].y, direction[i].z);
			light[i]->setPosition(position[i].x, position[i].y, position[i].z);
			light[i]->setDiffuseColour(intensity[1] * diffuse[i].x, intensity[1] * diffuse[i].y, intensity[1] * diffuse[i].z, 1.f);
			light[i]->generateProjectionMatrix(SCREEN_NEAR, SCREEN_DEPTH);
		}
	}

	perlinNoiseTexture = new PerlinNoiseTexture(50);
	//perlinNoiseTexture->GeneratePerlinNoiseTexture(renderer->getDevice(), textureMgr, params.x, params.y, params.z);
}

// Destructor for the App1 class, handles cleanup of allocated resources.
App1::~App1()
{
	// Step 1: Run the base class destructor to clean up base class resources.
	BaseApplication::~BaseApplication();

	// Step 2: Clean up shaders.
	if (depthShaderTess) {
		delete depthShaderTess;
		depthShaderTess = nullptr;
	}
	if (depthShader) {
		delete depthShader;
		depthShader = nullptr;
	}
	if (lightShaderTess) {
		delete lightShaderTess;
		lightShaderTess = nullptr;
	}
	if (lightShader) {
		delete lightShader;
		lightShader = nullptr;
	}
	if (textureShader) {
		delete textureShader;
		textureShader = nullptr;
	}
	if (skyDomeShader) {
		delete skyDomeShader;
		skyDomeShader = nullptr;
	}
	if (cloudsShader) {
		delete cloudsShader;
		cloudsShader = nullptr;
	}
	if (brightnessFilterShader) {
		delete brightnessFilterShader;
		brightnessFilterShader = nullptr;
	}
	if (sunBrightnessFilterShader) {
		delete sunBrightnessFilterShader;
		sunBrightnessFilterShader = nullptr;
	}
	if (gaussianBlurShader) {
		delete gaussianBlurShader;
		gaussianBlurShader = nullptr;
	}
	if (blendShader) {
		delete blendShader;
		blendShader = nullptr;
	}
	if (colorFilterShader) {
		delete colorFilterShader;
		colorFilterShader = nullptr;
	}
	if (sunShader) {
		delete sunShader;
		sunShader = nullptr;
	}

	// Step 3: Clean up meshes.
	if (mainMesh) {
		delete mainMesh;
		mainMesh = nullptr;
	}
	if (cloudsPlane) {
		delete cloudsPlane;
		cloudsPlane = nullptr;
	}
	if (sphere) {
		delete sphere;
		sphere = nullptr;
	}
	if (spotlightModel) {
		delete spotlightModel;
		spotlightModel = nullptr;
	}
	if (cottageModel) {
		delete cottageModel;
		cottageModel = nullptr;
	}
	if (skyDome) {
		delete skyDome;
		skyDome = nullptr;
	}
	if (sunSphere) {
		delete sunSphere;
		sunSphere = nullptr;
	}

	// Step 4: Clean up render textures.
	if (renderTextureSource) {
		delete renderTextureSource;
		renderTextureSource = nullptr;
	}
	if (renderTextureSourceBloom) {
		delete renderTextureSourceBloom;
		renderTextureSourceBloom = nullptr;
	}
	for (int i = 0; i < 14; i++) {
		if (renderTextureSunSphere[i]) {
			delete renderTextureSunSphere[i];
			renderTextureSunSphere[i] = nullptr;
		}
	}
	for (int i = 0; i < 6; i++) {
		if (renderTexturesBloom[i]) {
			delete renderTexturesBloom[i];
			renderTexturesBloom[i] = nullptr;
		}
	}
	if (renderTextureColorFilters) {
		delete renderTextureColorFilters;
		renderTextureColorFilters = nullptr;
	}

	// Step 5: Clean up ortho meshes.
	if (orthoMeshFull) {
		delete orthoMeshFull;
		orthoMeshFull = nullptr;
	}
	
	// Step 6: Clean up light objects.
	for (int i = 0; i < lightSize; i++) {
		if (light[i]) {
			delete light[i];
			light[i] = nullptr;
		}
	}

	// Step 7: Clean up shadow maps.
	for (int i = 0; i < lightSize; i++) {
		if (shadowMaps[i]) {
			delete shadowMaps[i];
			shadowMaps[i] = nullptr;
		}
	}
}

// Handles the main frame updates for the application, including rendering.
bool App1::frame()
{
	bool result;

	// Step 1: Call the base class frame function, which may handle common tasks like input or updating base components.
	result = BaseApplication::frame();
	if (!result)
	{
		return false;  // If the base frame function fails, return false to stop execution.
	}

	// Step 2: Call the render function to render the graphics for the current frame.
	result = render();
	if (!result)
	{
		return false;  // If rendering fails, return false to stop execution.
	}

	// Step 3: Return true if both the base frame and rendering were successful.
	return true;
}

// Main render function to handle all the stages of the rendering pipeline: scene rendering, shadow mapping, lighting, post-processing, and GUI rendering.
bool App1::render()
{
	// Step 1: Reset time when resetBool is set to true.
	if (resetBool) {
		elapsedTime = 0.f;   // Reset elapsed time to 0 for starting a fresh cycle.
		resetBool = false;    // Disable reset flag after resetting time.
	}

	// Step 2: Begin rendering the scene, clearing the screen with black.
	renderer->beginScene(0, 0, 0, 0); // Begin a new frame, setting background color to black.

	// Step 3: Set up the render texture and clear it for the initial render.
	renderTextureSource->setRenderTarget(renderer->getDeviceContext()); // Set render target to renderTextureSource.
	renderTextureSource->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 0.0f); // Clear with black (no color).

	// Step 4: Render the SkyBox (background) first.
	SkyBox(); // Call SkyBox function to render the sky before anything else (background effect).

	// Step 5: Render clouds on top of the skybox.
	//Clouds(); // Render cloud texture to the scene.

	// Step 6: Set wireframe mode if wireframeToggle is enabled.
	renderer->setWireframeMode(wireframeToggle); // If wireframeToggle is true, enable wireframe rendering mode.

	// Step 7: Optionally, enable shadow depth rendering.
	if (shadowBool) {
		shadowDepth(); // Call shadow depth function to generate shadow maps if shadowBool is true.
	}
	else {
		// Step 8: Clear shadow maps if shadows are disabled.
		for (int i = 0; i < lightSize; i++) {
			shadowMaps[i]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext()); // Unbind shadow maps.
			renderer->setBackBufferRenderTarget(); // Reset to back buffer.
			renderer->resetViewport(); // Reset viewport for the next render.
		}
	}

	// Step 9: Render the lighting effects (shaders and light sources).
	lighting(renderTextureSource, false); // Apply lighting effects to the renderTextureSource.

	// Step 10: Render the sun sphere in the scene.
	renderSunSphere(); // Render the sun in the scene (could be part of a lighting effect).

	Clouds();

	// Step 11: Reset the render target to the back buffer and disable wireframe mode.
	renderer->setBackBufferRenderTarget(); // Reset render target to back buffer for final scene rendering.
	renderer->setWireframeMode(false); // Turn off wireframe mode after rendering.

	// Step 12: Apply post-processing effects if enabled.
	if (postProcessingBool) {
		postProcessing(); // Apply post-processing effects such as bloom, color grading, etc.
		postProcessingBool = false;
	}

	// Step 13: Perform final rendering (combine all passes, add effects).
	finalRender(); // Final scene rendering including any post-processing effects.

	// Step 14: Render the graphical user interface (GUI) elements.
	gui(); // Render the GUI on top of the scene (HUD, menus, etc.).

	// Step 15: Present the rendered scene to the screen.
	renderer->endScene(); // Finalize the frame and present the result to the screen.

	return true; // Return true indicating the render function executed successfully.
}

// Renders the sun sphere in the scene, applying a sun shader and adjusting the position based on light direction.
void App1::renderSunSphere() {
	// Step 1: Check if the light direction is valid (not equal to the default).
	if (light[0]->getDirection().x != 0 && light[0]->getDirection().y != 0.1) {
		// Step 2: Prepare the transformation matrices for the sun sphere rendering.
		XMMATRIX worldMatrix = renderer->getWorldMatrix();    // World matrix for the sun sphere.
		XMMATRIX viewMatrix = camera->getViewMatrix();        // Camera view matrix.
		XMMATRIX projectionMatrix = renderer->getProjectionMatrix(); // Projection matrix for 3D rendering.

		// Step 3: Send the sun sphere data to the GPU.
		sunSphere->sendData(renderer->getDeviceContext());    // Send the sphere data for rendering.

		// Step 4: Set the shader parameters for the sun sphere rendering, including position, scale, and texture.
		// Apply transformations (scaling and translation), texture, and sun color.
		sunShader->setShaderParameters(
			renderer->getDeviceContext(),
			worldMatrix * XMMatrixScaling(1.5, 1.5, 1.5) * XMMatrixTranslation(position[0].x, position[0].y, position[0].z), // Scaling and translating the sun sphere.
			viewMatrix,    // View matrix for camera positioning.
			projectionMatrix, // Projection matrix for perspective.
			textureMgr->getTexture(L"sunTex"), // Sun texture to apply.
			sunColor       // Sun color to apply.
		);

		// Step 5: Render the sun sphere with the sun shader applied.
		sunShader->render(renderer->getDeviceContext(), sunSphere->getIndexCount()); // Render the sun sphere.
	}
}

// Renders clouds in the scene, updates cloud movement, and applies a cloud texture based on elapsed time.
// Rastertek (2013) DirectX 11 Terrain Tutorial (Lesson 11) (code version 1) [online tutorial]. Adapted from: https://rastertek.com/tertut11.html.
void App1::Clouds() {
	// Step 1: Increment the elapsed time to control cloud movement animation.
	timeFloat += timer->getTime();

	// Step 2: Retrieve and update the camera's position for correct cloud positioning.
	XMFLOAT3 camPosition;
	camera->update(); // Update the camera position and rotation.
	camPosition = camera->getPosition(); // Retrieve the updated camera position.

	// Step 3: Set render states for skybox rendering.
	renderer->setFaceCulling(D3D11_CULL_NONE); // Disable culling so that both sides of the clouds are visible.
	renderer->setZBuffer(false); // Disable Z-buffer to avoid depth issues when rendering clouds in the background.

	// Step 4: Prepare transformation matrices for the clouds.
	XMMATRIX worldMatrix = renderer->getWorldMatrix(); // World matrix for the clouds.
	XMMATRIX viewMatrix = camera->getViewMatrix();     // Camera's view matrix.
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix(); // Projection matrix for rendering.

	// Step 5: Adjust the world matrix to position the clouds based on the camera's position.
	// This ensures that clouds move with the camera for a more natural effect.
	//worldMatrix = XMMatrixTranslation(camPosition.x - 500, camPosition.y + 40, camPosition.z - 500);

	// Step 6: Send the cloud plane data to the GPU for rendering.
	//cloudsPlane->sendData(renderer->getDeviceContext());
	volumetricSphere->sendData(renderer->getDeviceContext());
	cloudsShader->setShaderParameters(
		renderer->getDeviceContext(),
		worldMatrix * XMMatrixTranslation(35, 12, 32),              // Position the clouds correctly.
		viewMatrix,               // Camera view for proper positioning.
		projectionMatrix,         // Project the clouds into 3D space.
		textureMgr->getTexture(L"default"),
		camera->getPosition(),
		XMFLOAT3(35, 12, 32),
		1,
		light[0]->getDirection(),
		light[0]->getDiffuseColour(),
		.4f						//the higher the value, the thicker the gas
	);
	cloudsShader->render(renderer->getDeviceContext(), volumetricSphere->getIndexCount());

	// Step 7: Determine the movement rate of the clouds based on whether time is enabled (timeBool).
	// This controls how quickly the clouds move across the screen.
	float moveRate = 0;
	if (timeBool) {
		moveRate = 0.01 / 2;  // Adjust the rate at which clouds move.
	}

	// Step 8: Set shader parameters for the clouds, including world, view, projection matrices, cloud texture, movement rate, and time.
	//cloudsShader->setShaderParameters(
	//	renderer->getDeviceContext(),
	//	worldMatrix,              // Position the clouds correctly.
	//	viewMatrix,               // Camera view for proper positioning.
	//	projectionMatrix,         // Project the clouds into 3D space.
	//	textureMgr->getTexture(L"clouds1"), // Cloud texture to apply to the cloud plane.
	//	moveRate,                 // Movement speed of the clouds.
	//	timeFloat                 // Elapsed time to animate cloud movement.
	//);

	// Step 9: Render the clouds with the applied shader.
	//cloudsShader->render(renderer->getDeviceContext(), cloudsPlane->getIndexCount());

	// Step 10: Restore the render states for subsequent rendering.
	renderer->setFaceCulling(D3D11_CULL_BACK);  // Enable back face culling for future objects.
	renderer->setZBuffer(true);  // Re-enable Z-buffer for proper depth testing.
}

// Renders the skybox and updates the view based on the camera's position.
// Rastertek (2013) DirectX 11 Terrain Tutorial(Lesson 10) (code version 1)[online tutorial]. Adapted from: https://rastertek.com/tertut10.html.
void App1::SkyBox() {
	// Step 1: Get the current camera position.
	XMFLOAT3 camPosition;
	camera->update();  // Update the camera's position and rotation.
	camPosition = camera->getPosition();  // Retrieve the camera position.

	// Step 2: Set render states for skybox rendering.
	renderer->setFaceCulling(D3D11_CULL_NONE);  // Disable face culling for skybox rendering (render both sides).
	renderer->setZBuffer(false);  // Disable the Z-buffer to allow rendering the skybox on top of all other objects.

	// Step 3: Prepare transformation matrices for rendering.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();  // World matrix for the skybox (transformation from object space to world space).
	XMMATRIX viewMatrix = camera->getViewMatrix();      // Camera's view matrix (defines the camera's position and orientation).
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();  // Projection matrix (defines how to project 3D objects onto a 2D screen).

	// Step 4: Adjust the world matrix to move the skybox with the camera.
	// Move the skybox to the camera position to create the effect of the skybox surrounding the viewer.
	worldMatrix = XMMatrixTranslation(camPosition.x, camPosition.y, camPosition.z);  // Translate the skybox to the camera's position.

	// Step 5: Render the skybox.
	skyDome->sendData(renderer->getDeviceContext());  // Render the sky dome geometry.

	// Step 6: Set shader parameters for the skybox and apply the shader.
	skyDomeShader->setShaderParameters(
		renderer->getDeviceContext(),
		worldMatrix,          // World matrix (position the skybox around the camera).
		viewMatrix,           // View matrix (camera's position and orientation).
		projectionMatrix,     // Projection matrix.
		apexColorVal,         // Color for the apex (top) of the skybox.
		centreColorVal,       // Color for the center of the skybox.
		position[0]           // Light position (for dynamic lighting effects in the skybox).
	);

	// Step 7: Render the skybox using the applied shader.
	skyDomeShader->render(renderer->getDeviceContext(), skyDome->getIndexCount());  // Render the sky dome with the shader applied.

	// Step 8: Restore render states.
	renderer->setFaceCulling(D3D11_CULL_BACK);  // Re-enable back face culling for subsequent rendering.
	renderer->setZBuffer(true);  // Re-enable the Z-buffer for future rendering passes.
}

// Linearly interpolate between two values, a and b, based on the interpolation factor t.
// cppreference.com (n.d.) std::lerp - cppreference.com (online reference). Adapted from: https://en.cppreference.com/w/cpp/numeric/lerp.
float lerp(const float& a, const float& b, float t) {
	// Clamp t to the range [0, 1] to avoid going beyond the two values.
	if (t <= 0) {
		return a;  // If t is 0, return the starting value a.
	}
	if (t >= 1) {
		return b;  // If t is 1, return the ending value b.
	}
	return a + t * (b - a);  // Linearly interpolate between a and b.
}

// Update the day-night cycle based on elapsed time and adjust the lighting and sky colors.
void DayNightCycle(float& eTime, Light* light) {
	// Variables to hold the colors of the sky dome during different times of day
	XMFLOAT4 centerColor;
	XMFLOAT4 apexColor;
	apexColor.w = 1; // Set alpha value of apex color to 1 (opaque)
	centerColor.w = 1; // Set alpha value of center color to 1 (opaque)

	// Calculate day duration based on time scale
	dayDuration = (60 * 20.f) / timeScale;
	float dayHrs = .5f * dayDuration; // Half of the day duration is considered daytime
	float nightHrs = dayDuration - dayHrs; // The rest of the time is considered nighttime

	// Reset elapsed time if it exceeds the total day duration (reset for next day)
	if (eTime > dayDuration) {
		eTime = 0;
	}
	// Daytime calculation
	else if (eTime <= dayHrs) {
		// Calculate the interpolation factor (t) for the sun position
		float t = eTime / dayHrs;
		float minVal = 0.2f;    // Minimum intensity at sunrise
		float maxVal = 1.0f;    // Maximum intensity at noon
		// Calculate the intensity of the sunlight based on time of day
		intensity[0] = minVal + (maxVal - minVal) * max(0.0f, sin((eTime / dayHrs) * XM_PI));

		// Calculate the sun's position using lerping
		float sunX = lerp(cos(0), cos(XM_PI), t);
		float sunY = 0;

		// Transitioning from morning to afternoon
		if (eTime < dayHrs / 2) {
			sunY = lerp(2 * cos(XM_PI / 2), 2 * cos(XM_PI), t);
			float T = t * 2; // Calculate time factor for color transitions

			// Update the center and apex colors of the sky dome
			centerColor.x = lerp(.95, .96, T);
			centerColor.y = lerp(.25, .84, T);
			centerColor.z = lerp(.24, .72, T);
			apexColor.x = lerp(.003, .61, T);
			apexColor.y = lerp(.403, .63, T);
			apexColor.z = lerp(.831, .72, T);

			// Update the tint, brightness, contrast, and saturation for a sunrise look
			tintColor.x = lerp(1, 1, T);
			tintColor.y = lerp(.85, 1, T);
			tintColor.z = lerp(.6, .9, T);
			tintStrength = lerp(.045, .04, T);
			brightness = lerp(1.5, 1.4, T);
			contrast = lerp(1.02, 1.015, T);
			saturation = lerp(1.25, 1.35, T);

			// Set the sun color for a warm yellow tone
			sunColor.x = lerp(1., 1., T);
			sunColor.y = lerp(.3, 1, T);
			sunColor.z = lerp(.3, .9, T);
		}
		// Afternoon to evening transition
		else if (eTime >= dayHrs / 2) {
			sunY = lerp(2 * cos(XM_PI), 2 * cos(XM_PI / 2), t);
			float T = (t - .5) / (1 - .5); // Normalize time for smooth transition

			// Update the sky dome colors for an evening look
			centerColor.x = lerp(.96, .95, T);
			centerColor.y = lerp(.84, .25, T);
			centerColor.z = lerp(.72, .24, T);
			apexColor.x = lerp(.61, .12, T);
			apexColor.y = lerp(.63, .043, T);
			apexColor.z = lerp(.72, .33, T);

			// Update tint color and other parameters for the evening ambiance
			tintColor.x = lerp(1, 1, T);
			tintColor.y = lerp(1, .5, T);
			tintColor.z = lerp(.9, .3, T);
			tintStrength = lerp(.04, .06, T);
			brightness = lerp(1.4, 1.5, T);
			contrast = lerp(1.015, 1.02, T);
			saturation = lerp(1.35, 1.35, T);

			// Set the sun color for a reddish tone at sunset
			sunColor.x = lerp(1, 1, T);
			sunColor.y = lerp(1, .2, T);
			sunColor.z = lerp(.9, .2, T);
		}

		// Set the light direction and update the sky dome colors
		direction[0] = XMFLOAT4(sunX, sunY, direction[0].z, direction[0].w);
		apexColorVal = apexColor;
		centreColorVal = centerColor;

		// Update light position based on the direction of the sun
		XMFLOAT3 norLightDir = light->getDirection();
		position[0] = XMFLOAT4(std::clamp(pivot.x - norLightDir.x * radius.x, -100.f, 100.f),
			std::clamp(pivot.y - norLightDir.y * radius.y, -100.f, 100.f),
			std::clamp(pivot.z - norLightDir.z * radius.z, -100.f, 100.f), 1.f);
		light->setDirection(direction[0].x, direction[0].y, direction[0].z);
		light->setSpecularColour(specularColour[0].x, specularColour[0].y, specularColour[0].z, 1.f);
		light->setAmbientColour(ambient[0].x, ambient[0].y, ambient[0].z, 1.f);
	}
	// Nighttime calculations
	else if (eTime >= dayHrs && eTime <= dayDuration) {
		float nightElapsed = eTime - dayHrs; // Time elapsed during the night
		float midNight = nightHrs / 2; // Calculate halfway point of the night
		float t = nightElapsed / nightHrs; // Normalize the time

		// Transition from evening to full night
		if (nightElapsed < midNight) {
			float T = 4 * t;
			// Update the sky dome colors to simulate nightfall
			centerColor.x = lerp(.95, 0, T);
			centerColor.y = lerp(.25, .011, T);
			centerColor.z = lerp(.24, .019, T);
			apexColor.x = lerp(.12, 0, T);
			apexColor.y = lerp(.043, .045, T);
			apexColor.z = lerp(.33, .077, T);

			// Update the tint color for nighttime ambiance
			tintColor.x = lerp(1, .2, T);
			tintColor.y = lerp(.5, .2, T);
			tintColor.z = lerp(.3, .8, T);
			tintStrength = lerp(.06, .07, T);
			brightness = lerp(1.5, 1.5, T);
			contrast = lerp(1.02, 1.02, T);
			saturation = lerp(1.35, 1.35, T);
		}
		// Transition from night to early morning
		else if (nightElapsed >= midNight) {
			float T = (t - .5) / (1 - .5); // Normalize time for transition back to morning
			centerColor.x = lerp(0, .95, T);
			centerColor.y = lerp(.011, .25, T);
			centerColor.z = lerp(.019, .24, T);
			apexColor.x = lerp(0, .003, T);
			apexColor.y = lerp(.045, .403, T);
			apexColor.z = lerp(.077, .83, T);

			// Set tint color for early morning
			tintColor.x = lerp(.2, 1, T);
			tintColor.y = lerp(.2, .85, T);
			tintColor.z = lerp(.8, .6, T);
			tintStrength = lerp(.07, .045, T);
			brightness = lerp(1.5, 1.5, T);
			contrast = lerp(1.02, 1.02, T);
			saturation = lerp(1.35, 1.25, T);
		}

		// Set the light intensity and direction for nighttime
		intensity[0] = 0.f; // No sunlight at night
		light->setDiffuseColour(intensity[0] * diffuse[0].x, intensity[0] * diffuse[0].y, intensity[0] * diffuse[0].z, 1.0f);

		// Set the light direction to a low angle for night
		direction[0] = XMFLOAT4(0, 0.1, 0, 0);
		light->setDirection(direction[0].x, direction[0].y, direction[0].z); // Apply light direction

		// Calculate the light's position based on its direction
		XMFLOAT3 norLightDir = light->getDirection();
		position[0] = XMFLOAT4(std::clamp(pivot.x - norLightDir.x * radius.x, -100.f, 100.f),
			std::clamp(pivot.y - norLightDir.y * radius.y, -100.f, 100.f),
			std::clamp(pivot.z - norLightDir.z * radius.z, -100.f, 100.f), 1.f);

		// Set specular and ambient colors to 0 for no light at night
		light->setSpecularColour(0, 0, 0, 1.f);
		light->setAmbientColour(ambient[0].x, ambient[0].y, ambient[0].z, 1.f); // Set ambient color for night

		// Update sky dome colors for night
		apexColorVal = apexColor;
		centreColorVal = centerColor;
	}
}

// Perform the lighting pass and apply the appropriate shaders to the scene, including shadow mapping and time-based day-night cycle changes.
void App1::lighting(RenderTexture* renderTexture, bool clear) {
	// Step 1: Update the time and apply the day-night cycle if timeBool is enabled.
	if (timeBool) {
		elapsedTime += timer->getTime(); // Increment elapsed time.
		DayNightCycle(elapsedTime, light[0]); // Update day-night cycle based on elapsed time.
	}

	// Step 2: Update camera and set render target.
	camera->update(); // Update the camera position and rotation.
	renderTexture->setRenderTarget(renderer->getDeviceContext()); // Set the output render target.
	if (clear) {
		renderTexture->clearRenderTarget(renderer->getDeviceContext(), 0, 0, 0, 0);
	}

	// Step 3: Prepare transformation matrices for rendering.
	XMMATRIX worldMatrix = renderer->getWorldMatrix(); // World transformation matrix.
	XMMATRIX viewMatrix = camera->getViewMatrix();     // Camera view matrix.
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix(); // Projection matrix for the scene.

	// Step 4: Prepare the shadow map resources for each light source.
	ID3D11ShaderResourceView* shadowMapsRSV[lightSize]; // Array of shadow map resources.
	for (int i = 0; i < lightSize; i++) {
		shadowMapsRSV[i] = shadowMaps[i]->getDepthMapSRV(); // Get shadow map for each light source.
	}

	// Step 5: Render main mesh with tessellation and lighting effects.
	mainMesh->sendData(renderer->getDeviceContext(), D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST); // Send main mesh data.
	lightShaderTess->setShaderParametersTess(
			renderer->getDeviceContext(),
			worldMatrix,        // World matrix for transformations.
			viewMatrix,         // View matrix for the camera's perspective.
			projectionMatrix,   // Projection matrix for 3D scene rendering.
			textureMgr->getTexture(L"perlinNoiseHeightMap"), // Height map texture for terrain.
			textureMgr->getTexture(L"mountain diffuse"), // Diffuse texture for the terrain.
			light,              // Lights to be applied.
			lightType,          // Light types (e.g., directional, spotlight).
			camera->getPosition(), // Camera position for lighting calculations.
			params,
			shadowMapsRSV       // Shadow maps for all lights.
		);
	lightShaderTess->render(renderer->getDeviceContext(), mainMesh->getIndexCount()); // Render with tessellated shader.

	// Step 6: Render additional objects (cottage, sphere, spotlight model) with the lighting shader.
	cottageModel->sendData(renderer->getDeviceContext());
	lightShader->setShaderParameters(
		renderer->getDeviceContext(),
		worldMatrix * XMMatrixRotationX(XM_PI / 2) * XMMatrixScaling(1, .4, .75) * XMMatrixTranslation(cottagePosition.x, cottagePosition.y, cottagePosition.z),
		viewMatrix,
		projectionMatrix,
		textureMgr->getTexture(L"cottage"), // Cottage texture.
		light,
		lightType,
		camera->getPosition(),
		shadowMapsRSV
	);
	lightShader->render(renderer->getDeviceContext(), cottageModel->getIndexCount()); // Render cottage model.

	//sphere->sendData(renderer->getDeviceContext());
	//lightShader->setShaderParameters(
	//	renderer->getDeviceContext(),
	//	worldMatrix * XMMatrixScaling(sphereScale.x, sphereScale.y, sphereScale.z) * XMMatrixTranslation(spherePos.x, spherePos.y, spherePos.z),
	//	viewMatrix,
	//	projectionMatrix,
	//	textureMgr->getTexture(L"default"), // Default texture for sphere.
	//	light,
	//	lightType,
	//	camera->getPosition(),
	//	shadowMapsRSV
	//);
	//lightShader->render(renderer->getDeviceContext(), sphere->getIndexCount()); // Render sphere model.

	// Step 7: Render spotlight model.
	spotlightModel->sendData(renderer->getDeviceContext());
	lightShader->setShaderParameters(
		renderer->getDeviceContext(),
		worldMatrix * XMMatrixScaling(0.05, 0.05, 0.05) * XMMatrixRotationX(XM_PI / 2) * XMMatrixRotationY(-XM_PI / 2) * XMMatrixTranslation(spotlightModelPosition.x, spotlightModelPosition.y, spotlightModelPosition.z),
		viewMatrix,
		projectionMatrix,
		textureMgr->getTexture(L"spotlight"), // Spotlight texture.
		light,
		lightType,
		camera->getPosition(),
		shadowMapsRSV
	);
	lightShader->render(renderer->getDeviceContext(), spotlightModel->getIndexCount()); // Render spotlight model.
}

// Render the depth information for shadows based on the light source(s).
void App1::shadowDepth() {
	// Prepare matrices for light's view and projection.
	XMMATRIX lightViewMatrix, lightOrthoMatrix, lightProjectionMatrix, worldMatrix;
	camera->update(); // Update camera position and rotation.

	// Loop through each light to generate shadows for both directional and spotlight lights.
	for (int i = 0; i < lightSize; i++) {
		// Set the shadow map as the render target for depth information.
		shadowMaps[i]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

		// Generate view matrix for the current light.
		light[i]->generateViewMatrix();

		// If the light is a directional light, use orthographic projection for depth capture.
		if (lightType[i].y == 1.f) {
			lightViewMatrix = light[i]->getViewMatrix();
			light[i]->generateOrthoMatrix((float)100, (float)100, SCREEN_NEAR, SCREEN_DEPTH);
			lightOrthoMatrix = light[i]->getOrthoMatrix();
			worldMatrix = renderer->getWorldMatrix(); // Get world matrix for rendering.

			// Render the main mesh with tessellation for the shadow map.
			mainMesh->sendData(renderer->getDeviceContext(), D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
			depthShaderTess->setShaderParametersTess(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightOrthoMatrix, camera->getPosition(), textureMgr->getTexture(L"heightMap"));
			depthShaderTess->render(renderer->getDeviceContext(), mainMesh->getIndexCount());

			// Render additional objects like the sphere, cottage, and spotlight model.
			sphere->sendData(renderer->getDeviceContext());
			depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * XMMatrixScaling(sphereScale.x, sphereScale.y, sphereScale.z) * XMMatrixTranslation(spherePos.x, spherePos.y, spherePos.z), lightViewMatrix, lightOrthoMatrix);
			depthShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

			cottageModel->sendData(renderer->getDeviceContext());
			depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * XMMatrixRotationX(XM_PI / 2) * XMMatrixScaling(1, .4, .75) * XMMatrixTranslation(cottagePosition.x, cottagePosition.y, cottagePosition.z), lightViewMatrix, lightOrthoMatrix);
			depthShader->render(renderer->getDeviceContext(), cottageModel->getIndexCount());

			spotlightModel->sendData(renderer->getDeviceContext());
			depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * XMMatrixScaling(0.05, 0.05, 0.05) * XMMatrixRotationX(XM_PI / 2) * XMMatrixRotationY(-XM_PI / 2) * XMMatrixTranslation(spotlightModelPosition.x, spotlightModelPosition.y, spotlightModelPosition.z), lightViewMatrix, lightOrthoMatrix);
			depthShader->render(renderer->getDeviceContext(), spotlightModel->getIndexCount());
		}
		// If the light is a spotlight, use perspective projection for depth capture.
		else {
			lightViewMatrix = light[i]->getViewMatrix();
			light[i]->generateProjectionMatrix(SCREEN_NEAR, SCREEN_DEPTH);
			lightProjectionMatrix = light[i]->getProjectionMatrix();
			worldMatrix = renderer->getWorldMatrix(); // Get world matrix for rendering.

			// Render the main mesh with tessellation for the shadow map.
			mainMesh->sendData(renderer->getDeviceContext(), D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
			depthShaderTess->setShaderParametersTess(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix, camera->getPosition(), textureMgr->getTexture(L"heightMap"));
			depthShaderTess->render(renderer->getDeviceContext(), mainMesh->getIndexCount());

			// Render additional objects (sphere, cottage, and spotlight model) for the shadow map.
			sphere->sendData(renderer->getDeviceContext());
			depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * XMMatrixScaling(sphereScale.x, sphereScale.y, sphereScale.z) * XMMatrixTranslation(spherePos.x, spherePos.y, spherePos.z), lightViewMatrix, lightProjectionMatrix);
			depthShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

			cottageModel->sendData(renderer->getDeviceContext());
			depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * XMMatrixRotationX(XM_PI / 2) * XMMatrixScaling(1, .4, .75) * XMMatrixTranslation(cottagePosition.x, cottagePosition.y, cottagePosition.z), lightViewMatrix, lightProjectionMatrix);
			depthShader->render(renderer->getDeviceContext(), cottageModel->getIndexCount());

			spotlightModel->sendData(renderer->getDeviceContext());
			depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * XMMatrixScaling(0.05, 0.05, 0.05) * XMMatrixRotationX(XM_PI / 2) * XMMatrixRotationY(-XM_PI / 2) * XMMatrixTranslation(spotlightModelPosition.x, spotlightModelPosition.y, spotlightModelPosition.z), lightViewMatrix, lightProjectionMatrix);
			depthShader->render(renderer->getDeviceContext(), spotlightModel->getIndexCount());
		}
	}

	// Step 2: Reset the render target to the back buffer and restore the viewport.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

// Apply a brightness filter to the source texture and store the result in the output texture.
void BrightnessFilter(D3D* renderer, FPCamera* camera, OrthoMesh* orthoMeshBloom, BrightnessFilterShader* brightnessFilterShader, RenderTexture* renderTextureSource, RenderTexture* renderTextureBrightnessFilter) {
	// Step 1: Set the output render texture as the render target.
	// This is where the brightness-filtered texture will be stored.
	renderTextureBrightnessFilter->setRenderTarget(renderer->getDeviceContext());
	renderTextureBrightnessFilter->clearRenderTarget(renderer->getDeviceContext(), 0, 0, 0, 0);

	// Step 2: Prepare matrices for 2D rendering with orthographic projection.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();         // Standard world matrix.
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();         // Orthographic projection matrix for 2D rendering.
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();   // Orthographic view matrix for the camera.

	// Step 3: Disable Z-buffer for 2D ortho mesh rendering.
	// This ensures the mesh is rendered in full without depth testing.
	renderer->setZBuffer(false);

	// Step 4: Send the ortho mesh data to the GPU for rendering.
	orthoMeshBloom->sendData(renderer->getDeviceContext());

	// Step 5: Set shader parameters for brightness filtering.
	// Pass the world, ortho view, and projection matrices along with the source texture.
	brightnessFilterShader->setShaderParameters(
		renderer->getDeviceContext(),
		worldMatrix,
		orthoViewMatrix,
		orthoMatrix,
		renderTextureSource->getShaderResourceView() // Input texture to apply brightness filter.
	);

	// Step 6: Apply the brightness filter shader to the mesh.
	// This extracts the bright spots of the input texture.
	brightnessFilterShader->render(renderer->getDeviceContext(), orthoMeshBloom->getIndexCount());

	// Step 7: Re-enable Z-buffer for subsequent rendering passes.
	renderer->setZBuffer(true);

	// Step 8: Reset the render target to the back buffer for normal rendering.
	renderer->setBackBufferRenderTarget();
}

// Apply a Gaussian blur to the input texture and store the result in the output texture.
void GaussianBlur(D3D* renderer, FPCamera* camera, OrthoMesh* orthoMeshBloom, GaussianBlurShader* gaussianBlurShader, RenderTexture* renderTextureSource, RenderTexture* renderTextureGaussianBlur) {
	// Step 1: Set the output render texture as the render target.
	// This is where the blurred texture will be stored.
	renderTextureGaussianBlur->setRenderTarget(renderer->getDeviceContext());
	renderTextureGaussianBlur->clearRenderTarget(renderer->getDeviceContext(), 0, 0, 0, 0);

	// Step 2: Prepare the matrices for 2D rendering with orthographic projection.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();         // Standard world matrix.
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();         // Orthographic projection matrix for 2D rendering.
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();   // Orthographic view matrix for the camera.

	// Step 3: Disable Z-buffer for 2D ortho mesh rendering.
	// This ensures the mesh is rendered in full without depth testing.
	renderer->setZBuffer(false);

	// Step 4: Send the ortho mesh data to the GPU for rendering.
	orthoMeshBloom->sendData(renderer->getDeviceContext());

	// Step 5: Set shader parameters for Gaussian blur.
	// Pass the world, ortho view, and projection matrices along with the source texture and output texture size.
	gaussianBlurShader->setShaderParameters(
		renderer->getDeviceContext(),
		worldMatrix,
		orthoViewMatrix,
		orthoMatrix,
		renderTextureSource->getShaderResourceView(),               // Input texture to apply blur to.
		XMFLOAT2(renderTextureGaussianBlur->getTextureWidth(),      // Dimensions of the output texture.
			renderTextureGaussianBlur->getTextureHeight())
	);

	// Step 6: Apply the Gaussian blur shader to the mesh.
	gaussianBlurShader->render(renderer->getDeviceContext(), orthoMeshBloom->getIndexCount());

	// Step 7: Re-enable Z-buffer for subsequent rendering passes.
	renderer->setZBuffer(true);

	// Step 8: Reset the render target to the back buffer for normal rendering.
	renderer->setBackBufferRenderTarget();
}

// Blend two textures (source and bloom) to create a final image with the bloom effect applied.
void Blend(D3D* renderer, FPCamera* camera, OrthoMesh* orthoMeshBloom, BlendShader* textureBlendShader, RenderTexture* renderTextureSource, RenderTexture* renderTextureBloom, RenderTexture* renderTextureFinal) {
	// Step 1: Set the final render texture as the render target.
	// This is where the result of the blending will be stored.
	renderTextureFinal->setRenderTarget(renderer->getDeviceContext());
	renderTextureFinal->clearRenderTarget(renderer->getDeviceContext(), 0, 0, 0, 0);

	// Step 2: Prepare transformation matrices for rendering.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();           // Standard world matrix.
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();           // Orthographic projection matrix for 2D rendering.
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();     // Orthographic view matrix for the camera.

	// Step 3: Disable Z-buffer for 2D ortho mesh rendering.
	// This ensures that the mesh is rendered in full without depth testing.
	renderer->setZBuffer(false);

	// Step 4: Send the ortho mesh data to the GPU for rendering.
	orthoMeshBloom->sendData(renderer->getDeviceContext());

	// Step 5: Set shader parameters for blending.
	// Pass the world, ortho view, and projection matrices along with the textures to be blended.
	textureBlendShader->setShaderParameters(
		renderer->getDeviceContext(),
		worldMatrix,
		orthoViewMatrix,
		orthoMatrix,
		renderTextureSource->getShaderResourceView(), // The original scene texture.
		renderTextureBloom->getShaderResourceView()   // The bloom texture.
	);

	// Step 6: Render the blended output using the shader.
	// This combines the source texture and the bloom texture into the final result.
	textureBlendShader->render(renderer->getDeviceContext(), orthoMeshBloom->getIndexCount());

	// Step 7: Re-enable Z-buffer for subsequent rendering passes.
	renderer->setZBuffer(true);

	// Step 8: Reset the render target to the back buffer for normal rendering.
	renderer->setBackBufferRenderTarget();
}

// Apply the bloom effect by performing brightness filtering, downscaling, Gaussian blurring, and blending to achieve the final bloom effect.
void App1::bloomPass(RenderTexture* source, RenderTexture* blendWith, RenderTexture* output) {
	// Step 1: Apply brightness filter to the source texture.
	// This step isolates the bright regions in the scene for the bloom effect.
	BrightnessFilter(renderer, camera, orthoMeshFull, brightnessFilterShader, source, renderTexturesBloom[0]);

	// Step 2: Perform Gaussian blurring on the brightness filter output.
	// The goal is to create a soft, glowing effect around bright areas in the texture.
	// Multiple passes of Gaussian blur are applied for better results.
	GaussianBlur(renderer, camera, orthoMeshFull, gaussianBlurShader, renderTexturesBloom[0], renderTexturesBloom[1]); // First pass
	GaussianBlur(renderer, camera, orthoMeshFull, gaussianBlurShader, renderTexturesBloom[1], renderTexturesBloom[2]); // Second pass
	GaussianBlur(renderer, camera, orthoMeshFull, gaussianBlurShader, renderTexturesBloom[2], renderTexturesBloom[3]); // Third pass
	GaussianBlur(renderer, camera, orthoMeshFull, gaussianBlurShader, renderTexturesBloom[3], renderTexturesBloom[4]); // Fourth pass

	// Step 3: Blend the final blurred texture with the original texture (blendWith).
	// This combines the bloom effect with the scene to give the final result.
	Blend(renderer, camera, orthoMeshFull, blendShader, blendWith, renderTexturesBloom[4], output);
}

// Apply color grading filters (tint, brightness, contrast, saturation) to the input texture.
void App1::colorFilters(RenderTexture* source, RenderTexture* output) {
	// Step 1: Set the output render texture as the render target.
	// The processed texture will be rendered to this target.
	output->setRenderTarget(renderer->getDeviceContext());
	output->clearRenderTarget(renderer->getDeviceContext(), 0, 0, 0, 0);

	// Step 2: Prepare matrices for rendering to the ortho mesh.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();         // Standard world matrix.
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();         // Orthographic projection matrix for 2D rendering.
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();   // Orthographic view matrix for the camera.

	// Step 3: Disable Z-buffer for rendering the ortho mesh.
	// This ensures the full-screen quad is rendered without depth testing.
	renderer->setZBuffer(false);

	// Step 4: Send the vertex data of the full-screen ortho mesh to the GPU.
	orthoMeshFull->sendData(renderer->getDeviceContext());

	// Step 5: Set shader parameters for color filtering.
	// Pass the world, ortho view, and projection matrices, along with the source texture,
	// and filter parameters like tint color, brightness, contrast, and saturation.
	colorFilterShader->setShaderParameters(
		renderer->getDeviceContext(),
		worldMatrix,
		orthoViewMatrix,
		orthoMatrix,
		source->getShaderResourceView(),
		tintColor,    // RGB tint color to apply.
		tintStrength, // Intensity of the tint effect.
		brightness,   // Adjust brightness.
		contrast,     // Adjust contrast.
		saturation    // Adjust saturation.
	);

	// Step 6: Render the ortho mesh with the color filter shader.
	colorFilterShader->render(renderer->getDeviceContext(), orthoMeshFull->getIndexCount());

	// Step 7: Re-enable the Z-buffer for subsequent rendering operations.
	renderer->setZBuffer(true);

	// Step 8: Reset the render target to the back buffer.
	// This ensures subsequent rendering outputs to the screen.
	renderer->setBackBufferRenderTarget();
}

// Render the scene with bloom effects and other post-processing.
void App1::RenderBloomTexture() {
	// Step 1: Set the bloom source render texture as the render target.
	// This texture will capture the rendered scene for bloom processing.
	renderTextureSourceBloom->setRenderTarget(renderer->getDeviceContext());

	// Step 2: Clear the render target with a fully transparent black color.
	// Arguments: Red, Green, Blue, Alpha (all set to 0.0f).
	renderTextureSourceBloom->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 0.0f);

	// Step 3: Render the cloud layer.
	// This draws clouds into the render target (source for bloom effect).
	//Clouds();

	// Step 4: Handle shadow processing based on the shadow toggle.
	if (shadowBool) {
		// If shadows are enabled, capture the shadow map.
		shadowDepth();
	}
	else {
		// If shadows are disabled, clear the shadow maps for all lights.
		for (int i = 0; i < lightSize; i++) {
			shadowMaps[i]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());
			renderer->setBackBufferRenderTarget();
			renderer->resetViewport();
		}
	}

	// Step 5: Render the lighting pass.
	// This includes applying lighting effects to the scene and outputs to the render texture.
	lighting(renderTextureSourceBloom, false);

	// Step 6: Reset the render target to the back buffer.
	// This ensures subsequent rendering operations output directly to the screen.
	renderer->setBackBufferRenderTarget();
}

// Render the sun sphere using post-processing.
void App1::RenderSunSpherePP() {
	// Set the first render texture as the render target.
	// This ensures the sun sphere is rendered into this texture.
	renderTextureSunSphere[0]->setRenderTarget(renderer->getDeviceContext());

	// Clear the render target with a fully transparent black color.
	// Arguments: Red, Green, Blue, Alpha (all set to 0.0f).
	renderTextureSunSphere[0]->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 0.0f);

	// Render the actual sun sphere.
	// This will use the previously set render target.
	renderSunSphere();

	// Revert the render target to the back buffer.
	// This ensures subsequent rendering operations go to the screen.
	renderer->setBackBufferRenderTarget();
}

// Apply a multi-pass bloom effect to the sun sphere and blend it into the final scene.
void App1::BloomSunSphere() {
	// Step 1: Apply a brightness filter to isolate bright regions of the sun sphere.
	// Inputs: RenderTextureSunSphere[0] (original sun sphere)
	// Output: RenderTextureSunSphere[2] (bright areas only)
	BrightnessFilter(renderer, camera, orthoMeshFull, sunBrightnessFilterShader, renderTextureSunSphere[0], renderTextureSunSphere[2]);

	// Step 2: Perform multiple passes of Gaussian blur to soften the bright regions.
	// Each pass takes the output from the previous and blurs it further.
	for (int i = 2; i < 12; i++) {
		GaussianBlur(renderer, camera, orthoMeshFull, gaussianBlurShader, renderTextureSunSphere[i], renderTextureSunSphere[i + 1]);
	}

	// Step 3: Blend the blurred sun sphere with the color-graded scene.
	// Inputs: RenderTextureSource (source/raw scene)
	//         RenderTextureSunSphere[12] (final blurred sun sphere)
	// Output: RenderTextureSunSphere[13] (blended result)
	Blend(renderer, camera, orthoMeshFull, blendShader, renderTextureSource, renderTextureSunSphere[12], renderTextureSunSphere[13]);
}

// Perform post-processing steps including bloom, sun sphere rendering, and blending.
void App1::postProcessing() {
	// Step 1: Render the bloom texture.
	// This includes clouds, shadows, lighting, and color grading operations.
	RenderBloomTexture();

	// Step 2: Render the sun sphere with post-processing effects.
	RenderSunSpherePP();

	// Step 3: Apply bloom to the sun sphere.
	// This step enhances the sun's brightness and bloom effect.
	BloomSunSphere();

	// Step 4: Final bloom pass and blending.
	// Bloom the main scene and combine with the bloom-processed sun sphere.
	// Outputs the final result to renderTexturesBloom[5].
	bloomPass(renderTextureSourceBloom, renderTextureSunSphere[13], renderTexturesBloom[5]);

	// Step 5: Color grading the final output.
	// Applying brightness, contrast, saturation, and tinting.
	colorFilters(renderTexturesBloom[5], renderTextureColorFilters);
}

// Perform the final rendering pass, displaying the processed or raw scene onto the screen.
void App1::finalRender() {
	// Step 1: Disable the Z-buffer for rendering the ortho mesh.
	// This ensures the ortho mesh is rendered without depth testing.
	renderer->setZBuffer(false);

	// Step 2: Prepare matrices for ortho mesh rendering.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();           // Standard world matrix
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();           // Orthographic projection matrix
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();     // Orthographic view matrix

	// Step 3: Render using post-processing effects if enabled.
	if (postProcessingBool) {
		// Send the full-screen ortho mesh data to the GPU.
		orthoMeshFull->sendData(renderer->getDeviceContext());

		// Set shader parameters with the bloom-processed texture (renderTextureColorFilters).
		textureShader->setShaderParameters(
			renderer->getDeviceContext(),
			worldMatrix,
			orthoViewMatrix,
			orthoMatrix,
			renderTextureColorFilters->getShaderResourceView()
		);

		// Render the ortho mesh with the post-processed texture.
		textureShader->render(renderer->getDeviceContext(), orthoMeshFull->getIndexCount());
	}
	// Step 4: Render the raw scene if post-processing is disabled.
	else {
		// Send the full-screen ortho mesh data to the GPU.
		orthoMeshFull->sendData(renderer->getDeviceContext());

		// Set shader parameters with the raw render texture (renderTextureSource).
		textureShader->setShaderParameters(
			renderer->getDeviceContext(),
			worldMatrix,
			orthoViewMatrix,
			orthoMatrix,
			renderTextureSource->getShaderResourceView()
		);

		// Render the ortho mesh with the raw texture.
		textureShader->render(renderer->getDeviceContext(), orthoMeshFull->getIndexCount());
	}

	// Step 5: Re-enable the Z-buffer for subsequent rendering passes.
	renderer->setZBuffer(true);
}

// Renders the application's graphical user interface using ImGui.
// Cornut, O. (n.d.) Dear ImGui(1.63)[Library / Framework]. Adapted from: https://github.com/ocornut/imgui#dear-imgui.
void App1::gui() {
	// Window locations
	ImVec2 topLeft(0, 0);
	ImVec2 bottomLeft(0, ImGui::GetIO().DisplaySize.y - 200);
	ImVec2 debugPos(ImGui::GetIO().DisplaySize.x - 550, ImGui::GetIO().DisplaySize.y - 700);
	ImVec2 topRight(ImGui::GetIO().DisplaySize.x - 275, 0);
	
	// Step 1: Disable unnecessary shader stages for UI rendering.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0); // Disable Geometry Shader
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0); // Disable Hull Shader
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0); // Disable Domain Shader

	// Step 2: Set a semi-transparent background for ImGui windows.
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.25f));

	// Step 3: Wireframe Mode Toggle.
	ImGui::SetNextWindowPos(topLeft, ImGuiCond_Always);
	ImGui::Begin("Wireframe");
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	ImGui::End();

	ImGui::Begin("Perlin Params");
	ImGui::SliderFloat3("Perlin parameters", (float*)&params, -100, 100, "%.2f");
	generateMap = ImGui::Button("Generate perlin map");
	if (generateMap) {
		perlinNoiseTexture->GeneratePerlinNoiseTexture(renderer->getDevice(), textureMgr);
		generateMap = false;
		call++;
		perlinNoiseTexName = L"perlinNoiseHeightMap" + std::to_wstring(call);
	}
	smooth = ImGui::Button("Smooth Height Map");
	if (smooth) {
		perlinNoiseTexture->SmoothHeightMap(renderer->getDevice(), textureMgr);
	}
	ImGui::End();

	// Step 4: Debug Mode UI.
	if (debugBool) {
		ImGui::SetNextWindowPos(debugPos, ImGuiCond_Always);
		ImGui::Begin("Debug Window");
		ImGui::Text("Pausing time might help with seeing changes");

		// Directional Light Controls.
		if (ImGui::CollapsingHeader("Directional Light")) {
			ImGui::SliderFloat3("Position D-light", (float*)&position[0], -100, 100, "%.2f");
			ImGui::SliderFloat3("Direction D-light", (float*)&direction[0], -1, 1, "%.2f");
			ImGui::ColorEdit4("Diffuse Color D-light", (float*)&diffuse[0]);
			ImGui::SliderFloat("D-light Intensity", (float*)&intensity[0], 0, 1, "%.2f");
			ImGui::ColorEdit4("Ambient Color D-light", (float*)&ambient[0]);
			ImGui::ColorEdit4("Specular Color D-light", (float*)&specularColour[0]);
			ImGui::SliderFloat("Specular Power D-light", (float*)&specularPower[0].x, 0, 100, "%.2f");
		}

		// Spotlight Controls.
		if (ImGui::CollapsingHeader("SpotLight")) {
			ImGui::SliderFloat3("Position S-light", (float*)&position[1], -100, 100, "%.2f");
			ImGui::SliderFloat3("Direction S-light", (float*)&direction[1], -1, 1, "%.2f");
			ImGui::ColorEdit4("Diffuse Color S-light", (float*)&diffuse[1]);
		}

		// Additional Object Controls.
		if (ImGui::CollapsingHeader("Cottage")) {
			ImGui::SliderFloat3("Position Cottage", (float*)&cottagePosition, -100, 100, "%.2f");
		}
		if (ImGui::CollapsingHeader("Sphere")) {
			ImGui::SliderFloat3("Position Sphere", (float*)&spherePos, -100, 100, "%.2f");
		}
		if (ImGui::CollapsingHeader("Spotlight Model")) {
			ImGui::SliderFloat3("Position Spotlight Model", (float*)&spotlightModelPosition, -100, 100, "%.2f");
		}

		// Time Controls.
		if (ImGui::CollapsingHeader("Time Scale")) {
			ImGui::SliderFloat("Time Scale", &timeScale, 1, 25, "%.2f");
		}
		ImGui::End();
	}

	// Step 5: Update Lights Data.
	UpdateLights();

	// Step 6: Main UI Elements.

	// Statistics Window.
	ImGui::SetNextWindowPos(topRight, ImGuiCond_Always);
	ImGui::Begin("Statistics");
	ImGui::Text("Frames Per Second (FPS): %.4f", timer->getFPS());
	ImGui::Text("Frame Time: %.4f ms/frame", 1000.f / timer->getFPS());
	ImGui::End();

	// Main Window.
	ImGui::SetNextWindowPos(bottomLeft, ImGuiCond_FirstUseEver);
	ImGui::Begin("Main Window");

	// Time Section.
	if (ImGui::CollapsingHeader("Time")) {
		ImGui::Indent();
		ImGui::Text("Time (out of 24): %.2f\nTime (Elapsed Time): %.2f", (elapsedTime / dayDuration) * 24, elapsedTime);
		ImGui::Unindent();
	}

	// Lights Section.
	if (ImGui::CollapsingHeader("Lights")) {
		ImGui::Indent();

		// Sun Data.
		if (ImGui::CollapsingHeader("The Sun")) {
			ImGui::Text("Sun's Colour");
			ImGui::ColorButton("Sun Colour", ImVec4(sunColor.x, sunColor.y, sunColor.z, 1.f));
			ImGui::Text("Sun's Position: X: %.4f, Y: %.4f, Z: %.4f", position[0].x, position[0].y, position[0].z);
			ImGui::Text("Sunlight's Intensity: [0.2, 1]: %.4f", intensity[0]);
			ImGui::Text("Sunlight's Direction: X: %.4f, Y: %.4f, Z: %.4f", direction[0].x, direction[0].y, direction[0].z);
			ImGui::Text("Sunlight's Diffuse Colour");
			ImGui::ColorButton("Sunlight's Diffuse Colour", ImVec4(diffuse[0].x, diffuse[0].y, diffuse[0].z, 1.0f));
			ImGui::Text("Sunlight's Ambient Colour");
			ImGui::ColorButton("Sunlight's Ambient Colour", ImVec4(ambient[0].x, ambient[0].y, ambient[0].z, 1.0f));
			ImGui::Text("Sunlight's Specular Colour");
			ImGui::ColorButton("Sunlight's Specular Colour", ImVec4(specularColour[0].x, specularColour[0].y, specularColour[0].z, 1.0f));
			ImGui::Text("Sunlight's Specular Power: %.0f", specularPower[0].x);
		}

		// Spotlight Data.
		if (ImGui::CollapsingHeader("The Spotlight")) {
			ImGui::Text("Spotlight's Position: X: %.4f, Y: %.4f, Z: %.4f", position[1].x, position[1].y, position[1].z);
			ImGui::Text("Spotlight's Direction: X: %.4f, Y: %.4f, Z: %.4f", direction[1].x, direction[1].y, direction[1].z);
			ImGui::Text("Spotlight's Diffuse Colour");
			ImGui::ColorButton("Spotlight's Diffuse Colour", ImVec4(diffuse[1].x, diffuse[1].y, diffuse[1].z, 1.0f));
		}
		ImGui::Unindent();
	}

	// Player Data.
	if (ImGui::CollapsingHeader("Player")) {
		ImGui::Indent();
		ImGui::Text("Player's Position: X: %.4f, Y: %.4f, Z: %.4f", camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);
		ImGui::Text("Player's Rotation: X: %.4f, Y: %.4f, Z: %.4f", camera->getRotation().x, camera->getRotation().y, camera->getRotation().z);
		ImGui::Unindent();
	}

	// Scene Enhancements Section.
	if (ImGui::CollapsingHeader("Scene Enhancements")) {
		ImGui::Indent();
		ImGui::Checkbox("Shadows?", &shadowBool);
		ImGui::Checkbox("Post-Processing?", &postProcessingBool);
		ImGui::Checkbox("Time?", &timeBool);
		ImGui::Unindent();
	}

	// Reset Time Section.
	if (ImGui::CollapsingHeader("Reset Time")) {
		ImGui::Indent();
		ImGui::Checkbox("Reset?", &resetBool);
		ImGui::Unindent();
	}

	// Debug Mode Toggle.
	if (ImGui::CollapsingHeader("Debug Mode")) {
		ImGui::Indent();
		ImGui::Checkbox("Debug?", &debugBool);
		ImGui::Unindent();
	}

	// Credits Section.
	if (ImGui::CollapsingHeader("Credits")) {
		ImGui::Text("Made by:\n\nKushagra\nMSc Computer Games Technology\n2400020");
	}

	ImGui::PopStyleColor(); // Reset window background transparency.
	ImGui::End(); // End of Main Window.

	// Step 7: Render the ImGui UI.
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

// Update the properties of all lights in the scene dynamically.
void App1::UpdateLights() {
	// Clamp intensity values to ensure they stay within the valid range [0.0, 1.0].
	for (int i = 0; i < lightSize; i++) {
		intensity[i] = std::clamp(intensity[i], 0.0f, 1.0f);
	}

	// Loop through all lights and update their properties.
	for (int i = 0; i < lightSize; i++) {
		// Update position.
		light[i]->setPosition(position[i].x, position[i].y, position[i].z);

		// Update direction.
		light[i]->setDirection(direction[i].x, direction[i].y, direction[i].z);

		// Update diffuse color scaled by intensity.
		light[i]->setDiffuseColour(
			intensity[i] * diffuse[i].x,
			intensity[i] * diffuse[i].y,
			intensity[i] * diffuse[i].z,
			intensity[i] * diffuse[i].w
		);

		// Update ambient color.
		light[i]->setAmbientColour(ambient[i].x, ambient[i].y, ambient[i].z, ambient[i].w);

		// Update specular color and power for directional light.
		if (i == 0) {
			light[i]->setSpecularColour(specularColour[i].x, specularColour[i].y, specularColour[i].z, specularColour[i].w);
			light[i]->setSpecularPower(specularPower[i].x);
		}
	}
}