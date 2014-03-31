#version 330

in vec4 outColor;
in vec2 outTexcoord;
in vec3 outNormal;

uniform sampler2D tex;

out vec4 fragcolor;

void main()
{
    vec4 texColor = texture(tex, outTexcoord); // Get texel for this frag
    fragcolor = outColor * texColor * (1.0/255); // Multiply texel by interpolated vertex color and dampen
	// Why is the 0.01 needed to adjust brightness? It wasn't the case when using glColorPointer
	// It's pretty close to 1/255, suggesting that something is a char instead of a float.
	// This applies to the scaling of the map too (0.02)
}