#version 120

uniform sampler2D tex;
varying vec4 outColor;

void main()
{
    vec4 texColor = texture2D(tex,gl_TexCoord[0].st); // Get texel for this frag
	vec4 vertColor = outColor;
    gl_FragColor = texColor * vertColor * 0.01; // Multiply texel by interpolated vertex color
	// Why is the 0.01 needed to adjust brightness? It wasn't the case when using glColorPointer
}