#version 330

in vec4 outColor;
in vec2 outTexcoord;
in vec2 outlmcoord;
in vec3 outNormal;

uniform sampler2D tex;
uniform sampler2D lm;

uniform bool vertexLighting;

out vec4 fragcolor;

void main()
{
    vec4 texColor = texture(tex, outTexcoord); // Get texel for this frag
	vec4 lmColor = texture(lm, outlmcoord); // Get the lightmap data

    if (lmColor.xyz == vec3(0,0,0)) lmColor = outColor * (1.0/255);
    if (!vertexLighting) fragcolor = 2 * lmColor * texColor; // Multiply texel by lm data and dampen
	else fragcolor = (outColor * (1.5/255)) * texColor; // Multiply texel by interpolated vertex color and dampen

	// Why is the 0.01 needed to adjust brightness? It wasn't the case when using glColorPointer
	// It's pretty close to 1/255, suggesting that something is a char instead of a float.
	// This applies to the scaling of the map too (0.02)
}