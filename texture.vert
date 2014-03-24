#version 120

attribute vec3 position;
attribute vec2 texcoord;
attribute vec3 normal;	// Not presently used
attribute vec4 color;

uniform mat4 projectionview;
uniform mat4 modelview;

varying vec4 outColor;
varying vec3 outNormal;

void main()
{
	vec4 swizzled = vec4(position.xz,-position.y, 1); // Quake 3 uses a swizzled axis, unswizzle it
    gl_Position = projectionview * modelview * swizzled; // Transform according to mvp matrix

	gl_TexCoord[0].st = texcoord; // Feed texture coords to the handle
	outNormal = normal;
	outColor = color;	// Pass along vertex colors to the frag shader
}


/* TODO:
Fog
Deferred lighting * Shadows
Motion blur
Bump mapping
Lightmapping
Bloom/HDR
Lightshafts
Q3 shader -> GLSL conversion
*/