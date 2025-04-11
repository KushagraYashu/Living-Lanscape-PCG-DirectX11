##### This repository is a duplicate of the original repository (https://github.com/Abertay-University-SDI/cmp301_coursework-KushagraYashu), the original repository can not be shared since its not owned by me.

# CMP502 Programming for Games, Coursework Repository
Name: Kushagra\
Student Number: 2400020\
Project Name: Living Landscape: A Dynamic Terrain Simulation with a Day-Night Cycle

## Issues: 
- Launching "release" build out of the box from visual studio will fail since .CSO files will not be present. First launch in "debug" mode to generate CSO and then release will work just fine.
- To run the project from Visual Studio or any other IDE, some library files are needed which can be fetched using the `GetLibraries.bat` file. It can be found inside the `Coursework` folder. Path: `main/Coursework/GetLibraries.bat`

## Project Description:
Living Landscape is a real-time 3D simulation showcasing a dynamic terrain influenced by height mapping and tessellation, integrated with a realistic day-night cycle. This project demonstrates advanced rendering techniques using DirectX.

### Features
-   **Dynamic Terrain and Tessellation**
    -   A height map manipulates a plane mesh that is dynamically tessellated for enhanced detail.
    -   Normals are correctly calculated using the height map for accurate lighting.
-   **Lighting and Shadows**
    -   Realistic lighting includes multiple light sources: a sun and a spotlight.
    -   The terrain and objects cast and receive shadows with proper shadow mapping.
-   **Post-Processing**
    -   Color grading (Akenine-Möller, Haines, & Hoffman, 2018) for scene-wide adjustments in brightness, contrast, and saturation.
    -   Bloom with multiple passes, applied to both the sun and the scene for a radiant effect.
-   **Day-Night Cycle**
    -   A smoothly transitioning day-night cycle that alters lighting dynamically.
-   **Graphical User Interface (Dear ImGui) (Cornut, 2014)**
    -   Provides full control over scene variables and objects, including:
        -   Light controls for position, intensity, and color.
        -   Model positioning controls.
        -   Time adjustment for the day-night cycle.
    -   Additional debugging tools:
        -   Checkboxes to toggle scene components and reveal underlying workings.
        -   FPS and frame time counters.
        -   A wireframe toggle for detailed visualization.
- **Additional Visual Features**
	- Moving Clouds
	    - Bitmap Clouds (RasterTek 2013b) that move based on time.
	 - Sky Dome
		  - A Sky Dome (RasterTek 2013a) sphere rendered without the depth buffer to create the illusion of a distant skybox.
   
### Technologies Used
-   **Programming and Graphics API:** C++ and DirectX 11.
-   **Shaders:** `hlsl`, including precompiled `.cso` files for height mapping, lighting, post-processing, etc.
-   **Resources:** Models and textures in `.fbx`,  `.png`, and `jpg` formats, placed in the `res` folder.
-   **User Interface:** `ImGui` (Cornut, 2014) for real-time parameter adjustments.

### Textures/Models Credits
- Mountain Height map and diffuse map: [Landscape Height Maps - Motion Forge Pictures](https://www.motionforgepictures.com/height-maps/)
- Clouds Texture: [Bitmap Clouds - RasterTek](https://rastertek.com/pic0206.gif)
- Sun Texture: [Sun Texture - Pinterest](https://it.pinterest.com/pin/417357090443735050/)
- Streetlight Model: [Street Lamp - SanguineArts](https://www.turbosquid.com/3d-models/street-lamp-794502)

## **References**
 Akenine-Möller, T., Haines, E., & Hoffman, N. (2018). _Real-Time Rendering (4th ed.)_ [Book]. Boca Raton: CRC Press.
 
Cornut, O. (2014). _Dear ImGui_ [Library/Framework]. Available at: [https://github.com/ocornut/imgui#dear-imgui](https://github.com/ocornut/imgui#dear-imgui) (Accessed: 8 December 2024).

RasterTek. (2013a). _Tutorial 10: Terrain Tessellation_ [Online]. Available at: [https://rastertek.com/tertut10.html](https://rastertek.com/tertut10.html) (Accessed: 3 December 2024).

RasterTek. (2013b). _Tutorial 11: Terrain Lighting_ [Online]. Available at: [https://rastertek.com/tertut11.html](https://rastertek.com/tertut11.html) (Accessed: 5 December 2024).
