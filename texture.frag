#version 120

uniform sampler2D tex;

void main()
{
    vec4 color = texture2D(tex,gl_TexCoord[0].st); // Get texel for this frag
    gl_FragColor = gl_Color*color*1.9; // Multiply texel by interpolated vertex color
}