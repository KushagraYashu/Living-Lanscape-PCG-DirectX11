#include "App1.h"

// Perlin noise height map texture parameters (.x - frequency, .y - amplitude)
XMFLOAT2 paramsHM = XMFLOAT2(3.6969f, 6.9f);
float paramsDMFreq = 0.1;

// Screen-Related Variables
int screenWidthVar, screenHeightVar;  // Holds the width and height of the screen for rendering
float aspectRatio;  // Stores the aspect ratio of the screen for correct projection
float fieldOfView = XM_PI / 4;  // Field of view for the camera, set to 45 degrees by default

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
XMFLOAT3 sphereScale = XMFLOAT3(.25, .25, .25);  // Scaling factor for the sphere
XMFLOAT3 cottagePosition = XMFLOAT3(22, 20, 20);  // Position of the cottage
XMFLOAT2 cottageGridPos = XMFLOAT2(cottagePosition.x - 1, cottagePosition.z - 1); // Position of the cottage on the terrain grid (X-Z plane)
XMFLOAT3 spotlightModelPosition = XMFLOAT3(cottagePosition.x, 20, cottagePosition.z + 2);  // Position of the spotlight model
XMFLOAT2 spotlightGridPos = XMFLOAT2(spotlightModelPosition.x - 1, spotlightModelPosition.z - 1); // Position of the spotlight model on the terrain grid (X-Z plane)
XMFLOAT3 sceneCentre = XMFLOAT3(25, 25, 25);  // The center of the scene, used for light positioning
XMFLOAT3 cloudBoxSize = XMFLOAT3(100, 50, 100); // The size of the volumetric cloud box
XMFLOAT3 cloudBoxPosition = XMFLOAT3(); // Position of the cloud box

// Gameplay variables
XMFLOAT2 coinPositionsXZ[5] = {					// Positions for various coins
	XMFLOAT2(35.6, 27.3),
	XMFLOAT2(42.45, 14.7),
	XMFLOAT2(28.7, 11.98),
	XMFLOAT2(13.75, 13.19),
	XMFLOAT2(11.61, 33.58)
};
bool coinCollected[5] = {						// Coin collection status
	false,
	false,
	false,
	false,
	false
};
bool allCollected = false;						// Flag to check if all coins have been collected		
bool gameFinish = false;						// Flag to check if the game is finished
int totCoins = 5;								// Total number of coins in the scene
int colCoins = 0;								// Number of coins collected

// Cloud related variables
XMFLOAT4 gasColor = XMFLOAT4(1, 1, 1, 1);		// cloud gas color
float sampleNumbers = 200;						// sample numbers for raymarching
float sigma_a = 0.5f;							// sigma_a of the gas (the higher the value, the thicker the gas)
float g = 0.25f;								// Phase function isotropy parameter
float gasDensity = 0.055f;						// density of the gas
float speedX = 0;								// cloud speed in X
float speedY = 0.02;							// clouds speed in Y

// Texturing related variables
XMFLOAT2 grassTexVals = XMFLOAT2(-.5, .2);		// height control values for grass texture
XMFLOAT2 rockTextVals = XMFLOAT2(-.2, 2);		// height control values for rock texture
XMFLOAT2 snowTexVals = XMFLOAT2(1, 5);			// height control values for snow texture

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
bool smooth = false;  // Enables or disables terrain smoothing
bool camFlightMode = false;  // Enables or disables camera flight mode
bool generateHM = false;  // Flag for generating a new terrain map
bool generateDM = false; // Flag for generating a new density map
bool gravity = true; // Flag for gravity

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
	// The textures (grass, rock, and snow) are sourced from Google Images.
	textureMgr->loadTexture(L"Grass Tex", L"res/grassTex.png");	//grass texture for height based painting
	textureMgr->loadTexture(L"Rock Tex", L"res/rockTex.png"); 	//rock texture for height based painting
	textureMgr->loadTexture(L"Snow Tex", L"res/snowTex.png");	//snow texture for height based painting
	// Coin model and texture sourced from https://sketchfab.com/3d-models/stylized-coin-8cd6f95c44994ed5944a42892d0ffc10
	textureMgr->loadTexture(L"Coin Tex", L"res/coinTex.jpg");	//coin texture for coin models
	// Spotlight model and texture sourced from https://www.turbosquid.com/3d-models/free-street-lamp-3d-model/794502
	textureMgr->loadTexture(L"spotlight", L"res/Street_Lamp_DM.dds"); // Load spotlight texture.
	// Cottage is from lab assets (Term 1 CMP502)
	textureMgr->loadTexture(L"cottage", L"res/cottage_diffuse_1.dds"); // Load cottage texture.
	// Sun texture sourced from https://it.pinterest.com/pin/417357090443735050/
	textureMgr->loadTexture(L"sunTex", L"res/sunTex.jpg"); // Load sun texture.

	// Step 4: Initialize mesh objects.
	// Create meshes used in the scene (plane, sphere, models).
	mainMesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext(), 50); // Plane mesh.
	volumetricCloudBox = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext()); // Box mesh for volumetric clouds.
	spotlightModel = new AModel(renderer->getDevice(), "res/models/Street_Lamp.FBX"); // Spotlight model.
	cottageModel = new AModel(renderer->getDevice(), "res/models/cottage_fbx.fbx"); // Cottage model.
	coinModel = new AModel(renderer->getDevice(), "res/coin.fbx"); // Coin model.
	skyDome = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext()); // Sky dome class for the background.
	cloudsPlane = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext(), 1000); // Clouds plane.
	sunSphere = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext(), 10); // Sun sphere.

	// Step 5: Initialize render textures.
	// Set up multiple render textures for various purposes (rendering to a texture for bloom, sun sphere, etc.).
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
		renderTextureClouds = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
		renderTextureCloudBlended = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	
	// Step 6: Initialize ortho mesh (for rendering textures over).
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
	// Noon colors. (reference values)
	/*skyDome->SetCenterColor(XMFLOAT4(0.9607843137254902, 0.8470588235294118, 0.7294117647058823, 1));
	skyDome->SetApexColor(XMFLOAT4(0.615686274509804, 0.6392156862745098, 0.7294117647058823, 1));
	tintColor = XMFLOAT3(1.0, 1.0, 0.9);
	tintStrength = 0.04;
	brightness = 1.4;
	contrast = 1.015;
	saturation = 1.35;
	sunColor = XMFLOAT4(1.0, 1.0, 0.9, 1.0);*/
	// Sunset colors. (reference values)
	/*skyDome->SetCenterColor(XMFLOAT4(0.9568627450980393, 0.2549019607843137, 0.24705882352941178, 1));
	skyDome->SetApexColor(XMFLOAT4(0.12941176470588237, 0.043137254901960784, 0.33725490196078434, 1));
	tintColor = XMFLOAT3(1.0, 0.5, 0.3);
	tintStrength = .06;
	brightness = 1.5;
	contrast = 1.02;
	saturation = 1.35;
	sunColor = XMFLOAT4(1.0, 0.2, 0.2, 1.0);*/
	// Night colors. (reference values)
	/*skyDome->SetCenterColor(XMFLOAT4(0, 0.011764705882352941, 0.019607843137254902, 1));
	skyDome->SetApexColor(XMFLOAT4(0, 0.13725490196078433/3, 0.23137254901960785/3, 1));
	tintColor = XMFLOAT3(0.2, 0.2, 0.8);
	tintStrength = .07;
	brightness = 1.5;
	contrast = 1.02;
	saturation = 1.35;*/

	// Step 8: Initialize shaders.
	// Create instances of different shaders used for rendering (vertex manipulation, depth rendering, lighting, etc.).
	linearDepthShaderTess = new DepthShader(renderer->getDevice(), hwnd, L"VertexManipulation_vs.cso", L"VertexManipulation_hs.cso", L"VertexManipulation_ds.cso", L"linearDepth_ps.cso");
	linearDepthShader = new DepthShader(renderer->getDevice(), hwnd, L"depth_vs.cso", L"linearDepth_ps.cso");
	depthShaderTess = new DepthShader(renderer->getDevice(), hwnd, L"VertexManipulation_vs.cso", L"VertexManipulation_hs.cso", L"VertexManipulation_ds.cso", L"depth_ps.cso"); // Tessellated depth shader.
	lightShaderTess = new LightShader(renderer->getDevice(), hwnd, L"VertexManipulation_vs.cso", L"VertexManipulation_hs.cso", L"VertexManipulation_ds.cso", L"light_ps.cso"); // Light shader for tessellated objects.
	lightShader = new LightShader(renderer->getDevice(), hwnd, L"light_vs.cso", L"lightNonTess_ps.cso"); // Light shader for non-tessellated objects.
	depthShader = new DepthShader(renderer->getDevice(), hwnd, L"depth_vs.cso", L"depth_ps.cso"); // Depth shader for shadow mapping.
	textureShader = new TextureShader(renderer->getDevice(), hwnd); // Texture shader for basic rendering.
	skyDomeShader = new SkyDomeShaderClass(renderer->getDevice(), hwnd, L"SkyDomeShader_vs.cso", L"SkyDomeShader_ps.cso"); // SkyDome shader
	cloudsShader = new CloudsShader(renderer->getDevice(), hwnd, L"CloudsShader_vs.cso", L"CloudsShader_ps.cso"); // Volumetric clouds shader
	brightnessFilterShader = new BrightnessFilterShader(renderer->getDevice(), hwnd, L"texture_vs.cso", L"BrightnessFilterShader_ps.cso"); // Brightness filter shader
	sunBrightnessFilterShader = new BrightnessFilterShader(renderer->getDevice(), hwnd, L"texture_vs.cso", L"SunBrightnessFilterShader_ps.cso"); // Sun brightness filter shader
	gaussianBlurShader = new GaussianBlurShader(renderer->getDevice(), hwnd, L"texture_vs.cso", L"GaussianBlurShader_ps.cso"); // Gaussian blur shader
	blendShader = new BlendShader(renderer->getDevice(), hwnd, L"texture_vs.cso", L"BlendShader_ps.cso");
	cloudBlendShader = new BlendShader(renderer->getDevice(), hwnd, L"texture_vs.cso", L"CloudBlendShader_ps.cso"); // Clouds blend shader
	colorFilterShader = new ColorGradingShader(renderer->getDevice(), hwnd, L"texture_vs.cso", L"ColorGradingShader_ps.cso"); // Color grading shader
	sunShader = new SunShader(renderer->getDevice(), hwnd, L"texture_vs.cso", L"SunShader_ps.cso"); // Sun rendering shader

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
	depthTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);

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
			direction[i] = XMFLOAT4(0.f, -1.f, .0f, 0.f);
			position[i] = XMFLOAT4(25.f, 14.65, 26.7, 0.f);
			diffuse[i] = XMFLOAT4(1.f, 0.f, 0.f, 1.f);
			light[i]->setDirection(direction[i].x, direction[i].y, direction[i].z);
			light[i]->setPosition(position[i].x, position[i].y, position[i].z);
			light[i]->setDiffuseColour(intensity[1] * diffuse[i].x, intensity[1] * diffuse[i].y, intensity[1] * diffuse[i].z, 1.f);
			light[i]->generateProjectionMatrix(SCREEN_NEAR, SCREEN_DEPTH);
		}
	}

	// Step 11: Generate Perlin Noise textures (Density and Height map).
	perlinNoiseTexture = new PerlinNoiseTexture(50, cloudBoxSize.x, cloudBoxSize.y, cloudBoxSize.z); // Initialising the generator with terrain size and required references.
	perlinNoiseTexture->GeneratePerlinNoiseTextureDM(renderer->getDevice(), textureMgr, paramsDMFreq); // Generate 3D density texture for volumetric clouds.
	perlinNoiseTexture->SmoothHeightMap(renderer->getDevice(), textureMgr); // Smooth/Reset the initial values for Height map
	perlinNoiseTexture->GeneratePerlinNoiseTextureHM(renderer->getDevice(), textureMgr, paramsHM.x, paramsHM.y); // Generate height map for the terrain
	for (int i = 0; i < 2; i++) {	// Multiple smooth calls for the desired effect
		perlinNoiseTexture->SmoothHeightMap(renderer->getDevice(), textureMgr);
	}

	// Step 12: Initialise camera variables.
	camera->noiseData = perlinNoiseTexture->GetHeightDataRaw(); // Set the height data in Camera class for collision detection and camera movement.
	camera->size = perlinNoiseTexture->GetTerrainSize(); // Set the size of the terrain in Camera class.
	camera->flightMode = false; // Set flight mode to false.
	camera->setPosition(22, 6, 23); // Set initial Position.
}

// Destructor for the App1 class, handles cleanup of allocated resources.
App1::~App1()
{
	// Step 1: Base class cleanup
	BaseApplication::~BaseApplication();

	// Step 2: Clean up shaders
	SAFE_DELETE(linearDepthShaderTess);
	SAFE_DELETE(linearDepthShader);
	SAFE_DELETE(depthShaderTess);
	SAFE_DELETE(depthShader);
	SAFE_DELETE(lightShaderTess);
	SAFE_DELETE(lightShader);
	SAFE_DELETE(textureShader);
	SAFE_DELETE(skyDomeShader);
	SAFE_DELETE(cloudsShader);
	SAFE_DELETE(brightnessFilterShader);
	SAFE_DELETE(sunBrightnessFilterShader);
	SAFE_DELETE(gaussianBlurShader);
	SAFE_DELETE(blendShader);
	SAFE_DELETE(cloudBlendShader);
	SAFE_DELETE(colorFilterShader);
	SAFE_DELETE(sunShader);

	// Step 3: Clean up meshes
	SAFE_DELETE(mainMesh);
	SAFE_DELETE(cloudsPlane);
	SAFE_DELETE(volumetricCloudBox);
	SAFE_DELETE(spotlightModel);
	SAFE_DELETE(cottageModel);
	SAFE_DELETE(coinModel);
	SAFE_DELETE(skyDome);
	SAFE_DELETE(sunSphere);

	// Step 4: Clean up render textures
	SAFE_DELETE(renderTextureSource);
	SAFE_DELETE(renderTextureSourceBloom);
	for (int i = 0; i < 14; i++) {
		SAFE_DELETE(renderTextureSunSphere[i]);
	}
	for (int i = 0; i < 6; i++) {
		SAFE_DELETE(renderTexturesBloom[i]);
	}
	SAFE_DELETE(renderTextureColorFilters);
	SAFE_DELETE(renderTextureClouds);
	SAFE_DELETE(renderTextureCloudBlended);
	SAFE_DELETE(depthTexture);

	// Step 5: Clean up ortho meshes
	SAFE_DELETE(orthoMeshFull);
	SAFE_DELETE(debugOrthoMesh[0]);
	SAFE_DELETE(debugOrthoMesh[1]);

	// Step 6: Clean up light objects
	for (int i = 0; i < lightSize; i++) {
		SAFE_DELETE(light[i]);
	}

	// Step 7: Clean up shadow maps
	for (int i = 0; i < lightSize; i++) {
		SAFE_DELETE(shadowMaps[i]);
	}

	// Step 8: Clean up noise texture generator
	SAFE_DELETE(perlinNoiseTexture);
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

void App1::UpdatePositions() {
	// Step 1: Update time and camera positions for further calculations.
	timeFloat += timer->getTime();
	camPos = camera->getPosition();

	if (gravity) { // Checking if gravity is enabled

		// Step 2: Update grid positions for objects and fetch the height value at it from the perlin texture.
		cottageGridPos = XMFLOAT2((int)(cottagePosition.x - 1), (int)(cottagePosition.z - 1)); // Grid position for cottage
		float heightValueAtCottage = (perlinNoiseTexture->GetHeightAt(cottageGridPos.x, cottageGridPos.y)); // Height value at cottage grid position
		spotlightGridPos = XMFLOAT2(spotlightModelPosition.x - 1, spotlightModelPosition.z - 1); // Grid position for spotlight model
		float heightValueAtSpotlight = (perlinNoiseTexture->GetHeightAt(spotlightGridPos.x, spotlightGridPos.y)); // Height value at spotlight model grid position

		// Step 3: Update the positions of the cottage and spotlight model based on the height values and artificial gravity.
		if (cottagePosition.y > heightValueAtCottage + 0.4f) {
			cottagePosition.y -= 0.075f; // Artificial gravity effect
		}
		else {
			cottagePosition.y = heightValueAtCottage + 0.4f; // Snapping to the ground (0.4 offset is due to the model's height)
		}

		if (spotlightModelPosition.y > heightValueAtSpotlight) {
			spotlightModelPosition.y -= 0.075f; // Artificial gravity effect
		}
		else {
			spotlightModelPosition.y = heightValueAtSpotlight; // Snapping to the ground
		}
	}
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
	renderer->beginScene(0, 0, 0, 1); // Begin a new frame, setting background color to black.

	// Step 3: Set up the render texture and clear it for the initial render.
	renderTextureSource->setRenderTarget(renderer->getDeviceContext()); // Set render target to renderTextureSource.
	renderTextureSource->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f); // Clear with black.

	// Step 4: Render the SkyBox (background) first.
	SkyBox(); // Call SkyBox function to render the sky before anything else (background effect).

	// Step 5: Update positions of objects in the scene.
	UpdatePositions();

	// Step 6: Optionally, enable shadow depth rendering.
	if (shadowBool) {
		shadowDepth(); // Call shadow depth function to generate shadow maps if shadowBool is true.
	}
	else {
		for (int i = 0; i < lightSize; i++) {
			shadowMaps[i]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext()); // Unbind shadow maps.
			renderer->setBackBufferRenderTarget(); // Reset to back buffer.
			renderer->resetViewport(); // Reset viewport for the next render.
		}
	}

	// Step 7: Set wireframe mode if wireframeToggle is enabled.
	renderer->setWireframeMode(wireframeToggle);

	// Step 8: Render the lighting effects (shaders and light sources).
	lighting(renderTextureSource, false); // Apply lighting effects to the renderTextureSource.

	// Step 9: Render the sun sphere in the scene.
	renderSunSphere(); // Render the sun in the scene (could be part of a lighting effect).

	// Step 10: Turn off wireframe mode after main rendering.
	renderer->setWireframeMode(false);

	// Step 11: Render the clouds and blend with the past render calls.
	Clouds(renderTextureSource);

	// Step 12: Reset the render target to the back buffer.
	renderer->setBackBufferRenderTarget(); // Reset render target to back buffer for final scene rendering.

	// Step 13: Apply post-processing effects if enabled.
	if (postProcessingBool) {
		postProcessing(); // Apply post-processing effects such as bloom, color grading, etc.
	}

	// Step 14: Perform final rendering (combine all passes, add effects).
	finalRender(renderTextureCloudBlended); // Final scene rendering onto an Ortho Mesh.

	// Step 15: Render the graphical user interface (GUI) elements.
	gui(); // Render the GUI on top of the scene (HUD, menus, etc.).

	// Step 16: Present the rendered scene to the screen.
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

// Renders volumetric clouds in the scene, updates cloud movement, and blend this cloud texture with the existing render.
void App1::Clouds(RenderTexture* renderTextureSource) {
	// Step 1: Prepare transformation matrices for the clouds.
	XMMATRIX worldMatrix = renderer->getWorldMatrix(); // World matrix for the clouds.
	XMMATRIX viewMatrix = camera->getViewMatrix();     // Camera's view matrix.
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix(); // Projection matrix for rendering.
	XMMATRIX camProjectionMatrix = camera->getProjectionMatrix(fieldOfView, SCREEN_NEAR, SCREEN_DEPTH, aspectRatio);

	// Step 2: Capture the linear depth of the scene objects
	depthTexture->setRenderTarget(renderer->getDeviceContext());
	depthTexture->clearRenderTarget(renderer->getDeviceContext(), 1, 1, 1, 1);
	// Main mesh
	mainMesh->sendData(renderer->getDeviceContext(), D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	linearDepthShaderTess->setShaderParametersLinearDepthTess(renderer->getDeviceContext(), worldMatrix, viewMatrix, camProjectionMatrix, camera->getPosition(), textureMgr->getTexture(L"perlinNoiseHeightMap"));
	linearDepthShaderTess->render(renderer->getDeviceContext(), mainMesh->getIndexCount());
	// Cottage
	cottageModel->sendData(renderer->getDeviceContext());
	linearDepthShader->setShaderParametersLinearDepth(renderer->getDeviceContext(), worldMatrix * XMMatrixRotationX(XM_PI / 2) * XMMatrixScaling(1, .4, .75) * XMMatrixTranslation(cottagePosition.x, cottagePosition.y, cottagePosition.z), viewMatrix, camProjectionMatrix, camera->getPosition());
	linearDepthShader->render(renderer->getDeviceContext(), cottageModel->getIndexCount());
	// Sportlight model
	spotlightModel->sendData(renderer->getDeviceContext());
	linearDepthShader->setShaderParametersLinearDepth(renderer->getDeviceContext(), worldMatrix * XMMatrixScaling(0.05, 0.05, 0.05) * XMMatrixRotationX(XM_PI / 2) * XMMatrixRotationY(-XM_PI / 2) * XMMatrixTranslation(spotlightModelPosition.x, spotlightModelPosition.y, spotlightModelPosition.z), viewMatrix, camProjectionMatrix, camera->getPosition());
	linearDepthShader->render(renderer->getDeviceContext(), spotlightModel->getIndexCount());

	// Step 3: Set the render target to the cloud texture.
	renderTextureClouds->setRenderTarget(renderer->getDeviceContext());
	renderTextureClouds->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f); // Clear the render target.

	// Step 4: Retrieve and update the camera's position for correct cloud positioning.
	XMFLOAT3 camPosition;
	camera->update(); // Update the camera position and rotation.
	camPosition = camera->getPosition(); // Retrieve the updated camera position.

	// Step 5: Adjust the position of the clouds.
	cloudBoxPosition = XMFLOAT3(25, 75, 25);

	// Step 6: Change culling mode
	renderer->setFaceCulling(D3D11_CULL_NONE); // Disable culling so that both sides of the clouds are visible.

	// Step 7: Send the cloud box data to the GPU for rendering.
	volumetricCloudBox->sendData(renderer->getDeviceContext());
	cloudsShader->setShaderParameters(
		renderer->getDeviceContext(),
		worldMatrix * XMMatrixScaling(cloudBoxSize.x, cloudBoxSize.y, cloudBoxSize.z) * XMMatrixTranslation(cloudBoxPosition.x, cloudBoxPosition.y, cloudBoxPosition.z),  // Position the clouds correctly.
		viewMatrix,               // Camera view for proper positioning.
		projectionMatrix,         // Project the clouds into 3D space.
		textureMgr->getTexture(L"densityVolumeTexture"), // 3D density texture for volumetric clouds.
		depthTexture->getShaderResourceView(),
		camera->getPosition(), // Camera position for volumetric calculations.
		cloudBoxPosition, // Cloud box position for volumetric calculations.
		cloudBoxSize, // Cloud box size for volumetric calculations.
		light[0]->getDirection(), // Light direction (limited to directional light) for volumetric calculations.
		light[0]->getDiffuseColour(), // Light color for volumetric calculations.
		.25f,				// sigma s value for the light. (higher the value, the thicker the gas)
		XMFLOAT2(speedX, speedY), // cloud movement speed in UV axis.
		timeFloat, // Time for cloud movement.
		gasColor,
		XMFLOAT3(sigma_a, sampleNumbers, g),
		gasDensity
	);
	cloudsShader->render(renderer->getDeviceContext(), volumetricCloudBox->getIndexCount());

	// Step 8: Set the render target to the blended cloud texture.
	renderTextureCloudBlended->setRenderTarget(renderer->getDeviceContext());
	renderTextureCloudBlended->clearRenderTarget(renderer->getDeviceContext(), 0, 0, 0, 1); // Clear the render target.

	// Step 9: Blend the cloud texture with source texture and write onto the cloud blend texture.
	orthoMeshFull->sendData(renderer->getDeviceContext());
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getOrthoViewMatrix();
	projectionMatrix = renderer->getOrthoMatrix();
	cloudBlendShader->setShaderParameters(
		renderer->getDeviceContext(),
		worldMatrix,
		viewMatrix,
		projectionMatrix,
		renderTextureSource->getShaderResourceView(),
		renderTextureClouds->getShaderResourceView()
	);
	cloudBlendShader->render(renderer->getDeviceContext(), orthoMeshFull->getIndexCount());

	// Step 10: Reset the culling mode back
	renderer->setFaceCulling(D3D11_CULL_BACK);
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
static float lerp(const float& a, const float& b, float t) {
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
		float minVal = 0.0f;    // Minimum intensity at sunrise
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
			textureMgr->getTexture(L"Grass Tex"), // Grass texture for the terrain.
		    textureMgr->getTexture(L"Rock Tex"), // Rock texture for terrain.
		    textureMgr->getTexture(L"Snow Tex"), // Snow texture for terrain.
		    grassTexVals, // Grass values for height based shading.
		    rockTextVals,  // Rock values for height based shading.
		    snowTexVals,     // Snow values for height based shading.
			light,              // Lights to be applied.
			lightType,          // Light types (e.g., directional, spotlight).
			camera->getPosition(), // Camera position for lighting calculations.
			shadowMapsRSV       // Shadow maps for all lights.
		);
	lightShaderTess->render(renderer->getDeviceContext(), mainMesh->getIndexCount()); // Render with tessellated shader.

	// Step 6: Render additional objects (cottage, coins, spotlight model) with the lighting shader and check for gameplay logic.
	// Gameplay logic: check if all coins are collected and if the player is near the cottage.
	if (allCollected && !gameFinish) {
		camera->update();
		XMFLOAT3 playerPos = camera->getPosition();
		XMVECTOR pPos = XMLoadFloat3(&playerPos);
		XMVECTOR cPos = XMLoadFloat3(&cottagePosition);
		XMVECTOR diff = XMVectorSubtract(cPos, pPos);
		XMVECTOR length = XMVector3Length(diff);
		float distance;
		XMStoreFloat(&distance, length);
		if (distance < 4) {
			gameFinish = true;
		}
	}
	// Render cottage model.
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
	// Render spotlight model.
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
	// Render the coins based on their collection state and positions.
	for (int i = 0; i < 5; i++) {
		camera->update();
		XMFLOAT3 playerPos = camera->getPosition();
		float height = perlinNoiseTexture->GetHeightAt((int)(coinPositionsXZ[i].x - 1), (int)(coinPositionsXZ[i].y - 1));
		XMFLOAT3 coinPos = XMFLOAT3(coinPositionsXZ[i].x, height, coinPositionsXZ[i].y);
		XMVECTOR pPos = XMLoadFloat3(&playerPos);
		XMVECTOR cPos = XMLoadFloat3(&coinPos);
		XMVECTOR diff = XMVectorSubtract(cPos, pPos);
		XMVECTOR length = XMVector3Length(diff);
		float distance;
		XMStoreFloat(&distance, length);
		if (distance < 4 && !coinCollected[i]) {
			coinCollected[i] = true;
			colCoins++;
			if (colCoins >= totCoins) {
				allCollected = true;
			}
		}

		if (!coinCollected[i]) {
			coinModel->sendData(renderer->getDeviceContext());
			textureShader->setShaderParameters(
				renderer->getDeviceContext(),
				worldMatrix * XMMatrixRotationY(2 * timeFloat) * XMMatrixTranslation(coinPositionsXZ[i].x, height, coinPositionsXZ[i].y),
				viewMatrix,
				projectionMatrix,
				textureMgr->getTexture(L"Coin Tex") // Spotlight texture.
			);
			textureShader->render(renderer->getDeviceContext(), coinModel->getIndexCount()); // Render coin model.
		}
	}
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
			depthShaderTess->setShaderParametersTess(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightOrthoMatrix, camera->getPosition(), textureMgr->getTexture(L"perlinNoiseHeightMap"));
			depthShaderTess->render(renderer->getDeviceContext(), mainMesh->getIndexCount());

			// Render additional objects like the cottage, and spotlight model.
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
			depthShaderTess->setShaderParametersTess(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix, camera->getPosition(), textureMgr->getTexture(L"perlinNoiseHeightMap"));
			depthShaderTess->render(renderer->getDeviceContext(), mainMesh->getIndexCount());

			// Render additional objects (cottage, and spotlight model) for the shadow map.
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
	// Inputs: renderTextureCloudBlended (source/raw scene)
	//         RenderTextureSunSphere[12] (final blurred sun sphere)
	// Output: RenderTextureSunSphere[13] (blended result)
	Blend(renderer, camera, orthoMeshFull, blendShader, renderTextureCloudBlended, renderTextureSunSphere[12], renderTextureSunSphere[13]);
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
void App1::finalRender(RenderTexture* renderTexture) {
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

		// Set shader parameters with the raw render texture (passed argument).
		textureShader->setShaderParameters(
			renderer->getDeviceContext(),
			worldMatrix,
			orthoViewMatrix,
			orthoMatrix,
			renderTexture->getShaderResourceView()
		);

		// Render the ortho mesh with the raw texture.
		textureShader->render(renderer->getDeviceContext(), orthoMeshFull->getIndexCount());
	}

	// Step 5: Re-enable the Z-buffer for subsequent rendering passes, if any.
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

	// Step 4: Gameplay insttruction window
	ImGui::SetNextWindowPos(ImVec2(topLeft.x + 150, topLeft.y + 150), ImGuiCond_Always);
	ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoCollapse);
	if (!allCollected) {
		ImGui::Text("Use \"W A S D\" to Move!\nHold RMB and use mouse to Look Around!");
		ImGui::Text("Collect all the coins: %d/%d", colCoins, totCoins);
	}
	else if (!gameFinish) {
		ImGui::Text("Use \"W A S D\" to Move!\nHold RMB and use mouse to Look Around!");
		ImGui::Text("All coins are collected, now go to the House.");
	}
	else {
		ImGui::Text("Game finished!\nEnjoy the scenery.");
	}
	ImGui::End();

	// Step 5: Debug Mode UI.
	if (debugBool) {
		ImGui::SetNextWindowPos(debugPos, ImGuiCond_Always);
		ImGui::Begin("Debug Window");
		ImGui::Text("Pausing time or gravity might help with seeing changes");

		// Cloud Controls
		if (ImGui::CollapsingHeader("Clouds Controls")) {
			XMFLOAT2 speed = XMFLOAT2(speedX, speedY);
			ImGui::SliderFloat2("Cloud Speed (XY)", (float*)&speed, -10, 10, "%.3f");
			speedX = speed.x; speedY = speed.y;
			ImGui::SliderFloat("Gas Density", (float*)&gasDensity, 0, 1, "%.4f");
			ImGui::SliderFloat("Raymarching Sample Nos", (float*)&sampleNumbers, 0, 1000, "%.0f");
			ImGui::SliderFloat("\"sigma_a\" for the gas", (float*)&sigma_a, 0, 1, "%.4f");
			ImGui::Text("Isotropy parameter for phase function\n 0 - isotropic\n<0 - backward bias\n>0 - forward bias");
			ImGui::SliderFloat("g", (float*)&g, -1, 1, "%.4f");
		}

		// Texturing controls
		if (ImGui::CollapsingHeader("Terrain Texturing Controls")) {
			ImGui::SliderFloat2("Grass texturing\nX - min height val\nY - max height val", (float*)&grassTexVals, -20, 20, "%.2f");
			ImGui::SliderFloat2("Rock texturing\nX - min height val\nY - max height val", (float*)&rockTextVals, -20, 20, "%.2f");
			ImGui::SliderFloat2("Snow texturing\nX - min height val\nY - max height val", (float*)&snowTexVals, -20, 20, "%.2f");
		}

		// Perlin Noise controls
		if (ImGui::CollapsingHeader("Perlin Noise Height Map")) {
			ImGui::SliderFloat("HM Frequency:", (float*)&paramsHM.x, -20, 20, "%.3f");
			ImGui::SliderFloat("HM Amplitude:", (float*)&paramsHM.y, -40, 40, "%.1f");
			generateHM = ImGui::Button("Generate perlin map");
			if (generateHM) {
				perlinNoiseTexture->GeneratePerlinNoiseTextureHM(renderer->getDevice(), textureMgr, paramsHM.x, paramsHM.y);
				camera->noiseData = perlinNoiseTexture->GetHeightDataRaw();
				camera->size = perlinNoiseTexture->GetTerrainSize();
				generateHM = false;
			}
			smooth = ImGui::Button("Smooth Height Map");
			if (smooth) {
				perlinNoiseTexture->SmoothHeightMap(renderer->getDevice(), textureMgr);
				camera->noiseData = perlinNoiseTexture->GetHeightDataRaw();
				camera->size = perlinNoiseTexture->GetTerrainSize();
			}
		}
		if (ImGui::CollapsingHeader("Perlin Noise Density Map")) {
			ImGui::SliderFloat("DM Frequency:", (float*)&paramsDMFreq, -2, 2, "% .3f");
			generateDM = ImGui::Button("Generate density map");
			if (generateDM) {
				perlinNoiseTexture->GeneratePerlinNoiseTextureDM(renderer->getDevice(), textureMgr, paramsDMFreq);
				generateDM = false;
			}
		}

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
		if (ImGui::CollapsingHeader("Spotlight Model")) {
			ImGui::SliderFloat3("Position Spotlight Model", (float*)&spotlightModelPosition, -100, 100, "%.2f");
		}

		// Time Controls.
		if (ImGui::CollapsingHeader("Time Scale")) {
			ImGui::SliderFloat("Time Scale", &timeScale, 1, 25, "%.2f");
		}
		ImGui::End();
	}

	// Step 6: Update Lights Data.
	UpdateLights();

	// Step 7: Main UI Elements.

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
		ImGui::Checkbox("Gravity?", &gravity);
		ImGui::Checkbox("Flight Mode? (Use Q and E)", &camFlightMode);
		if (camera->flightMode != camFlightMode)
		{
			if (camFlightMode)
				camera->flightMode = camFlightMode;
			else {
				camera->flightMode = camFlightMode;
				camera->setPosition(28, 10, 27);
			}
		}
		ImGui::Unindent();
	}

	// Reset Time Section.
	if (ImGui::CollapsingHeader("Reset Time")) {
		ImGui::Indent();
		resetBool = ImGui::Button("Reset?");
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

	// Step 8: Render the ImGui UI.
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

// Update the properties of all lights in the scene dynamically.
void App1::UpdateLights() {
	// Update the position of the spotlight based on the spotlight model.
	position[1] = XMFLOAT4(spotlightModelPosition.x, spotlightModelPosition.y + 2.1f, spotlightModelPosition.z + 1.f, 1.f);

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