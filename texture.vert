#version 330

in vec3 position;
in vec2 texcoord;
in vec3 normal;	// Not presently used
in vec4 color;

uniform mat4 projectionview;
uniform mat4 modelview;

out vec4 outColor;
out vec2 outTexcoord;
out vec3 outNormal;

void main()
{
	//De-swizzle and transform the vector position
	vec4 deswizzled = vec4(position.xz, -position.y, 1);
    gl_Position = projectionview * modelview * deswizzled;

	// Pass interpolated data to the frag shader
	outTexcoord = texcoord;
	outNormal = normal;
	outColor = color;
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