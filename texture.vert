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
    gl_Position = projectionview * modelview * vec4(position, 1); // Transform according to mvp matrix
	gl_TexCoord[0].st = texcoord; // Feed texture coords to the handle
	outNormal = normal;
	outColor = color;	// Pass along vertex colors to the frag shader
}