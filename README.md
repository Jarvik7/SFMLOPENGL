SFMLOPENGL
==========

This is just a repo of me playing around with SFML and OpenGL to make a basic framework for learning.


Libraries:
SFML2.1: Window/GL context management, timers, music, input, texture loading
ASSIMP 3.0: Model loading
GLEW 1.10.1: Extension management
FreeGlut 2.8.1 (custom): Geometry routines

Externals:
Assimp.dll / assimp.framework
GLew32.dll / glew.framework
OpenGL32.dll <-- This is mesa3d, for use in VMs etc
SFML-xxx.framework (get from official sit for osx, win32 is statically compiled)

Currently done:
Basic game loop with some input
Looped music
Arbitrary textured 3d model with rotation (displaylist)
Ambient illumination

Todo:
User controlled camera
Light sources
Shadows
Reflections
Bump mapping
Use vertex arrays/compiled vertex arrays/VBO
Use OpenGL>3.0
Use libphysfs to pack resources?
Customize freeglut svn for opengl>3.0 geometry functions

