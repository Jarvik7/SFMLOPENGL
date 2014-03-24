#version 120

void main()
{
    gl_FrontColor = gl_Color; // Set color to vertex color
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position = ftransform(); // Transform according to mvp matrix
}