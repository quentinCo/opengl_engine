# Opengl Engine 

This project is an application that render a classical scene ([Sponza](http://graphics.cs.williams.edu/data/meshes.xml)) and enable to navigate in it.
This application implements different opengl rendering and physical simulation methods to render multiple pointlights and animate an particule system by attraction.
A user interface allows, among other things, to set the number of pointlight in scene, the blur effect apply on particles and the type physical interaction between them.

[![IMAGE ALT TEXT HERE](http://img.youtube.com/vi/uJkZxVlYVa0/0.jpg)](https://youtu.be/uJkZxVlYVa0)

## Rendering Methods
This application implement different advanced opengl method:
- [Forward+](https://fr.slideshare.net/takahiroharada/forward-34779335)
- Normal Mapping
- [Gaussian Blur](http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/)

## Physical Methods
Physical engine use a classic spring–mass system to simulate the particle attraction.
The application implement 3 differents interactions type:
- Simple spring-mass system attraction
- [Lennard Jones potential](https://en.wikipedia.org/wiki/Lennard-Jones_potential)
- [Gravitational field](https://en.wikipedia.org/wiki/Gravitational_field)

## Compilation instruction
Execute the cmake to generate the project.
- [x] Tested on windows (visual studio)
- [x] Tested on linux
- [ ] Tested on Mac

On visual studio set "engine" as main project.

### Possible problemes
If you have a error at the compilation with the inlcude of "experimental/filesystem", it's probabli that you don't support the experimental c++ methods. In this case, use the library [Boost](http://www.boost.org/) and reexecute the cmake file with command line below:
	cmake -DGLMLV_USE_BOOST_FILESYSTEM=ON ../opengl_engine/
Or by select GLMLV_USE_BOOST_FILESYSTEM, if you use cmake GUI.

## Navigation control
- z-q-s-d (or w-a-s-d with qwerty keybord) : moving front - left - back - right.
- space / shift: up / down.
- left / right click: activate / deactivate camera orientation.

## Directory organisation
- **/apps/engine**: contains the main application.
- **/lib**: contains the library "qc". This library contain every source codes and shaders of the graphic and physical engine.
- **/third-party**:
	- [glfw](http://www.glfw.org/): window and input gestion (easier than SDL).
	- [glm](http://glm.g-truc.net/0.9.8/index.html): mathematical library.
	- [glad](http://glad.dav1d.de/): openGl loader (like GLEW)
	- [imgui](https://github.com/ocornut/imgui): user interface.
	- [json](https://github.com/nlohmann/json): read / write json.
	- [tinyobjloader](https://github.com/syoyo/tinyobjloader): loading of .obj file.
	- [stb](https://github.com/nothings/stb): image loading.

### Internal library organisation : qc
The library is divided in two part, each other with their namespace:

- **qc::graphic**: this part contains every class and method used for the graphic rendering.
- **qc::physic**: this part contains every class and method used for the physical simulation. 

## Author
*[Quentin Couillard](http://www.quentincouillard.com/)*