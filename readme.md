Basic rendering-system created as an exercise with the following aims:
- Abstracting away the underlying graphics API (OpenGL 4.3 in this case).
- Having all OpenGL-commands execute in a separate rendering-thread.
- Employing basic deferred shading.
- Keeping things relatively simple; the aim is not to create a complete graphics engine.

The rendering-system is inspired by [BGFX](https://github.com/bkaradzic/bgfx), and works in the same general way:
1. The system creates and returns handles to resources.
2. Graphics-commands (uploads, updates, binds, state-changes, draw-commands, etc.) are logged and once a complete frame is generated, given to the rendering-thread for execution.
3. The rendering-thread owns the OpenGL-context, shadows it's state (to avoid redundant state-changes), and executes all commands to draw a frame once they are received.
4. While the frame is being drawn, the next frame is generated on the main thread.

### Usage
To compile, you'll need a C++11-supporting compiler, CMake, [GLFW](http://www.glfw.org/) (included as Git submodule), [GLM](http://glm.g-truc.net/0.9.5/index.html) (ditto), [GLEW](http://glew.sourceforge.net/) (included in repository), and [LuaJIT](http://luajit.org/) (libs for Visual C++ 2013 included).

Steps for Visual C++ 2013 (after git clone)
1. git submodule init
2. git submodule update
3. Run CMake
4. Compile project
5. Copy resources/shaders to build-folder
6. Copy data-folder containing scene data to build-folder (see below)
7. Run it

The needed scene data is the result of using [this exporter](https://github.com/cforfang/scene-exporter) on a e.g. a .obj. I've been using [Crytek Sponza](http://graphics.cs.williams.edu/data/meshes.xml), so the data-folder would then contain "scene.lua" and "meshdata.bin" from the tool, and the "textures"-folder from Sponza.

I've uploaded a ready-to-go data-folder [here](https://mega.co.nz/#!PdEAhJTC!Yo_O5B74K-e6hWo-byaYgfVJ9ml1W3IM1HCdFzOYA0M) (~76 MB).

When it's running, you use WASD to move the camera (shift to move faster), hold right-mouse-button to look around, F1 to toggle SSAO (on/off/occlusion only), F2 to toggle normal-mapping on/off, and F3 to toggle parallax-mapping on/off.

### Some takeaways:
- Deferring the execution of commands makes error-handling, detection, and logging more difficult since the context of those commands are lost (e.g. who generated it).

 As an example, an error compiling a shader is not detected before the rendering-thread executes the command, and when reporting it the only available information is what's explicitly sent over with the command. In BGFX, for instance, the file a shader's source was loaded from is _not_ available, so to track it down the user has to look at the dumped source (which is also likely to have been changed by BGFX when compared to the actual file in question) and go from there. Programming errors (e.g. a received command has bad data) also becomes much harder to track down.

- Since direct OpenGL-calls isn't available in the main thread, it becomes harder/impossible to use libraries like [AntTweakBar](http://anttweakbar.sourceforge.net) -- which I've been using in other projects.

- Having the graphics-commands execute in a separate thread is nice; in my draw-bound test-case the main thread was almost completely idle, and doing some heavier cycle-wasting to get the utilization up produced no drop in framerate. 

- I chose to use uniform-buffers (UBOs) for all my uniform-data, instead of glUniform*-commands. 
  
 When comparing the two previously, I found multiple glUniform-calls to be much faster than using a single glBuffer(Sub)Data, even when avoiding (blocking) buffer synchronization (e.g. by orphaning the buffer, having each drawcall use it's own buffer etc.). 

  The difference became smaller with more data, but for per-draw data consisting of just a mat4-model-matrix it was at least 5x slower on one system I tested. (This can have improved since, and for other systems it wasn't as bad.) Because of this I normally use glUniform for per-draw-data, and UBOs for per-frame and e.g. per-material data (as only makes sense) -- but for simplicity I used UBOs for everything here. (This seems like the way it should be anyway; hopefully drivers improve.)

- The basic form of deferred shading I implemented worked fine, and it was super-nice to decouple the geometry-drawing from the lighting. 

 Instead of worrying about packing the G-buffer as tightly as possible, I simply used MRT with one texture for diffuse colors, one for normals, and the depth-buffer to reconstruct positions. I also didn't bother to implement any fancy lighting-systems (with e.g. culling of lights not affecting the scene and drawing of minimum-bounding-objects for the lights) but instead did a single full-screen pass with all the lighting information in an UBO. The lighting itself is just basic diffuse lighting. 
 
  Since the scene I was using (Crytek Sponza, see above) included height-textures, I had a go at parallax-mapping which seemed to work fine, but with rather mediocre results -- probably because of the quality of the assets. SSAO post-processing was also easily implemented, and looks as SSAO does. Adding shadow-mapping should be super-easy, but having recently experimented with cascade shadow mapping and related techniques in an another project I didn't bother.


### Some TODOs/possible improvements:
- Complete the rendering-system API: it is currently just the bare minimum needed to draw a simple scene. No options to change state like face-culling, depth-testing, blending, stencil operations, set the vertex-format (it's hardcoded), and only a limited number of options for things like texture-formats are exposed.

- The rendering-system currently does a malloc()-free()-pair for each update of GPU buffers, but should probably instead use some kind of per-frame allocator which stores all data for the current frame together in preallocated storage. A quick run of VTune points this out as the primary hotspot on the main thread. (However, it's also ~2% of the time spent inside glDrawElements on the render-thread, so it's not really a slowdown.)

- There's no support for resource (e.g. programs, buffers, textures, ...) deletion -- only creation and updates, and the rendering-system destroys everything on exit. This wouldn't be hard to add; see e.g. how BGFX does it.
