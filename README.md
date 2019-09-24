# Computer Graphics and Virtual Reality

This code was written for a lab work for Computer Graphic class. 
This course presented the theoretical and practical concepts of 3D computer graphics and its applications in virtual reality. This is a discovery of computational and data models that are instrumental for the representation of objects and virtual scenes in 3D, such as shape, lighting, reflectance, textures and sensor models. This course also detailed how to exploit these models of rendering algorithms, such as the visibility determination, the calculation of direct and indirect lighting, or the treatment of the digital surface. In particular, this course gives an introduction to the following topics:

- Shape modeling,
- Image synthesis,
- Geometric processing and analysis,
- Computational geometry,
- Computer animation,
- Interactive 3D applications.

The languages used are C++, object-oriented programming and GPU programming with the OpenGL API.

## Getting Started

### Building

This is a standard CMake project. Building it consits in running:

```
cd <path-to-BaseGL-directory>
mkdir build
cd build
cmake 
cmake --build build
```

The resuling BaseGL executable is automatically copied to the root BaseGL directory, so that resources (shaders, meshes) can be loaded easily. By default, the program is compile in Debug mode. For a high performance Release binary, just us:

```
cmake --build build --config Release
```

### Running

To run the program
```
cd <path-to-BaseGL-directory>
./BaseGL [file.off]
```
Note that a collection of example meshes are provided in the Resources/Models directory. 

When starting to edit the source code, rerun 

```
cmake --build build 
```

to recompile. The resulting binary to use is always the one at located in the BaseGL directory, you can safely ignore whatever is generated in the build directory. 

This code was created with a base code written by Tamy Boubekeur (tamy.boubekeur@telecom-paristech.fr)
