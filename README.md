SFMLOPENGL
==========

This is just a repo of me playing around with SFML and OpenGL to make a basic framework for learning.


Libraries:
SFML2.1: Window/GL context management, timers, music, input, texture loading
ASSIMP 3.0: Model loading
GLEW 1.10.1: Extension management
GLM

Externals:
Assimp.dll / assimp.framework
GLew32.dll / glew.framework
OpenGL32.dll <-- This is mesa3d, for use in VMs etc
SFML-xxx.framework (get from official sit for osx, win32 is statically compiled)

Currently done:
Rendering of all geometric information in BSP.
Diffuse textures
Vertex lighting
Anisotropic filtering w/ mipmaps
Antialiasing

Todo:
Dynamic lighting (deferred)
Fog
Culling
Collision
Proper face/material handling (done by passing type to shader?)

Goal tech:
Deferred lighting /w FXAA antialiasing pass (transparency w/ forward rendering?)

